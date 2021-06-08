#pragma once

#include <modm/board.hpp>
#include <modm/processing/protothread.hpp>

#include <erpc_services/matrix/matrix_multiply.h>

#include <utils/IoBufPack.hpp>

template<class BUFFER, uint16_t TimeoutUsRx = 1000>
class FifoDeviceWrapper : public Device, modm::NestedResumable<2>
{
public:
    FifoDeviceWrapper(IoBufPack<BUFFER> buffers) : buffers_(buffers){}
	bool hasReceived() final {
		return buffers_.getReceiveBuffer()->size() > 0;
	}

    erpc_status_t connect(){
        return kErpcStatus_Success;
    }

	modm::ResumableResult<erpc_status_t>
	write(const uint8_t* data, const uint32_t size) final
	{
        MODM_LOG_INFO << "write to " << (unsigned long) buffers_.getSendBuffer() << ": " << size << modm::endl;
        erpc_status_t status = kErpcStatus_SendFailed;
        RF_BEGIN();
        for(unsigned int i = 0; i < size; i++){
            uint8_t val = data[i];
            buffers_.getSendBuffer()->push(val);
        }
        status = kErpcStatus_Success;
        RF_END_RETURN(status);
	}

	modm::ResumableResult<erpc_status_t>
	read(uint8_t* data, const uint32_t size) final
	{
        MODM_LOG_INFO << "read: from " << (unsigned long) buffers_.getReceiveBuffer() << ": " << size << modm::endl;
        erpc_status_t status = kErpcStatus_ReceiveFailed;
        RF_BEGIN();
        timeout.restart(std::chrono::milliseconds(TimeoutUsRx));
        MODM_LOG_ERROR << " -- sizeA " << buffers_.getReceiveBuffer()->size() << modm::endl;
        RF_WAIT_UNTIL( (buffers_.getReceiveBuffer()->size() >= size) or timeout.isExpired());
        MODM_LOG_ERROR << " -- sizeB " << buffers_.getReceiveBuffer()->size() << modm::endl;
        if (timeout.isExpired()){
            MODM_LOG_ERROR << "RECEIVE TIMEOUT!!!!" << modm::endl;
            status = kErpcStatus_Timeout;
        }
        else{
             MODM_LOG_INFO << "R_0: " << modm::endl;
            for(unsigned int i = 0; i < size; i++){
                uint8_t val = buffers_.getReceiveBuffer()->front();
                data[i] = val;
                buffers_.getReceiveBuffer()->pop();
            }
            status = kErpcStatus_Success;
        }
        RF_END_RETURN(status);
    }

protected:
	modm::ShortPreciseTimeout timeout;
    IoBufPack<BUFFER> buffers_;
};

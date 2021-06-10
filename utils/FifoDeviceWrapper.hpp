#pragma once

#include <modm/board.hpp>

#include <erpc_services/matrix/matrix_multiply.h>

#include <utils/IoBufPack.hpp>


template<class BUFFER, uint16_t TimeoutUsRx = 1000>
class FifoDeviceWrapper : public Device
{
public:
    FifoDeviceWrapper(IoBufPack<BUFFER> buffers) : buffers_(buffers)
    {}

	bool hasReceived() final {
		return buffers_.getReceiveBuffer()->size() > 0;
	}

    erpc_status_t connect(){
        return kErpcStatus_Success;
    }

    erpc_status_t write(const uint8_t* data, const uint32_t size) final
	{
        // MODM_LOG_INFO << "write to " << (unsigned long) buffers_.getSendBuffer() << ": " << size << modm::endl;
        erpc_status_t status = kErpcStatus_SendFailed;
        for(unsigned int i = 0; i < size; i++){
            uint8_t val = data[i];
            buffers_.getSendBuffer()->push(val);
        }
        status = kErpcStatus_Success;
        return status;
    }

	erpc_status_t read(uint8_t* data, const uint32_t size) final
	{
        // MODM_LOG_INFO << "read: from " << (unsigned long) buffers_.getReceiveBuffer() << ": " << size << modm::endl;
        erpc_status_t status = kErpcStatus_ReceiveFailed;
        /// timeouts do not work with freertos in modm
        /// because for some reason the sysclock does not count anymore
        // timeout.restart(std::chrono::milliseconds(TimeoutUsRx));
        // while(buffers_.getReceiveBuffer()->size() < size and !timeout.isExpired()){
        //     // ...
        // }
        // if (timeout.isExpired()){
        //     MODM_LOG_ERROR << "RECEIVE TIMEOUT!!!!" << modm::endl;
        //     status = kErpcStatus_Timeout;
        // }
        /// so we dont have a timeout at all for now .. (bad)
        // else{
        //     for(unsigned int i = 0; i < size; i++){
        //         uint8_t val = buffers_.getReceiveBuffer()->front();
        //         data[i] = val;
        //         buffers_.getReceiveBuffer()->pop();
        //     }
        //     status = kErpcStatus_Success;
        // }
        while(buffers_.getReceiveBuffer()->size() < size){
            // ...
        }
        for(unsigned int i = 0; i < size; i++){
            uint8_t val = buffers_.getReceiveBuffer()->front();
            data[i] = val;
            buffers_.getReceiveBuffer()->pop();
        }
        status = kErpcStatus_Success;
        return status;
    }

protected:
	modm::ShortPreciseTimeout timeout;
    IoBufPack<BUFFER> buffers_;
};


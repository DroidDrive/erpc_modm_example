#pragma once

/// include erpc base stuff
#include <erpc/erpc_c/setup/erpc_server_setup.h>
/// include generated idl stuff for services
#include <erpc_services/matrix/matrix_multiply_server.h>
/// include generated idl stuff for services
#include <erpc_services/matrix/matrix_multiply.h>

/// include erpc addons used
#include <erpc/erpc_c/setup/erpc_mbf_setup_addons.h>
#include <erpc/erpc_c/setup/erpc_transport_setup_addons.h>

/// include transport used
#include <erpc/erpc_c/transports/erpc_modm_device_transport.h>

#include <modm/board.hpp>
#include <modm/processing/protothread.hpp>

#include <utils/IoBufPack.hpp>
/// ###############################################################
/// Server
/// ###############################################################

template<class BUFFER, uint16_t TimeoutUsRx = 1000>
class FifoDeviceWrapper : public Device, modm::Resumable<3>
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
        RF_BEGIN(1);
        erpc_status_t status = kErpcStatus_SendFailed;
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
        RF_BEGIN(2);
        erpc_status_t status = kErpcStatus_ReceiveFailed;
        timeout.restart(std::chrono::microseconds(TimeoutUsRx));
        RF_WAIT_UNTIL( (buffers_.getReceiveBuffer()->size() > 0) or timeout.isExpired());
        if (timeout.isExpired()){
            status = kErpcStatus_Timeout;
        }
        else{
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


template<class BUFFER>
class RpcServer : modm::pt::Protothread, modm::NestedResumable<2>
{
public:
    static inline bool serverUp = false;
public:
    RpcServer(IoBufPack<BUFFER> buffers) : device_{FifoDeviceWrapper<BUFFER>(buffers)}{}

    bool run()
    {
        PT_BEGIN()
        {
            MODM_LOG_INFO << "[Server] Starting ..." << modm::endl;

            /* Init eRPC server environment */
            /* transport layer initialization */
            erpc_transport_t s_transport = erpc_transport_modm_device_init(&device_);

            /* MessageBufferFactory initialization */
            erpc_mbf_t s_message_buffer_factory = erpc_mbf_static_fixed_init<2, 256>();

            MODM_LOG_INFO << "[Server] Initializing server ..." << modm::endl;

            /* eRPC server side initialization */
            erpc_server_init(s_transport, s_message_buffer_factory);

            MODM_LOG_INFO << "[Server] Adding services ..." << modm::endl;

            /* connect generated service into server, look into erpc_matrix_multiply_server.h */
            erpc_service_t service = create_MatrixMultiplyService_service();
            erpc_add_service_to_server(service);

            MODM_LOG_INFO << "[Server] Spinning ..." << modm::endl;
            RpcServer::serverUp = true;
            while(true){		
                auto status = erpc_server_poll();
                if(status){
                    MODM_LOG_ERROR << "[Server] Poll() returned status '" << (int) status << "'!" << modm::endl; 
                }
                modm::delay(10ms);
            }
            MODM_LOG_INFO << "[Server] END ..." << modm::endl;
        }
        PT_END();
    }
protected:
    FifoDeviceWrapper<BUFFER> device_;
};

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
#include <utils/FifoDeviceWrapper.hpp>
/// ###############################################################
/// Server
/// ###############################################################
template<class BUFFER>
class RpcServer : modm::pt::Protothread
{
public:
    RpcServer(IoBufPack<BUFFER> buffers) : device_{FifoDeviceWrapper<BUFFER>(buffers)}{}

    bool init(){
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
        MODM_LOG_INFO << "[Server] Initialization done ..." << modm::endl;
        return true;
    }

    bool run()
    {
        erpc_status_t status;
        PT_BEGIN();
        MODM_LOG_INFO << "[Server] Spinning ..." << modm::endl;
        while(true){		
            status = erpc_server_poll();
            if(status){
                MODM_LOG_ERROR << "[Server] Poll() returned status '" << (int) status << "'!" << modm::endl; 
            }
            // modm::delay(10ms);
            timeout_.restart(10ms);
			PT_WAIT_UNTIL(timeout_.isExpired());
        }
        MODM_LOG_INFO << "[Server] END ..." << modm::endl;
        PT_END();
    }
protected:
    FifoDeviceWrapper<BUFFER> device_;
private:
    modm::ShortTimeout timeout_;

};

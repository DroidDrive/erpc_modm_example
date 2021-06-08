#pragma once

#include <functional>
#include <modm/board.hpp>
#include <modm/processing/protothread.hpp>

/// include erpc base stuff
#include <erpc/erpc_c/setup/erpc_client_setup.h>
/// include generated idl stuff for services
#include <erpc_services/matrix/matrix_multiply.h>

/// include transport used
#include <erpc/erpc_c/transports/erpc_modm_device_transport.h>

#include <modm/board.hpp>

#include <utils/IoBufPack.hpp>
#include <utils/FifoDeviceWrapper.hpp>

/// ###################################
/// Client
/// ###################################
template<class BUFFER>
class RpcClient : modm::pt::Protothread
{
public:
    RpcClient(IoBufPack<BUFFER> buffers) : device_{FifoDeviceWrapper<BUFFER>(buffers)}, m1_{{0}}, m2_{{0}}
	{}

    bool init()
	{
		MODM_LOG_INFO << "[Client] Starting ..." << modm::endl;
		// result = {{0}};

		/* Init eRPC client environment */
		/* transport layer initialization */
		erpc_transport_t c_transport = erpc_transport_modm_device_init(&device_);

		/* MessageBufferFactory initialization */
		erpc_mbf_t c_message_buffer_factory = erpc_mbf_static_fixed_init<2, 256>();

		MODM_LOG_INFO << "[Client] Initializing transport ..." << modm::endl;

		/* eRPC client side initialization */
		erpc_client_init(c_transport, c_message_buffer_factory);
		
		erpc_client_set_error_handler(&RpcClient::clientErrorCallback);
		
		/* other code like init matrix1 and matrix2 values */
		for(int i = 0; i < 5; i++){
			for(int j = 0; j < 5; j++){
				m1_[i][j] = j+1;
				m2_[i][j] = j+1;
			}
		}
		MODM_LOG_INFO << "[Client] Initializing done ..." << modm::endl;
		return true;
	}

    bool run()
	{
		Matrix result;
		PT_BEGIN();
        while(true)
		{
			MODM_LOG_INFO << "[Client] Calling service() ..." << modm::endl;
			/* call eRPC functions */
			remote::erpcMatrixMultiplyX(m1_, m2_, result);
			MODM_LOG_INFO << "[Client] Service called() ..." << modm::endl;
			if(RpcClient::latestClientError == 0)
			{
				MODM_LOG_INFO << "\n   = \n\n";
				/* other code like print result matrix */
				for(int i = 0; i < 5; i++){
					for(int j = 0; j < 5; j++){
						MODM_LOG_INFO << result[i][j] << " ";
						/// change input matrix slightly
						m1_[i][j] += 1;
					}
					MODM_LOG_INFO << "; " << modm::endl;
				}
			}
			MODM_LOG_INFO << "[Client] Done ..." << modm::endl; 
			timeout_.restart(10ms);
			PT_WAIT_UNTIL(timeout_.isExpired());
			break;
		}
		MODM_LOG_INFO << "[Client] END ..." << modm::endl; 
		PT_END();
    }
private:	
	static void clientErrorCallback(erpc_status_t err, uint32_t functionID){
		latestClientError = err;
		if(err != 0){
			MODM_LOG_INFO << "[ERROR] " << err << " in function " << functionID << modm::endl; 
		}
	}
protected:
    FifoDeviceWrapper<BUFFER> device_;
private:
    modm::ShortTimeout timeout_;
	Matrix m1_, m2_;
	static inline erpc_status_t latestClientError = kErpcStatus_Success;
};

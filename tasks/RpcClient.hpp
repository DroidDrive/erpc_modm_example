#pragma once

#include <tasks/RpcServer.hpp>

/// include erpc base stuff
#include <erpc/erpc_c/setup/erpc_client_setup.h>
/// include generated idl stuff for services
#include <erpc_services/matrix/matrix_multiply.h>

/// include transport used
#include <erpc/erpc_c/setup/erpc_transport_setup_addons.h>
#include <erpc/erpc_c/setup/erpc_mbf_setup_addons.h>
#include <erpc/erpc_c/transports/erpc_fifo_transport.h>

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

#include <utils/IoBufPack.hpp>

/// ###################################
/// Client
/// ###################################
template<class BUFFER>
class RpcClient
{
public:
	static constexpr char name[] { "RpcClient" };
	static bool latestClientError;
public:
	static void run(void* params)
	{
		MODM_LOG_INFO << "[Client] Waiting for Server ..." << modm::endl;
		while(!RpcServer<BUFFER>::serverUp){
		}

		MODM_LOG_INFO << "[Client] Starting ..." << modm::endl;

		IoBufPack<BUFFER>* buffers =  static_cast<IoBufPack<BUFFER>*>(params);

		/* Matrices definitions */
		Matrix matrix1, matrix2, result = {{0}};

		/* Init eRPC client environment */
		/* transport layer initialization */
		erpc_transport_t c_transport = erpc_transport_fifo_init(buffers->getReceiveBuffer(), buffers->getSendBuffer());

		/* MessageBufferFactory initialization */
		erpc_mbf_t c_message_buffer_factory = erpc_mbf_static_fixed_init<2, 256>();

		MODM_LOG_INFO << "[Client] Initializing transport ..." << modm::endl;

		/* eRPC client side initialization */
		erpc_client_init(c_transport, c_message_buffer_factory);
		erpc_client_set_error_handler(RpcClient<BUFFER>::clientErrorCallback);
		
		/* other code like init matrix1 and matrix2 values */
		for(int i = 0; i < 5; i++){
			for(int j = 0; j < 5; j++){
				matrix1[i][j] = j+1;
				matrix2[i][j] = j+1;
			}
		}

		while(true)
		{

			MODM_LOG_INFO << "[Client] Calling service() ..." << modm::endl;
			/* call eRPC functions */
			remote::erpcMatrixMultiplyX(matrix1, matrix2, result);
			MODM_LOG_INFO << "[Client] Service called() ..." << modm::endl;

			if(latestClientError == 0)
			{
				MODM_LOG_INFO << "\n   = \n\n";
				/* other code like print result matrix */
				for(int i = 0; i < 5; i++){
					for(int j = 0; j < 5; j++){
						MODM_LOG_INFO << result[i][j] << " ";
						/// change input matrix slightly
						matrix1[i][j] += 1;
					}
					MODM_LOG_INFO << "; " << modm::endl;
				}
			}
			MODM_LOG_INFO << "[Client] Done ..." << modm::endl; 
			vTaskDelay( 1000 * MILLISECONDS );        
		}
		MODM_LOG_INFO << "[Client] END ..." << modm::endl; 
	}

	static void clientErrorCallback(erpc_status_t err, uint32_t functionID){
		latestClientError = err;
		if(err != 0){
			MODM_LOG_INFO << "[ERROR] " << err << " in function " << functionID << modm::endl; 
		}
	}
};


template<class BUFFER>
inline bool RpcClient<BUFFER>::latestClientError = false;
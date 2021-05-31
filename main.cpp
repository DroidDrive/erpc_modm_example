/*
 * Copyright (c) 2014, Georgi Grinshpun
 * Copyright (c) 2014, Sascha Schade
 * Copyright (c) 2015-2017, 2019 Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

using namespace modm::platform;

/// ###################################
/// Server includes 
/// ###################################
/// include generated idl stuff for services
#include "erpc_services/matrix/matrix_multiply_server.h"
/// include erpc base stuff
#include <erpc_server_setup.h>
#include <erpc_fifo_transport.h>
/// ###################################
/// Client includes 
/// ###################################
/// include generated idl stuff for services
#include "erpc_services/matrix/matrix_multiply.h"
/// include erpc base stuff
#include "erpc_client_setup.h"
#include "erpc_fifo_transport.h"

#include "utils/RingBuffer.hpp"


template<class T>
class IOBufs{
public:
	IOBufs(T* in, T* out)
	: receiveBuf_{in}, sendBuf_{out}{}
public:
	T* getReceiveBuffer(){return receiveBuf_;}
	T* getSendBuffer(){return sendBuf_;}
private:
	T* receiveBuf_;
	T* sendBuf_;
};

using myContainer_t = std::queue<uint8_t>;

/// ###############################################################
/// Services
/// ###############################################################
/* implementation of function call */
void services::erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    /* code for multiplication of matrices */
    //MODM_LOG_INFO << "Service erpcMatrixMultiply() called!" << modm::endl;
    /// test: return matrix 1
    //std::memcpy(result_matrix, matrix1, 5*5*sizeof(uint32_t));
    /// initialize result matrix to zero
    std::memset(result_matrix, 0, 5*5*sizeof(uint32_t));
    // Multiplying matrix a and b and storing in result.
    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < 5; ++j){
            for(int k = 0; k < 5; ++k){
                result_matrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            MODM_LOG_INFO << matrix1[i][j] << " ";
        }
        MODM_LOG_INFO << "; " << modm::endl;
    }
    MODM_LOG_INFO << "\n   * \n\n";
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            MODM_LOG_INFO << matrix2[i][j] << " ";
        }
        MODM_LOG_INFO << "; " << modm::endl;
    }
   //MODM_LOG_INFO << "Service erpcMatrixMultiply() end!" << modm::endl;
}

/// ###############################################################
/// Server
/// ###############################################################
class RpcServer
{
public:
	static constexpr char name[] { "RpcServer" };

public:
	static void run(void* params)
	{
		MODM_LOG_INFO << "[Server] Starting ..." << modm::endl;

		IOBufs<myContainer_t>* buffers =  static_cast<IOBufs<myContainer_t>*>(params);

        /* Init eRPC server environment */
        /* transport layer initialization */
        erpc_transport_t s_transport = erpc_transport_fifo_init(buffers->getReceiveBuffer() , buffers->getSendBuffer());

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
        while(true){		
            auto status = erpc_server_poll();
			vTaskDelay( 10 * MILLISECONDS );        
		}
        MODM_LOG_INFO << "[Server] END ..." << modm::endl;
}
};

/// ###############################################################
/// Client
/// ###############################################################
static bool latestClientError = false;
void clientErrorCallback(erpc_status_t err, uint32_t functionID){
    latestClientError = err;
    if(err != 0)
        MODM_LOG_INFO << "[ERROR] " << err << " in function " << functionID << modm::endl; 
}

class RpcClient
{
public:
	static constexpr char name[] { "RpcClient" };

public:
	static void run(void* params)
	{
        MODM_LOG_INFO << "[Client] Starting ..." << modm::endl;

		IOBufs<myContainer_t>* buffers =  static_cast<IOBufs<myContainer_t>*>(params);

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
        erpc_client_set_error_handler(clientErrorCallback);
        
        /* other code like init matrix1 and matrix2 values */
        for(int i = 0; i < 5; i++){
            for(int j = 0; j < 5; j++){
                matrix1[i][j] = j+1;
                matrix2[i][j] = j+1;
            }
        }
    
        while(true)
        {

            MODM_LOG_INFO << "[Client] Calling service ..." << modm::endl;
            /* call eRPC functions */
            remote::erpcMatrixMultiplyX(matrix1, matrix2, result);
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
};

// ----------------------------------------------------------------------------
template <typename Gpio, int SleepTime>
class P: modm::rtos::Thread
{
public:
	P(char c): Thread(2, 1<<11), c(c) {
	}

	void run()
	{
		Gpio::setOutput();
		while (true)
		{
			sleep(SleepTime * MILLISECONDS);

			Gpio::toggle();
			{
				static modm::rtos::Mutex lm;
				modm::rtos::MutexGuard m(lm);
				//MODM_LOG_INFO << char(i + c);
			}
			i = (i+1)%10;
			a *= 3.141f;
		}
	}
private:
	char c;
	uint8_t i = 0;
	volatile float a = 10.f;
};

P< Board::LedRed,   260      > p1('0');
P< Board::LedGreen, 260 + 10 > p2('a');
P< Board::LedBlue,  260 + 20 > p3('A');


// ----------------------------------------------------------------------------
// pushing into here wont allocate stuff properly?
static myContainer_t buffer1;
static myContainer_t buffer2;

IOBufs<myContainer_t> serverBuffers(&buffer1, &buffer2);
IOBufs<myContainer_t> clientBuffers(&buffer2, &buffer1);

int main()
{
	Board::initialize();

	xTaskCreate(RpcServer::run, RpcServer::name, 12000, (void*) &serverBuffers, 2, 0);
	xTaskCreate(RpcClient::run, RpcClient::name, 12000, (void*) &clientBuffers, 3, 0);

	modm::rtos::Scheduler::schedule();
	return 0;
}
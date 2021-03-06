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

using namespace modm::platform;

#include <erpc/erpc_c/transports/erpc_fifo_transport.h>

#include <utils/IoBufPack.hpp>
#include <tasks/RpcServer.hpp>
#include <tasks/RpcClient.hpp>
#include <tasks/LedTask.hpp>

using myContainer_t = erpc::RingBuffer<uint8_t, 1024>;

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

// ----------------------------------------------------------------------------
// pushing into here wont allocate stuff properly?
static myContainer_t buffer1;
static myContainer_t buffer2;

IoBufPack<myContainer_t> serverBuffers(&buffer1, &buffer2);
IoBufPack<myContainer_t> clientBuffers(&buffer2, &buffer1);

int main()
{
	Board::initialize();

    LedTask<Board::LedGreen, 260 + 10> led1;
    LedTask<Board::LedBlue,  260 + 20> led2;
    RpcClient<myContainer_t> client(clientBuffers);
    RpcServer<myContainer_t> server(serverBuffers);

	modm::rtos::Scheduler::schedule();
	return 0;
}

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
#include <modm/architecture/utils.hpp>
#include <modm/processing/timer.hpp>
#include <modm/processing/resumable.hpp>

using namespace modm::platform;

#include <utils/IoBufPack.hpp>
#include <tasks/RpcServer.hpp>
#include <tasks/RpcClient.hpp>
#include <tasks/Led.hpp>

#include <erpc/erpc_c/setup/erpc_transport_setup_addons.h>

using myContainer_t = erpc::RingBuffer<uint8_t, 1024>;

/// ###############################################################
/// Services
/// ###############################################################
/* implementation of function call */
void services::erpcMatrixMultiply(Matrix matrix1, Matrix matrix2, Matrix result_matrix)
{
    /* code for multiplication of matrices */
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

// pushing into here wont allocate stuff properly?
static myContainer_t buffer1;
static myContainer_t buffer2;

IoBufPack<myContainer_t> serverBuffers(&buffer1, &buffer2);
IoBufPack<myContainer_t> clientBuffers(&buffer2, &buffer1);

// protothreads
RpcClient<myContainer_t> client(clientBuffers);
RpcServer<myContainer_t> server(serverBuffers);
BlinkingLight light;

int main()
{
	Board::initialize();

    Board::LedRed::setOutput();
    bool serverOk = server.init();
    bool clientOk = client.init();
    while(serverOk && clientOk){
        light.run();
        server.run();
        client.run();
    }
    MODM_LOG_ERROR << "ERROR: Client [" << (int) clientOk << "], Server [" << (int) serverOk << "]" << modm::endl;
    while(true){
        // idle ...
    }
	return 0;
}


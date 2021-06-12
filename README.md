# erpc_modm

This example utilizes the modm/erpc module ([from this modm fork](https://github.com/DroidDrive/modm/tree/erpc)) and uses [erpc](https://github.com/EmbeddedRPC/erpc) + some added on features to enable remote procedure calls (RPC) on a stm32f7 microcontroller.



## Prerequisites

* [install modm](https://modm.io/guide/installation/)
  * dont forget to use [this modm fork](https://github.com/DroidDrive/modm/tree/erpc)  instead of the original modm
* `mkdir ${HOME}/gitclones`
* `git clone https://github.com/DroidDrive/erpc_modm ${HOME}/gitclones/erpc_modm`

## Contents

* project.xml
  * modm lbuild project file containing module descriptions, targets and dependencies
  * it contains the full (absolute) path to the modm repository used (can be changed, just for convenience on my part atm)
* main.cpp
  * software entrypoint
* FreeRTOSConfigLocal.h
  * FreeRTOS configuration file which is automagically included by modm
* erpc_services
  * containing generated messaging files using erpcgen (can be installed via installing erpc)
  * for detailed explanations on how to use the erpc IDL [click here](https://github.com/EmbeddedRPC/erpc/wiki)
* tasks/
  * directory containing classes for all the freertos tasks
* utils/
  * directory containing utility classes

# IDL

The IDL files (inside erpc_services) were originally created by erpc's `erpcgen` tool like this

* erpcgen -I erpc_services/ -o erpc_services/matrix matrix.erpc

Because erpc is unable to utilize server and client in the same code (that's just not a use case), i have changed the IDL generated code slightly to allow that. These changes are mainly to distinguish the server service function (the RPC server endpoint) from the client service function (the RPC client endpoint). These changes are not supported and are only here so I can test client and server from the same code base. 

## Build

* `cd ${HOME}/gitclones/erpc_modm`
* `lbuild build`
  * this steps utilizes modm's lbuild to gather all relevant modules (including the erpc module) and create scons templates
* `scons`
  * compile

## Run

Be sure to build the software prior to this step

* attach nucleo_f7 target via usb
* `scons program`

The blue and green LEDs should light up and do an alternating blink pattern


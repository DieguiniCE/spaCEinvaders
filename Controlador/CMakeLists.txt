# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# CMakeLists.txt - spaCEinvaders Pico Controller
cmake_minimum_required(VERSION 3.13)

if(DEFINED ENV{PICO_SDK_PATH} AND NOT "$ENV{PICO_SDK_PATH}" STREQUAL "")
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
else()
    set(PICO_SDK_PATH "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/pico-sdk")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(spaceinvaders_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(spaceinvaders_controller
    controller.c
)

pico_enable_stdio_usb(spaceinvaders_controller 1)
pico_enable_stdio_uart(spaceinvaders_controller 0)

target_link_libraries(spaceinvaders_controller
    pico_stdlib
    hardware_gpio
)

pico_add_extra_outputs(spaceinvaders_controller)
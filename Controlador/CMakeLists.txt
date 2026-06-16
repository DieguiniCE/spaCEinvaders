<<<<<<< Updated upstream
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

# CMakeLists.txt – spaCEinvaders Pico Controller
cmake_minimum_required(VERSION 3.13)

# Importar el SDK del Pico antes de project()
=======
# CMakeLists.txt - spaCEinvaders Pico Controller
cmake_minimum_required(VERSION 3.13)

>>>>>>> Stashed changes
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(spaceinvaders_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
<<<<<<< Updated upstream
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(spaceinvaders_controller
    controller.c
)

# Habilitar salida USB-serial para depuración (stdio_init_all)
pico_enable_stdio_usb(spaceinvaders_controller 1)
pico_enable_stdio_uart(spaceinvaders_controller 0)

# Enlazar librerías del SDK que usamos
target_link_libraries(spaceinvaders_controller
    pico_stdlib
    hardware_uart
    hardware_gpio
)

# Generar .uf2 (el archivo que se arrastra al Pico en modo BOOTSEL)
=======

pico_sdk_init()

add_executable(spaceinvaders_controller controller.c)

# UART por USB (CDC serial virtual)
pico_enable_stdio_usb(spaceinvaders_controller 1)
pico_enable_stdio_uart(spaceinvaders_controller 0)

target_link_libraries(spaceinvaders_controller pico_stdlib hardware_gpio)

>>>>>>> Stashed changes
pico_add_extra_outputs(spaceinvaders_controller)

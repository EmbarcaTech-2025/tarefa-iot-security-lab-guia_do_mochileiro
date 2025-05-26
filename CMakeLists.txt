# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(iot_security_lab C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(publisher_firmware
    main_publisher.c
    src/wifi_conn.c
    src/mqtt_comm.c
    src/xor_cipher.c
    src/ssd1306.c
    src/display.c
)

add_executable(subscriber_firmware
    main_subscriber.c
    src/wifi_conn.c
    src/mqtt_comm.c
    src/xor_cipher.c
    src/ssd1306.c
    src/display.c
)

pico_set_program_name(publisher_firmware "iot_security_lab_publisher")
pico_set_program_version(publisher_firmware "0.1")

pico_set_program_name(subscriber_firmware "iot_security_lab_subscriber")
pico_set_program_version(subscriber_firmware "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_usb(publisher_firmware 1)
pico_enable_stdio_uart(publisher_firmware 0)

pico_enable_stdio_usb(subscriber_firmware 1)
pico_enable_stdio_uart(subscriber_firmware 0)

set(LINK_LIBRARIES
# Biblioteca padrão do Pico SDK, que fornece funções básicas para o RP2040 (GPIO, temporizadores, UART, etc.).
        pico_stdlib
        # Oferece suporte Wi-Fi (CYW43) com LwIP (pilha TCP/IP) em um modo thread-safe em segundo plano.
        pico_cyw43_arch_lwip_threadsafe_background
        # Adiciona suporte a cliente MQTT sobre o LwIP.
        pico_lwip_mqtt
        # A pilha Lightweight IP (LwIP), que fornece funcionalidades TCP/IP (HTTP, MQTT, UDP, DHCP, etc.).
        pico_lwip
        # O driver do chip Wi-Fi CYW43 (usado no Pico W).
        pico_cyw43_driver
        # i2c hardware driver, que permite comunicação com dispositivos I2C.        
        hardware_i2c
        )

target_link_libraries(publisher_firmware ${LINK_LIBRARIES})
target_link_libraries(subscriber_firmware ${LINK_LIBRARIES})

# Add the standard include files to the build
target_include_directories(publisher_firmware PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include()
)

target_include_directories(subscriber_firmware PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include()
)

pico_add_extra_outputs(publisher_firmware)
pico_add_extra_outputs(subscriber_firmware)
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(LuxCor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(LuxCor LuxCor.c lib/bh1750_light_sensor.c lib/gy33.c lib/ssd1306.c lib/matriz_leds.c lib/led_rgb.c)

pico_set_program_name(LuxCor "LuxCor")
pico_set_program_version(LuxCor "0.1")

# Generate PIO header
pico_generate_pio_header(LuxCor ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(LuxCor 1)
pico_enable_stdio_usb(LuxCor 0)

# Add the standard library to the build
target_link_libraries(LuxCor
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(LuxCor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Add any user requested libraries
target_link_libraries(LuxCor 
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_adc
        hardware_uart
        )

pico_add_extra_outputs(LuxCor)


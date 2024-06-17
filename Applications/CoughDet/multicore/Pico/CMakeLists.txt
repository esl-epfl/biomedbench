cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(CoughDetect C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
FILE(GLOB_RECURSE MyCSources Src/*.c)

add_executable(CoughDetect
  ${MyCSources}
)

add_compile_definitions(PICO=1)
add_compile_definitions(NUMBER_OF_CORES=2)
add_compile_options(-O3 -g3 -ffast-math)
include_directories(Inc)

pico_enable_stdio_usb(CoughDetect 1)
pico_enable_stdio_uart(CoughDetect 0)
pico_add_extra_outputs(CoughDetect)
target_link_libraries(CoughDetect 
    pico_stdlib
    pico_multicore
    hardware_spi 
    hardware_sleep 
    hardware_clocks
    hardware_rosc
    hardware_rtc)


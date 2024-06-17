cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(HeartBeatClass C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
FILE(GLOB_RECURSE MyCSources Src/*.c)

add_definitions(-DPICO)
add_compile_options(-O3 -g3)

add_executable(HeartBeatClass
  ${MyCSources}
)

include_directories(Inc)

pico_enable_stdio_usb(HeartBeatClass 1)
pico_enable_stdio_uart(HeartBeatClass 0)
pico_add_extra_outputs(HeartBeatClass)
target_link_libraries(HeartBeatClass 
    pico_stdlib
    hardware_spi 
    hardware_sleep 
    hardware_clocks
    hardware_rosc
    hardware_rtc)


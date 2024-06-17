cmake_minimum_required (VERSION 3.13)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(BSS_pico C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

include_directories(Inc)
include_directories(Inc/data)
include_directories(Inc/PICO)

add_compile_options(-O3 -g3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
FILE(GLOB_RECURSE MyCSources Src/*.c fixmath/*.c)
add_executable(BSS_pico
  ${MyCSources}
)

pico_enable_stdio_usb(BSS_pico 1)
pico_enable_stdio_uart(BSS_pico 0)

pico_add_extra_outputs(BSS_pico)

target_link_libraries(BSS_pico m)
target_link_libraries(BSS_pico pico_stdlib
                                hardware_spi 
                                hardware_sleep 
                                hardware_clocks
                                hardware_rosc
                                hardware_rtc)

add_compile_definitions(PICO=1)

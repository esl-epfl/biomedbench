cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
project(SeizDetSVM C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
FILE(GLOB_RECURSE MyCSources Src/*.c Src/*.cpp)

add_definitions(-DPICO)
add_compile_options(-O3 -g3)

add_executable(SeizDetSVM
  ${MyCSources}
)

include_directories(Inc)
include_directories(Inc/lib)

set(FREERTOS_SRC_DIRECTORY "${PICO_SDK_PATH}/lib/tinyusb/lib/FreeRTOS-Kernel")

# Add FreeRTOS as a library
add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
)

pico_enable_stdio_usb(SeizDetSVM 1)
pico_enable_stdio_uart(SeizDetSVM 0)
pico_add_extra_outputs(SeizDetSVM)
target_link_libraries(SeizDetSVM 
    pico_stdlib
    pico_malloc
    pico_mem_ops
    hardware_spi 
    hardware_sleep 
    hardware_clocks
    hardware_rosc
    hardware_rtc
    FreeRTOS)
    


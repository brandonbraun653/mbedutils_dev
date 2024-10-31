# This file adds targets to compile in the FreeRTOS runtime, configured for POSIX simulation.
#
# https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS/Demo/Posix_GCC
# https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/03-Emulation-and-simulation/Linux/FreeRTOS-simulator-for-Linux


set(LIB_NAME ${PROJECT_NAME}_lib_freertos)

add_library(${LIB_NAME} STATIC
    ${PROJECT_SOURCE_DIR}/../lib/freertos/croutine.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/event_groups.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/list.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/queue.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/stream_buffer.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/tasks.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/timers.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/portable/MemMang/heap_4.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/portable/ThirdParty/GCC/Posix/port.c
    ${PROJECT_SOURCE_DIR}/../lib/freertos/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${PROJECT_NAME}_headers
    ${PROJECT_NAME}_internal_headers
    etl::etl
)

target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../lib/freertos/include
    ${PROJECT_SOURCE_DIR}/../lib/freertos/portable/ThirdParty/GCC/Posix/
    ${PROJECT_SOURCE_DIR}/../lib/freertos/portable/ThirdParty/GCC/Posix/utils
)

target_compile_options(${LIB_NAME} PRIVATE
    -D_WINDOWS
    -Wall
    -Wextra
    -Wpedantic
)
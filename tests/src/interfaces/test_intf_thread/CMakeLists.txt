include(${MBEDUTILS_TEST_DIR}/test_target.cmake)
create_test_target(
    TARGET
        IntTest_Interface_Thread_SIM
    TEST_SOURCES
        test_intf_thread.cpp
    INSTRUMENTED_SOURCES
        ${PROJECT_SOURCE_DIR}/../lib/mbedutils_sim/sim_thread.cpp
    INCLUDE_DIRS
        ${TST_CMN_INC_DIRS}
    LIBRARIES
        mbedutils_headers
        mbedutils_internal_headers
    EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

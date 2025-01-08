/******************************************************************************
 *  File Name:
 *    test_runtime_harness.cpp
 *
 *  Description:
 *    Provides a runtime-agnostic test harness for STL and FreeRTOS based tests
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "mbedutils/drivers/threading/thread.hpp"
#include <tests/harness/test_runtime_harness.hpp>

#if defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
#include "FreeRTOS.h"
#include "task.h"
#endif    // MBEDUTILS_TEST_RUNTIME_FREERTOS

namespace TestHarness
{
  /*---------------------------------------------------------------------------
  Private Data
  ---------------------------------------------------------------------------*/
  static mb::thread::Internal::ControlBlockStorage<32> s_control_blocks;

#if defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
  static int                                  s_argc          = 0;
  static char                               **s_argv          = nullptr;
  static int                                  s_testResult    = 0;
  static bool                                 s_testsComplete = false;
  static mb::thread::Task::Storage<32 * 1024> s_task_storage;
#endif    // MBEDUTILS_TEST_RUNTIME_FREERTOS

  /*---------------------------------------------------------------------------
  Private Functions
  ---------------------------------------------------------------------------*/
#if defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
  /**
   * @brief Core test thread for FreeRTOS based tests
   *
   * @param arg Unused
   */
  static void _freertos_test_thread( void *arg )
  {
    s_testResult    = RUN_ALL_TESTS( s_argc, s_argv );
    s_testsComplete = true;
    vTaskDelete( nullptr );
  }
#endif    // MBEDUTILS_TEST_RUNTIME_FREERTOS

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

#if defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
  /**
   * @brief FreeRTOS idle hook to exit the test suite when complete
   */
  extern "C" void vApplicationIdleHook( void )
  {
    if( TestHarness::s_testsComplete )
    {
      exit( TestHarness::s_testResult );
    }
  }
#endif    // MBEDUTILS_TEST_RUNTIME_FREERTOS

  int runTests( int argc, char **argv )
  {
    using namespace mb::thread;
    int result = -1;

    /*-------------------------------------------------------------------------
    Power up the thread driver
    -------------------------------------------------------------------------*/
#if defined( INTEGRATION_TEST ) || defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
    Internal::ModuleConfig cfg;
    cfg.tsk_control_blocks = &s_control_blocks;
    mb::thread::driver_setup( cfg );
#endif    // INTEGRATION_TEST || MBEDUTILS_TEST_RUNTIME_FREERTOS

#if defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
    s_argc = argc;
    s_argv = argv;

    /*-------------------------------------------------------------------------
    Construct the thread
    -------------------------------------------------------------------------*/
    Task::Config thread_cfg;
    thread_cfg.reset();

    thread_cfg.id              = 0;
    thread_cfg.name            = "MainTestThread";
    thread_cfg.priority        = 1;
    thread_cfg.func            = _freertos_test_thread;
    thread_cfg.user_data       = nullptr;
    thread_cfg.affinity        = 0;
    thread_cfg.stack_buf       = s_task_storage.stack;
    thread_cfg.stack_size      = sizeof( s_task_storage.stack ) / sizeof( s_task_storage.stack[ 0 ] );
    thread_cfg.block_on_create = false;

    mb::thread::create( thread_cfg );

    /*-------------------------------------------------------------------------
    Start the FreeRTOS scheduler. Only returns if the scheduler is manually
    stopped somewhere in the test suite.
    -------------------------------------------------------------------------*/
    mb::thread::startScheduler();
    result = s_testResult;
#else
    /*-------------------------------------------------------------------------
    Directly call the CppUTest runner (STL Runtime based)
    -------------------------------------------------------------------------*/
    result = RUN_ALL_TESTS( argc, argv );
#endif    // MBEDUTILS_TEST_RUNTIME_FREERTOS

    /*-------------------------------------------------------------------------
    Tear down the thread driver
    -------------------------------------------------------------------------*/
#if defined( INTEGRATION_TEST ) || defined( MBEDUTILS_TEST_RUNTIME_FREERTOS )
    mb::thread::driver_teardown();
#endif    // INTEGRATION_TEST || MBEDUTILS_TEST_RUNTIME_FREERTOS

    return result;
  }
}    // namespace TestHarness

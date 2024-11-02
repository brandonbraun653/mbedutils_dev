/******************************************************************************
 *  File Name:
 *    test_condition.cpp
 *
 *  Description:
 *    Tests the condition variable implementation
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/threading.hpp>
#include <thread>

#include <tests/harness/test_runtime_harness.hpp>
#include <CppUMockGen.hpp>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTestExt/MockSupport.h>
#include "mbedutils/drivers/threading/thread.hpp"
#include "mbedutils/interfaces/mutex_intf.hpp"
#include "mbedutils/interfaces/thread_intf.hpp"

using namespace mb::thread;

/*-----------------------------------------------------------------------------
Test Helpers
-----------------------------------------------------------------------------*/

static int               test_predicate_call_count;
static std::vector<bool> test_predicate_state;

static bool test_predicate()
{
  if( test_predicate_call_count >= test_predicate_state.size() )
  {
    return false;
  }

  return test_predicate_state[ test_predicate_call_count++ ];
}


/*-----------------------------------------------------------------------------
Test Data
-----------------------------------------------------------------------------*/

static ConditionVariable                    test_cv;
static mb::osal::mb_mutex_t                 test_mtx;
static size_t                               test_event_count;
static mb::thread::Task::Storage<32 * 1024> task_storage_1;
static mb::thread::Task::Storage<32 * 1024> task_storage_2;
static mb::thread::Task::Storage<32 * 1024> task_storage_3;

static void condition_var_waiter_thread( void *arg )
{
  test_cv.wait( test_mtx );
  test_event_count++;
  mb::osal::unlockMutex( test_mtx );
}

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return TestHarness::runTests( argc, argv );
}

TEST_GROUP( ConditionVariable_MultiThreading_Integration )
{

  /*---------------------------------------------------------------------------
  Test Setup
  ---------------------------------------------------------------------------*/
  void setup()
  {
    mock().ignoreOtherCalls();

    test_cv.init();
    test_predicate_call_count = 0;
    test_predicate_state.clear();
    test_event_count = 0;

    CHECK( mb::osal::buildMutexStrategy( test_mtx ) );
  }

  /*---------------------------------------------------------------------------
  Test Teardown
  ---------------------------------------------------------------------------*/
  void teardown()
  {
    test_cv.deinit();
    mb::osal::destroyMutex( test_mtx );

    mock().checkExpectations();
    mock().clear();
  }
};

TEST( ConditionVariable_MultiThreading_Integration, notify_one )
{
  /*---------------------------------------------------------------------------
  Construct a thread that's waiting on the condition variable
  ---------------------------------------------------------------------------*/
  Task::Config thread_cfg;
  thread_cfg.reset();

  thread_cfg.id         = 77;
  thread_cfg.name       = "TestThread1";
  thread_cfg.priority   = 1;
  thread_cfg.func       = condition_var_waiter_thread;
  thread_cfg.user_data  = nullptr;
  thread_cfg.affinity   = 0;
  thread_cfg.stack_buf  = task_storage_1.stack;
  thread_cfg.stack_size = sizeof( task_storage_1.stack ) / sizeof( task_storage_1.stack[ 0 ] );

  Task t1 = mb::thread::create( thread_cfg );

  /*---------------------------------------------------------------------------
  Wait for the thread to start, then check the state
  ---------------------------------------------------------------------------*/
  t1.start();
  mb::thread::this_thread::sleep_for( 25 );

  CHECK( t1.joinable() );
  CHECK( test_event_count == 0 );

  /*---------------------------------------------------------------------------
  Notify the waiting thread and validate the result
  ---------------------------------------------------------------------------*/
  test_cv.notify_one();
  t1.join();
  CHECK( test_event_count == 1 );
}

TEST( ConditionVariable_MultiThreading_Integration, notify_all )
{
  /*---------------------------------------------------------------------------
  Construct a thread that's waiting on the condition variable
  ---------------------------------------------------------------------------*/
  Task::Config thread_cfg;
  thread_cfg.reset();

  thread_cfg.id         = 77;
  thread_cfg.name       = "TestThread1";
  thread_cfg.priority   = 1;
  thread_cfg.func       = condition_var_waiter_thread;
  thread_cfg.user_data  = nullptr;
  thread_cfg.affinity   = 0;
  thread_cfg.stack_buf  = task_storage_1.stack;
  thread_cfg.stack_size = sizeof( task_storage_1.stack ) / sizeof( task_storage_1.stack[ 0 ] );

  Task t1 = mb::thread::create( thread_cfg );
  t1.start();

  /* Thread 2 */
  thread_cfg.id         = 78;
  thread_cfg.name       = "TestThread2";
  thread_cfg.stack_buf  = task_storage_2.stack;
  thread_cfg.stack_size = sizeof( task_storage_2.stack ) / sizeof( task_storage_2.stack[ 0 ] );

  Task t2 = mb::thread::create( thread_cfg );
  t2.start();

  /* Thread 3 */
  thread_cfg.id         = 79;
  thread_cfg.name       = "TestThread3";
  thread_cfg.stack_buf  = task_storage_3.stack;
  thread_cfg.stack_size = sizeof( task_storage_3.stack ) / sizeof( task_storage_3.stack[ 0 ] );

  Task t3 = mb::thread::create( thread_cfg );
  t3.start();

  /*---------------------------------------------------------------------------
  Give threads enough time to start and wait on the condition variable
  ---------------------------------------------------------------------------*/
  mb::thread::this_thread::sleep_for( 25 );

  CHECK( t1.joinable() );
  CHECK( t2.joinable() );
  CHECK( t3.joinable() );
  CHECK( test_event_count == 0 );

  /*---------------------------------------------------------------------------
  Notify the waiting thread
  ---------------------------------------------------------------------------*/
  test_cv.notify_all();

  t1.join();
  t2.join();
  t3.join();

  CHECK( test_event_count == 3 );
}
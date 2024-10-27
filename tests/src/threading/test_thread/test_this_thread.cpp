/******************************************************************************
 *  File Name:
 *    test_this_thread.cpp
 *
 *  Description:
 *    Higher level tests for the thread module using the STL interfaces.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/threading.hpp>

#include "mbedutils/drivers/threading/thread.hpp"
#include "mutex_intf_expect.hpp"
#include "assert_expect.hpp"
#include "test_thread_harness.hpp"
#include "thread_intf_expect.hpp"

#include "CppUMockGen.hpp"
#include "CppUTestExt/MockSupport.h"
#include "CppUTest/CommandLineTestRunner.h"

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

using namespace mb::thread;
using namespace CppUMockGen;

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( this_thread )
{
  Internal::ControlBlockStorage<32>         test_control_blocks;
  Task::Storage<4096, TestTaskMessage, 15>  test_task_storage;
  etl::array<Task, 32>                      test_tasks;

  void setup()
  {
    mock().ignoreOtherCalls();

    Internal::ModuleConfig cfg;
    cfg.tsk_control_blocks = &test_control_blocks;

    mb::thread::driver_setup( cfg );
  }

  void teardown()
  {
    mb::thread::driver_teardown();

    mock().checkExpectations();
    mock().clear();
  }
}; /* clang-format on */


static void test_this_thread_basic( void * arg )
{
  ( void )arg;

  CHECK( this_thread::get_name() == "TestThread" );

}

TEST( this_thread, basic_thread_test )
{
  /*---------------------------------------------------------------------------
  Construct the thread
  ---------------------------------------------------------------------------*/
  Task::Config cfg;
  cfg.reset();

  cfg.id                  = 1;
  cfg.name                = "TestThread";
  cfg.priority            = 1;
  cfg.func                = test_this_thread_basic;
  cfg.user_data           = nullptr;
  cfg.affinity            = 0;
  cfg.msg_queue_inst      = &test_task_storage.msg_queue;
  cfg.msg_queue_cfg.pool  = &test_task_storage.msg_queue_storage.pool;
  cfg.msg_queue_cfg.queue = &test_task_storage.msg_queue_storage.queue;
  cfg.stack_buf           = test_task_storage.stack;
  cfg.stack_size          = sizeof( test_task_storage.stack ) / sizeof( test_task_storage.stack[ 0 ] );

  test_tasks[ 0 ] = mb::thread::create( cfg );

  /*---------------------------------------------------------------------------
  Run the test
  ---------------------------------------------------------------------------*/
  CHECK( test_tasks[ 0 ].implementation() != nullptr );
  test_tasks[ 0 ].start();
  test_tasks[ 0 ].join();
}

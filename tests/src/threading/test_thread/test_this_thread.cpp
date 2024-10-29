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
#include <chrono>
#include <thread>
#include <mbedutils/threading.hpp>

#include "mbedutils/drivers/threading/message.hpp"
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
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
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

    /*-------------------------------------------------------------------------
    Prepare the driver to successfully initialize
    -------------------------------------------------------------------------*/
    expect::mb$::osal$::createMutex( IgnoreParameter(), true );
    expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
    expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );

    /*-------------------------------------------------------------------------
    Power up the thread driver
    -------------------------------------------------------------------------*/
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

  /*---------------------------------------------------------------------------
  Check static properties
  ---------------------------------------------------------------------------*/
  CHECK( this_thread::get_name() == "TestThread" );
  CHECK( this_thread::id() == 1 );

  /*---------------------------------------------------------------------------
  Check sleep_for function
  ---------------------------------------------------------------------------*/
  auto current_time = std::chrono::system_clock::now();

  this_thread::sleep_for( 100 );

  auto new_time = std::chrono::system_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>( new_time - current_time ).count();
  CHECK( diff >= 100 );

  /*---------------------------------------------------------------------------
  Check sleep_until function
  ---------------------------------------------------------------------------*/
  current_time = std::chrono::system_clock::now();
  auto time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>( current_time.time_since_epoch() ).count();

  this_thread::sleep_until( time_in_ms + 100 );

  new_time = std::chrono::system_clock::now();
  diff = std::chrono::duration_cast<std::chrono::milliseconds>( new_time - current_time ).count();
  CHECK( ( diff > 95 ) && ( diff < 105 ) );

  /*---------------------------------------------------------------------------
  Check yield function. Really this is just a no-op to ensure no exceptions.
  ---------------------------------------------------------------------------*/
  this_thread::yield();
  CHECK( true );

  /*---------------------------------------------------------------------------
  Check awaitMessage function
  ---------------------------------------------------------------------------*/
  TestTaskMessage rcv_msg;
  memset( &rcv_msg, 0, sizeof( rcv_msg ) );

  Message msg;
  msg.data     = &rcv_msg;
  msg.size     = sizeof( rcv_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  CHECK( !this_thread::awaitMessage( msg, 100 ) );
  CHECK( rcv_msg.id == 42 );
  CHECK( rcv_msg.data == 0x1234 );
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
  CHECK( test_tasks[ 0 ].implementation() );
  test_tasks[ 0 ].start();

  /* Wait a bit for the simple checks to run */
  std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );

  TestTaskMessage send_msg;
  memset( &send_msg, 0, sizeof( send_msg ) );
  send_msg.id   = 42;
  send_msg.data = 0x1234;

  Message msg;
  msg.data     = &send_msg;
  msg.size     = sizeof( send_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  CHECK( mb::thread::sendMessage( 1, msg, 100 ) );

  test_tasks[ 0 ].join();
}

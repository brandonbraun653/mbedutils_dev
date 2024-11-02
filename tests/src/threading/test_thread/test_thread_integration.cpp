/******************************************************************************
 *  File Name:
 *    test_this_thread.cpp
 *
 *  Description:
 *    Integration tests for the thread module using the STL interfaces.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstddef>
#include <mbedutils/threading.hpp>

#include "mbedutils/drivers/threading/message.hpp"
#include "mbedutils/drivers/threading/thread.hpp"
#include "mbedutils/interfaces/time_intf.hpp"
#include "mutex_intf_expect.hpp"
#include "assert_expect.hpp"
#include "test_thread_harness.hpp"
#include "thread_intf_expect.hpp"

#include <tests/harness/test_runtime_harness.hpp>
#include <CppUMockGen.hpp>
#include <CppUTestExt/MockSupport.h>
#include <CppUTest/CommandLineTestRunner.h>

/*-----------------------------------------------------------------------------
Constants
-----------------------------------------------------------------------------*/

static constexpr size_t BASIC_THREAD_MESSAGE_TIMEOUT_MS = 100;

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

using namespace mb::thread;
using namespace CppUMockGen;

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return TestHarness::runTests( argc, argv );
}

/* clang-format off */
TEST_GROUP( this_thread )
{
  Task::Storage<4096, TestTaskMessage, 15>  test_task_storage;
  Task::Storage<4096, TestTaskMessage, 15>  test_task_storage2;
  etl::array<Task, 32>                      test_tasks;

  void setup()
  {
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }
}; /* clang-format on */


/*-----------------------------------------------------------------------------
Basic Test: Inspect properties and send a single message
-----------------------------------------------------------------------------*/
static TestTaskMessage rcv_msg;
static size_t          rx_diff;
static TaskName        test_name;
static TaskId          test_id;
static size_t          test_sleep_for_time;
static size_t          test_sleep_until_time;

static void test_this_thread_basic( void *arg )
{
  ( void )arg;

  /*---------------------------------------------------------------------------
  Check awaitMessage function
  ---------------------------------------------------------------------------*/
  memset( &rcv_msg, 0, sizeof( rcv_msg ) );

  Message msg;
  msg.data     = &rcv_msg;
  msg.size     = sizeof( rcv_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  size_t start_time = mb::time::millis();
  this_thread::awaitMessage( msg, BASIC_THREAD_MESSAGE_TIMEOUT_MS );
  rx_diff = mb::time::millis() - start_time;

  /*---------------------------------------------------------------------------
  Check static properties
  ---------------------------------------------------------------------------*/
  test_name = this_thread::get_name();
  test_id   = this_thread::id();

  /*---------------------------------------------------------------------------
  Check sleep_for function
  ---------------------------------------------------------------------------*/
  auto current_time = mb::time::millis();
  this_thread::sleep_for( 100 );
  test_sleep_for_time = mb::time::millis() - current_time;

  /*---------------------------------------------------------------------------
  Check sleep_until function
  ---------------------------------------------------------------------------*/
  current_time = mb::time::millis();
  this_thread::sleep_until( current_time + 100 );
  test_sleep_until_time = mb::time::millis() - current_time;

  /*---------------------------------------------------------------------------
  Check yield function. Really this is just a no-op to ensure no exceptions.
  ---------------------------------------------------------------------------*/
  this_thread::yield();
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
  CHECK( test_tasks[ 0 ].id() != TASK_ID_INVALID );
  test_tasks[ 0 ].start();

  /*---------------------------------------------------------------------------
  Send a message to the thread, but do it before the thread is ready to receive
  ---------------------------------------------------------------------------*/
  mb::thread::this_thread::sleep_for( BASIC_THREAD_MESSAGE_TIMEOUT_MS / 2 );

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

  /*---------------------------------------------------------------------------
  Wait for the thread to finish
  ---------------------------------------------------------------------------*/
  test_tasks[ 0 ].join();

  /*---------------------------------------------------------------------------
  Validate the results
  ---------------------------------------------------------------------------*/
  size_t min_sleep_with_tolerance = ( BASIC_THREAD_MESSAGE_TIMEOUT_MS / 2 ) - 5;

  CHECK( rx_diff >= min_sleep_with_tolerance );           // Should have blocked for a bit
  CHECK( rx_diff <= BASIC_THREAD_MESSAGE_TIMEOUT_MS );    // Should have received the message in time
  CHECK( rcv_msg.id == 42 );
  CHECK( rcv_msg.data == 0x1234 );
  CHECK( test_name == "TestThread" );
  CHECK( test_id == 1 );
  CHECK( test_sleep_for_time >= 100 );
  CHECK( test_sleep_until_time > 95 );
}


/*-----------------------------------------------------------------------------
Basic Test: Send multiple messages and ensure they are enqueued
-----------------------------------------------------------------------------*/

static constexpr size_t MULTI_MESSAGE_COUNT      = 10;
static constexpr size_t MULTI_MESSAGE_TIMEOUT_MS = 25;
static size_t           s_multi_message_count    = 0;

static void test_this_thread_multi_message_enqueued( void *arg )
{
  ( void )arg;

  TestTaskMessage rcv_msg;
  memset( &rcv_msg, 0, sizeof( rcv_msg ) );

  Message msg;
  msg.data     = &rcv_msg;
  msg.size     = sizeof( rcv_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  while( this_thread::awaitMessage( msg, MULTI_MESSAGE_TIMEOUT_MS ) )
  {
    s_multi_message_count++;
  }
}


TEST( this_thread, multi_message_enqueued )
{
  /*---------------------------------------------------------------------------
  Construct the thread
  ---------------------------------------------------------------------------*/
  Task::Config cfg;
  cfg.reset();

  cfg.id                  = 1;
  cfg.name                = "TestThread";
  cfg.priority            = 1;
  cfg.func                = test_this_thread_multi_message_enqueued;
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
  CHECK( test_tasks[ 0 ].id() != TASK_ID_INVALID );
  test_tasks[ 0 ].start();

  /*---------------------------------------------------------------------------
  Send a message to the thread, but do it before the thread is ready to receive
  ---------------------------------------------------------------------------*/
  mb::thread::this_thread::sleep_for( MULTI_MESSAGE_TIMEOUT_MS / 2 );

  TestTaskMessage send_msg;
  memset( &send_msg, 0, sizeof( send_msg ) );
  send_msg.id   = 42;
  send_msg.data = 0x1234;

  Message msg;
  msg.data     = &send_msg;
  msg.size     = sizeof( send_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  for( size_t i = 0; i < MULTI_MESSAGE_COUNT; i++ )
  {
    CHECK( mb::thread::sendMessage( 1, msg, 100 ) );
  }

  /*---------------------------------------------------------------------------
  Wait for the thread to finish
  ---------------------------------------------------------------------------*/
  test_tasks[ 0 ].join();
  CHECK( s_multi_message_count == MULTI_MESSAGE_COUNT );
}


/*-----------------------------------------------------------------------------
Basic Test: Send multiple messages and look for a specific one
-----------------------------------------------------------------------------*/
static int s_message_22_count;
static int s_message_33_count;

static bool test_predicate_22( const Message &msg )
{
  TestTaskMessage *data = reinterpret_cast<TestTaskMessage *>( msg.data );
  return data->id == 22;
}

static bool test_predicate_33( const Message &msg )
{
  TestTaskMessage *data = reinterpret_cast<TestTaskMessage *>( msg.data );
  return data->id == 33;
}

static void test_thread_consume_message_id_22( void *arg )
{
  ( void )arg;

  TestTaskMessage rcv_msg;
  memset( &rcv_msg, 0, sizeof( rcv_msg ) );

  Message msg;
  msg.data     = &rcv_msg;
  msg.size     = sizeof( rcv_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  auto predicate = MessagePredicate::create<test_predicate_22>();
  while( this_thread::awaitMessage( msg, predicate, MULTI_MESSAGE_TIMEOUT_MS ) )
  {
    s_message_22_count++;
  }
}

static void test_thread_consume_message_id_33( void *arg )
{
  ( void )arg;

  TestTaskMessage rcv_msg;
  memset( &rcv_msg, 0, sizeof( rcv_msg ) );

  Message msg;
  msg.data     = &rcv_msg;
  msg.size     = sizeof( rcv_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  auto predicate = MessagePredicate::create<test_predicate_33>();

  while( this_thread::awaitMessage( msg, predicate, MULTI_MESSAGE_TIMEOUT_MS ) )
  {
    s_message_33_count++;
  }
}

TEST( this_thread, multi_message_predicate )
{
  /*---------------------------------------------------------------------------
  Construct the threads
  ---------------------------------------------------------------------------*/
  Task::Config cfg;
  cfg.reset();

  cfg.id                  = 1;
  cfg.name                = "TestThread1";
  cfg.priority            = 1;
  cfg.func                = test_thread_consume_message_id_22;
  cfg.user_data           = nullptr;
  cfg.affinity            = 0;
  cfg.msg_queue_inst      = &test_task_storage.msg_queue;
  cfg.msg_queue_cfg.pool  = &test_task_storage.msg_queue_storage.pool;
  cfg.msg_queue_cfg.queue = &test_task_storage.msg_queue_storage.queue;
  cfg.stack_buf           = test_task_storage.stack;
  cfg.stack_size          = sizeof( test_task_storage.stack ) / sizeof( test_task_storage.stack[ 0 ] );

  test_tasks[ 0 ] = mb::thread::create( cfg );

  cfg.id                  = 2;
  cfg.name                = "TestThread2";
  cfg.func                = test_thread_consume_message_id_33;
  cfg.user_data           = nullptr;
  cfg.affinity            = 0;
  cfg.msg_queue_inst      = &test_task_storage2.msg_queue;
  cfg.msg_queue_cfg.pool  = &test_task_storage2.msg_queue_storage.pool;
  cfg.msg_queue_cfg.queue = &test_task_storage2.msg_queue_storage.queue;
  cfg.stack_buf           = test_task_storage2.stack;
  cfg.stack_size          = sizeof( test_task_storage2.stack ) / sizeof( test_task_storage2.stack[ 0 ] );

  test_tasks[ 1 ] = mb::thread::create( cfg );

  /*---------------------------------------------------------------------------
  Run the test
  ---------------------------------------------------------------------------*/
  CHECK( test_tasks[ 0 ].id() != TASK_ID_INVALID );
  CHECK( test_tasks[ 1 ].id() != TASK_ID_INVALID );
  test_tasks[ 0 ].start();
  test_tasks[ 1 ].start();

  /*---------------------------------------------------------------------------
  Send a message to the thread, but do it before the thread is ready to receive
  ---------------------------------------------------------------------------*/
  mb::thread::this_thread::sleep_for( MULTI_MESSAGE_TIMEOUT_MS / 2 );

  TestTaskMessage send_msg;
  memset( &send_msg, 0, sizeof( send_msg ) );
  send_msg.id   = 22;
  send_msg.data = 0x1234;

  Message msg;
  msg.data     = &send_msg;
  msg.size     = sizeof( send_msg );
  msg.sender   = this_thread::id();
  msg.priority = 0;

  for( size_t i = 0; i < MULTI_MESSAGE_COUNT; i++ )
  {
    send_msg.id = 22;
    CHECK( mb::thread::sendMessage( 1, msg, 100 ) );
    send_msg.id = 33;
    CHECK( mb::thread::sendMessage( 2, msg, 100 ) );
  }

  /*---------------------------------------------------------------------------
  Wait for the threads to finish
  ---------------------------------------------------------------------------*/
  test_tasks[ 0 ].join();
  test_tasks[ 1 ].join();
  CHECK( s_message_22_count == MULTI_MESSAGE_COUNT );
  CHECK( s_message_33_count == MULTI_MESSAGE_COUNT );
}

/******************************************************************************
 *  File Name:
 *    test_message.cpp
 *
 *  Description:
 *    Higher level tests for the core thread messaging
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/threading.hpp>

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

using namespace mb::thread;

/*-----------------------------------------------------------------------------
Harness
-----------------------------------------------------------------------------*/

static constexpr size_t TSK_MSG_POOL_DEPTH = 10;

enum TestMsgId : MessageId
{
  TEST_MSG_ID_1 = 0,
  TEST_MSG_ID_2,
  TEST_MSG_ID_3,
  TEST_MSG_ID_4,
  TEST_MSG_ID_5
};


struct TestTaskMsg
{
  MessageId id; /**< Message identifier */
  union
  {
    uint32_t data;
    struct
    {
      uint8_t a;
      uint8_t b;
      uint8_t c;
      uint8_t d;
    };
  } payload;
};

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( MessageQueue )
{

  MessageQueue::Storage<TestTaskMsg, TSK_MSG_POOL_DEPTH> test_storage;
  MessageQueue* test_queue;

  void setup()
  {
    mock().ignoreOtherCalls();

    test_queue = new MessageQueue();

    /*-------------------------------------------------------------------------
    Configure the message queue for testing
    -------------------------------------------------------------------------*/
    MessageQueue::Config cfg;
    cfg.pool  = &test_storage.pool;
    cfg.queue = &test_storage.queue;
    test_queue->configure( cfg );
  }

  void teardown()
  {

    mock().checkExpectations();
    mock().clear();

    delete test_queue;
  }
}; /* clang-format on */


TEST( MessageQueue, calling_members_before_configuration )
{
  MessageQueue q;
  Message      msg;

  CHECK( q.push( msg ) == false );
  CHECK( q.pop( msg ) == false );
  CHECK( q.peek( msg ) == false );
  CHECK( q.empty() == true );
  CHECK( q.size() == 0 );
}

TEST( MessageQueue, configure_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Prepare the test
  ---------------------------------------------------------------------------*/
  MessageQueue         q;
  MessageQueue::Config cfg;
  TestTaskMsg          msg;
  Message              tsk_msg;

  tsk_msg.data = &msg;
  tsk_msg.size = sizeof( TestTaskMsg );

  /*---------------------------------------------------------------------------
  Default configuration
  ---------------------------------------------------------------------------*/
  q.configure( cfg );
  CHECK( q.push( tsk_msg ) == false );

  /*---------------------------------------------------------------------------
  Bad pool configuration
  ---------------------------------------------------------------------------*/
  cfg.pool  = nullptr;
  cfg.queue = &test_storage.queue;
  q.configure( cfg );
  CHECK( q.push( tsk_msg ) == false );

  /*---------------------------------------------------------------------------
  Bad queue configuration
  ---------------------------------------------------------------------------*/
  cfg.pool  = &test_storage.pool;
  cfg.queue = nullptr;
  q.configure( cfg );
  CHECK( q.push( tsk_msg ) == false );
}

TEST( MessageQueue, pop_peek_empty_queue )
{
  Message msg;
  CHECK( test_queue->pop( msg ) == false );
  CHECK( test_queue->peek( msg ) == false );
}

TEST( MessageQueue, push_pop_single_message )
{
  Message     push_msg;
  Message     pop_msg;
  TestTaskMsg push_data;
  TestTaskMsg pop_data;

  /*---------------------------------------------------------------------------
  Prepare the test
  ---------------------------------------------------------------------------*/
  push_data.id    = TEST_MSG_ID_1;
  push_msg.data = &push_data;
  push_msg.size = sizeof( TestTaskMsg );

  /*---------------------------------------------------------------------------
  Push the message onto the queue
  ---------------------------------------------------------------------------*/
  CHECK( test_queue->push( push_msg ) == true );
  CHECK( test_queue->empty() == false );
  CHECK( test_queue->size() == 1 );

  /*---------------------------------------------------------------------------
  Pop the message off the queue
  ---------------------------------------------------------------------------*/
  pop_msg.data = &pop_data;
  pop_msg.size = sizeof( TestTaskMsg );

  CHECK( test_queue->pop( pop_msg ) == true );
  CHECK( test_queue->empty() == true );
  CHECK( test_queue->size() == 0 );

  /*---------------------------------------------------------------------------
  Verify the message contents
  ---------------------------------------------------------------------------*/
  CHECK( memcmp( push_msg.data, pop_msg.data, push_msg.size ) == 0 );
  CHECK( push_msg.size == pop_msg.size );
  CHECK( push_msg.sender == pop_msg.sender );
  CHECK( push_msg.priority == pop_msg.priority );
}

TEST( MessageQueue, multi_message_with_priority )
{
  etl::array<TestTaskMsg, 5> test_data;
  etl::vector<Message, 5>    test_msgs;

  for( size_t i = 0; i < test_data.size(); i++ )
  {
    test_data[i].id = static_cast<MessageId>( i );
    test_msgs.push_back( Message() );
    test_msgs.back().data = &test_data[i];
    test_msgs.back().size = sizeof( TestTaskMsg );
    test_msgs.back().priority = rand();
  }

  /*---------------------------------------------------------------------------
  Push the messages onto the queue, initially unsorted
  ---------------------------------------------------------------------------*/
  for( auto& msg : test_msgs )
  {
    CHECK( test_queue->push( msg ) == true );
  }

  /*---------------------------------------------------------------------------
  Pop the messages off the queue, they should be sorted by priority
  ---------------------------------------------------------------------------*/
  etl::sort( test_msgs.begin(), test_msgs.end(), Internal::TskMsgCompare() );

  for( auto &msg : test_msgs )
  {
    Message     pop_msg;
    TestTaskMsg pop_data;

    pop_msg.data = &pop_data;
    pop_msg.size = sizeof( TestTaskMsg );

    CHECK( test_queue->pop( pop_msg ) == true );
    CHECK( memcmp( msg.data, pop_msg.data, msg.size ) == 0 );
    CHECK( msg.size == pop_msg.size );
    CHECK( msg.sender == pop_msg.sender );
    CHECK( msg.priority == pop_msg.priority );
  }
}
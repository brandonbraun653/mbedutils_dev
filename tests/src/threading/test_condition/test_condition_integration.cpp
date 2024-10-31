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

#include "CppUMockGen.hpp"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"
#include "mbedutils/interfaces/mutex_intf.hpp"

using namespace mb::thread;

/*-----------------------------------------------------------------------------
Test Helpers
-----------------------------------------------------------------------------*/

static int               test_predicate_call_count;
static std::vector<bool> test_predicate_state;
static bool              test_predicate()
{
  if( test_predicate_call_count >= test_predicate_state.size() )
  {
    return false;
  }

  return test_predicate_state[ test_predicate_call_count++ ];
}

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

TEST_GROUP( ConditionVariable_Int_STL )
{
  /*---------------------------------------------------------------------------
  Test Data
  ---------------------------------------------------------------------------*/
  ConditionVariable    test_cv;
  mb::osal::mb_mutex_t test_mtx;
  size_t               test_event_count;

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

TEST( ConditionVariable_Int_STL, notify_one )
{
  /*---------------------------------------------------------------------------
  Construct a thread that's waiting on the condition variable
  ---------------------------------------------------------------------------*/
  test_event_count = 0;

  std::thread t1( [ & ]() {
    test_cv.wait( test_mtx );
    test_event_count++;
    mb::osal::unlockMutex( test_mtx );
  } );

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

  CHECK( t1.joinable() );
  CHECK( test_event_count == 0 );

  /*---------------------------------------------------------------------------
  Notify the waiting thread
  ---------------------------------------------------------------------------*/
  test_cv.notify_one();

  t1.join();
  CHECK( test_event_count == 1 );
}

TEST( ConditionVariable_Int_STL, notify_all )
{
  /*---------------------------------------------------------------------------
  Construct a thread that's waiting on the condition variable
  ---------------------------------------------------------------------------*/
  test_event_count = 0;

  std::thread t1( [ & ]() {
    test_cv.wait( test_mtx );
    test_event_count++;
    mb::osal::unlockMutex( test_mtx );
  } );

  std::thread t2( [ & ]() {
    test_cv.wait( test_mtx );
    test_event_count++;
    mb::osal::unlockMutex( test_mtx );
  } );

  std::thread t3( [ & ]() {
    test_cv.wait( test_mtx );
    test_event_count++;
    mb::osal::unlockMutex( test_mtx );
  } );

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

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
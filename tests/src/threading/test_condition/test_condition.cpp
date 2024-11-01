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

#include <mutex_intf_expect.hpp>
#include <smphr_intf_expect.hpp>

#include <CppUMockGen.hpp>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTestExt/MockSupport.h>
#include "mbedutils/interfaces/mutex_intf.hpp"

using namespace mb::thread;
using namespace CppUMockGen;

/*-----------------------------------------------------------------------------
Test Helpers
-----------------------------------------------------------------------------*/

static int test_predicate_call_count;
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
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

TEST_GROUP( ConditionVariable )
{
  /*---------------------------------------------------------------------------
  Test Data
  ---------------------------------------------------------------------------*/
  ConditionVariable    test_cv;
  mb::osal::mb_mutex_t test_mtx;

  /*---------------------------------------------------------------------------
  Test Setup
  ---------------------------------------------------------------------------*/
  void setup()
  {
    mock().ignoreOtherCalls();

    /*-------------------------------------------------------------------------
    Setup
    -------------------------------------------------------------------------*/
    expect::mb$::osal$::createMutex( IgnoreParameter(), true );
    expect::mb$::osal$::createSmphr( IgnoreParameter(), 1, 0, true );

    test_cv.init();
    test_predicate_call_count = 0;
    test_predicate_state.clear();
  }

  /*---------------------------------------------------------------------------
  Test Teardown
  ---------------------------------------------------------------------------*/
  void teardown()
  {
    /*-------------------------------------------------------------------------
    Teardown
    -------------------------------------------------------------------------*/
    expect::mb$::osal$::destroyMutex( IgnoreParameter() );
    expect::mb$::osal$::destroySmphr( IgnoreParameter() );

    test_cv.deinit();

    mock().checkExpectations();
    mock().clear();
  }

};

TEST( ConditionVariable, init_then_deinit )
{
  ConditionVariable cv;

  /*---------------------------------------------------------------------------
  Setup
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::osal$::createSmphr( IgnoreParameter(), 1, 0, true );

  /*---------------------------------------------------------------------------
  Test first initialization
  ---------------------------------------------------------------------------*/
  cv.init();
  mock().checkExpectations();
  mock().clear();

  /*---------------------------------------------------------------------------
  Test re-initialization
  ---------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::osal::createMutex" );
  mock().expectNoCall( "mb::osal::createSmphr" );

  cv.init();
  mock().checkExpectations();
  mock().clear();

  /*---------------------------------------------------------------------------
  Test deinitialization
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::destroyMutex( IgnoreParameter() );
  expect::mb$::osal$::destroySmphr( IgnoreParameter() );

  cv.deinit();
  mock().checkExpectations();
  mock().clear();

  /*---------------------------------------------------------------------------
  Test re-deinitialization
  ---------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::osal::destroyMutex" );
  mock().expectNoCall( "mb::osal::destroySmphr" );

  cv.deinit();
  mock().checkExpectations();
  mock().clear();
}

TEST( ConditionVariable, wait )
{
  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::acquireSmphr( 1, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );

  test_cv.wait( test_mtx );
}

TEST( ConditionVariable, wait_predicate )
{
  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::acquireSmphr( 1, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );
  test_predicate_state.push_back( false );
  test_predicate_state.push_back( true );

  test_cv.wait( test_mtx, cv_predicate::create<test_predicate>() );
}

TEST( ConditionVariable, wait_for )
{
  /*---------------------------------------------------------------------------
  Test: Timeout
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::tryAcquireSmphr( 1, IgnoreParameter(), 100, false );

  auto return_status = test_cv.wait_for( test_mtx, 100 );
  CHECK( cv_status::timeout == return_status );

  mock().checkExpectations();
  mock().clear();
  mock().ignoreOtherCalls();

  /*---------------------------------------------------------------------------
  Test: No timeout
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::tryAcquireSmphr( 1, IgnoreParameter(), 100, true );

  return_status = test_cv.wait_for( test_mtx, 100 );
  CHECK( cv_status::no_timeout == return_status );

  mock().checkExpectations();
  mock().clear();
}

TEST( ConditionVariable, wait_for_predicate_timeout )
{
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::tryAcquireSmphr( 1, IgnoreParameter(), 100, false );

  test_predicate_state.push_back( false ); // First call.
  test_predicate_state.push_back( true ); // Second call, but we never hit this because of the timeout.

  auto return_status = test_cv.wait_for( test_mtx, 100, cv_predicate::create<test_predicate>() );
  CHECK( false == return_status );
}

TEST( ConditionVariable, wait_for_predicate_no_timeout )
{
  expect::mb$::osal$::lockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::unlockMutex( 3, IgnoreParameter() );
  expect::mb$::osal$::tryAcquireSmphr( 1, IgnoreParameter(), 100, true );

  test_predicate_state.push_back( false );  // First call. Invokes the semaphore wait.
  test_predicate_state.push_back( true ); // Second call. Predicate is now true. Breaks the loop.

  auto return_status = test_cv.wait_for( test_mtx, 100, cv_predicate::create<test_predicate>() );
  CHECK( true == return_status );
}

TEST( ConditionVariable, notify_one )
{
  /*---------------------------------------------------------------------------
  This test is a bit tricky. We require an actual thread to check the true
  notification behavior. Best we can do here is validate we don't release a
  semaphore if we're not waiting on it.
  ---------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::osal::releaseSmphr" );
  test_cv.notify_one();
}

TEST( ConditionVariable, notify_all )
{
  /*---------------------------------------------------------------------------
  This test is a bit tricky. We require an actual thread to check the true
  notification behavior. Best we can do here is validate we don't release a
  semaphore if we're not waiting on it.
  ---------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::osal::releaseSmphr" );
  test_cv.notify_all();
}

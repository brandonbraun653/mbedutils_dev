/******************************************************************************
 *  File Name:
 *    test_thread_stl.cpp
 *
 *  Description:
 *    Tests the STL based thread implementation for Mbedutils
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/interfaces/thread_intf.hpp>
#include <atomic>
#include <chrono>
#include <thread>

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

/* clang-format off */
TEST_GROUP(ThreadInterface)
{
  void setup()
  {
    mb::thread::intf::initialize();
  }

  void teardown()
  {
    // Cleanup code if necessary
  }
};
/* clang-format on */

TEST( ThreadInterface, CreateAndDestroyTask )
{
  std::atomic<bool> task_executed{ false };

  mb::thread::TaskConfig cfg;
  cfg.func = []( void *user_data ) {
    auto *executed = static_cast<std::atomic<bool> *>( user_data );
    *executed      = true;
  };
  cfg.user_data = &task_executed;

  auto handle = mb::thread::intf::create_task( cfg );
  CHECK( handle != nullptr );

  // Give some time for the task to execute
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

  CHECK( task_executed == true );

  mb::thread::intf::destroy_task( handle );
}

TEST( ThreadInterface, DestroyNonExistentTask )
{
  mb::thread::TaskHandle invalid_handle = reinterpret_cast<mb::thread::TaskHandle>( 0xDEADBEEF );
  mb::thread::intf::destroy_task( invalid_handle );

  // No crash or exception should occur
  CHECK( true );
}

TEST( ThreadInterface, SetAffinity )
{
  std::atomic<bool> task_executed{ false };

  mb::thread::TaskConfig cfg;
  cfg.func = []( void *user_data ) {
    auto *executed = static_cast<std::atomic<bool> *>( user_data );
    *executed      = true;
  };
  cfg.user_data = &task_executed;

  auto handle = mb::thread::intf::create_task( cfg );
  CHECK( handle != nullptr );

  mb::thread::intf::set_affinity( handle, 0 );

  // Give some time for the task to execute
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

  CHECK( task_executed == true );

  mb::thread::intf::destroy_task( handle );
}

TEST( ThreadInterface, SchedulerStart )
{
  mb::thread::intf::start_scheduler();

  // Since the scheduler is a no-op, just ensure no crash occurs
  CHECK( true );
}

TEST( ThreadInterface, OnStackOverflow )
{
  CHECK_THROWS( std::runtime_error, mb::thread::intf::on_stack_overflow() );
}

TEST( ThreadInterface, OnMallocFailed )
{
  CHECK_THROWS( std::runtime_error, mb::thread::intf::on_malloc_failed() );
}

TEST( ThreadInterface, OnIdle )
{
  // Ensure no exception is thrown
  mb::thread::intf::on_idle();
  CHECK( true );
}

TEST( ThreadInterface, OnTick )
{
  // Ensure no exception is thrown
  mb::thread::intf::on_tick();
  CHECK( true );
}

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

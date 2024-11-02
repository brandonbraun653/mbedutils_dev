/******************************************************************************
 *  File Name:
 *    test_intf_thread.cpp
 *
 *  Description:
 *    Tests the STL based thread implementation for Mbedutils. The FreeRTOS
 *    implementation is quite difficult to test in a unit test environment and
 *    we rely on the integration tests to validate that implementation.
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

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP(ThreadInterface)
{
  void setup()
  {
    mb::thread::intf::driver_setup();
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

  mb::thread::Task::Config cfg;
  cfg.func = []( void *user_data ) {
    auto *executed = static_cast<std::atomic<bool> *>( user_data );
    *executed      = true;
  };
  cfg.user_data = &task_executed;

  auto handle = mb::thread::intf::create_task( cfg );
  CHECK( handle );
  mb::thread::intf::start_scheduler();

  // Give some time for the task to execute
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

  CHECK( task_executed == true );

  mb::thread::intf::destroy_task( handle );
}

TEST( ThreadInterface, DestroyNonExistentTask )
{
  mb::thread::intf::destroy_task( 0xDEADBEEF );

  // No crash or exception should occur
  CHECK( true );
}

TEST( ThreadInterface, SetAffinity )
{
  std::atomic<bool> task_executed{ false };

  mb::thread::Task::Config cfg;
  cfg.func = []( void *user_data ) {
    auto *executed = static_cast<std::atomic<bool> *>( user_data );
    *executed      = true;
  };
  cfg.user_data = &task_executed;

  auto handle = mb::thread::intf::create_task( cfg );
  CHECK( handle );

  mb::thread::intf::set_affinity( handle, 0 );
  mb::thread::intf::start_scheduler();

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

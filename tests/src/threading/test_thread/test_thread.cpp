/******************************************************************************
 *  File Name:
 *    test_thread.cpp
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
#include "thread_intf_expect.hpp"

#include "test_thread_harness.hpp"
#include <CppUMockGen.hpp>
#include <CppUTestExt/MockSupport.h>
#include <CppUTest/CommandLineTestRunner.h>

/*-----------------------------------------------------------------------------
Task implementation stub, since this particular test doesn't care about the
implementation details.
-----------------------------------------------------------------------------*/
namespace mb::thread
{
  Task::Task() noexcept : mId( TASK_ID_INVALID ), mHandle( 0 )
  {
  }


  Task::~Task()
  {
  }


  Task &Task::operator=( Task &&other ) noexcept
  {
    this->mId     = other.mId;
    this->mHandle = other.mHandle;
    return *this;
  }


  void Task::start()
  {
  }


  void Task::kill()
  {
  }


  void Task::join()
  {
  }


  bool Task::joinable()
  {
    return false;
  }


  TaskId Task::id() const
  {
    return mId;
  }


  TaskName Task::name() const
  {
    return mName;
  }


  TaskHandle Task::implementation() const
  {
    return mHandle;
  }

  namespace this_thread
  {
    void yield()
    {
    }

    TaskId id()
    {
      return 0;
    }
  }
}    // namespace mb::thread

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

using namespace mb::thread;
using namespace CppUMockGen;

int main( int argc, char **argv )
{
  TaskConfigComparator taskConfigComparator;
  mock().installComparator( "mb::thread::Task::Config", taskConfigComparator );

  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( thread_module )
{

  Internal::ControlBlockStorage<32> test_control_blocks;

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


TEST( thread_module, initialization_mutex_failure )
{
  Internal::ModuleConfig cfg;
  mb::thread::driver_teardown();
  mock().clear();

  /*-------------------------------------------------------------------------
  Unable to build a mutex for the module
  -------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), false );
  expect::mb$::assert$::log_assert_failure( false, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), false );
  mock().expectNoCall( "mb::thread::intf::driver_setup" );

  mb::thread::driver_setup( cfg );
}

TEST( thread_module, initialization_control_blocks_failure )
{
  Internal::ModuleConfig cfg;
  mb::thread::driver_teardown();
  mock().clear();

  /*---------------------------------------------------------------------------
  Mutex succeeds, but now the control blocks are bad
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( false, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), false );
  mock().expectNoCall( "mb::thread::intf::driver_setup" );

  cfg.tsk_control_blocks = nullptr;
  mb::thread::driver_setup( cfg );
}

TEST( thread_module, initialization_success )
{
  Internal::ModuleConfig cfg;
  mb::thread::driver_teardown();
  mock().clear();

  /*---------------------------------------------------------------------------
  Everything is good to go
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::thread$::intf$::driver_setup();

  Internal::ControlBlockStorage<32> map;
  cfg.tsk_control_blocks = &map;
  mb::thread::driver_setup( cfg );
}

TEST( thread_module, initialization_cannot_be_called_after_success )
{
  Internal::ModuleConfig cfg;
  mb::thread::driver_teardown();
  mock().clear();

  /*---------------------------------------------------------------------------
  Everything is good to go
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::thread$::intf$::driver_setup();

  Internal::ControlBlockStorage<32> map;
  map.insert( { 1, {} } );
  CHECK( map.size() == 1 );

  cfg.tsk_control_blocks = &map;

  mb::thread::driver_setup( cfg );
  CHECK( map.size() == 0 );

  /*---------------------------------------------------------------------------
  Try to initialize again
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  mock().expectNoCall( "mb::thread::intf::driver_setup" );

  map.insert( { 1, {} } );
  CHECK( map.size() == 1 );

  mb::thread::driver_setup( cfg );
  CHECK( map.size() == 1 );
}

TEST( thread_module, teardown_not_initialized )
{
  /*---------------------------------------------------------------------------
  Ensure we're not initialized
  ---------------------------------------------------------------------------*/
  mb::thread::driver_teardown();
  mock().clear();

  /*---------------------------------------------------------------------------
  Try to teardown again
  ---------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::thread::intf::driver_teardown" );
  mb::thread::driver_teardown();
}

TEST( thread_module, task_create_not_initialized )
{
  Task::Config cfg;
  mb::thread::driver_teardown();
  mock().clear();

  /*---------------------------------------------------------------------------
  Try to create a task without initializing the module
  ---------------------------------------------------------------------------*/
  Task empty_task;
  CHECK( empty_task.implementation() == 0 );

  empty_task = mb::thread::create( cfg );
  CHECK( empty_task.implementation() == 0 );
}

TEST( thread_module, task_create_simple )
{
  /*---------------------------------------------------------------------------
  Initialize the module
  ---------------------------------------------------------------------------*/
  Internal::ModuleConfig module_cfg;
  module_cfg.tsk_control_blocks = &test_control_blocks;

  /* Reset the driver */
  mb::thread::driver_teardown();

  /* Initialize with this test configuration */
  expect::mb$::osal$::createMutex( IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
  expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );

  mb::thread::driver_setup( module_cfg );
  mock().clear();
  mock().ignoreOtherCalls();

  /*---------------------------------------------------------------------------
  Set up the task configuration
  ---------------------------------------------------------------------------*/
  Task         empty_task;
  Task::Config cfg;
  cfg.id = 1;

  uint32_t pretend_task_handle = 0x1234;
  expect::mb$::thread$::intf$::create_task( cfg, reinterpret_cast<TaskHandle>( &pretend_task_handle ) );

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  Task new_task = mb::thread::create( cfg );
  CHECK( new_task.implementation() );
}

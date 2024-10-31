/******************************************************************************
 *  File Name:
 *    test_mutex_stl.cpp
 *
 *  Description:
 *    Tests the STL based mutex implementation for Mbedutils
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/interfaces/mutex_intf.hpp>

#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTestExt/MockSupport.h>
#include <CppUTestExt/MockSupportPlugin.h>

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( MutexSTLTests )
{
  void setup() override
  {
    mock().ignoreOtherCalls();
    mb::osal::initMutexDriver();
  }

  void teardown() override
  {
    // No teardown needed for now
    mock().checkExpectations();
    mock().clear();
  }
};
/* clang-format on */

TEST( MutexSTLTests, CreateAndDestroyMutex )
{
  mb::osal::mb_mutex_t mutex = nullptr;
  CHECK( mb::osal::createMutex( mutex ) );
  CHECK( mutex != nullptr );
  mb::osal::destroyMutex( mutex );
  CHECK( mutex == nullptr );
}

TEST( MutexSTLTests, AllocateAndDeallocateMutex )
{
  mb::osal::mb_mutex_t mutex = nullptr;
  CHECK( mb::osal::allocateMutex( mutex ) );
  CHECK( mutex != nullptr );
  mb::osal::deallocateMutex( mutex );
  CHECK( mutex == nullptr );
}

TEST( MutexSTLTests, LockAndUnlockMutex )
{
  mb::osal::mb_mutex_t mutex = nullptr;
  mb::osal::createMutex( mutex );
  mb::osal::lockMutex( mutex );
  mb::osal::unlockMutex( mutex );
  mb::osal::destroyMutex( mutex );
}

TEST( MutexSTLTests, TryLockMutex )
{
  mb::osal::mb_mutex_t mutex = nullptr;
  mb::osal::createMutex( mutex );
  CHECK( mb::osal::tryLockMutex( mutex ) );
  mb::osal::unlockMutex( mutex );
  mb::osal::destroyMutex( mutex );
}

TEST( MutexSTLTests, TryLockMutexWithTimeout )
{
  mb::osal::mb_mutex_t mutex = nullptr;
  mb::osal::createMutex( mutex );
  CHECK( mb::osal::tryLockMutex( mutex, 100 ) );
  mb::osal::unlockMutex( mutex );
  mb::osal::destroyMutex( mutex );
}

TEST( MutexSTLTests, AllocateAndDeallocateRecursiveMutex )
{
  mb::osal::mb_recursive_mutex_t mutex = nullptr;
  CHECK( mb::osal::allocateRecursiveMutex( mutex ) );
  CHECK( mutex != nullptr );
  mb::osal::deallocateRecursiveMutex( mutex );
  CHECK( mutex == nullptr );
}

TEST( MutexSTLTests, CreateAndDestroyRecursiveMutex )
{
  mb::osal::mb_recursive_mutex_t mutex = nullptr;
  CHECK( mb::osal::createRecursiveMutex( mutex ) );
  CHECK( mutex != nullptr );
  mb::osal::destroyRecursiveMutex( mutex );
  CHECK( mutex == nullptr );
}

TEST( MutexSTLTests, LockAndUnlockRecursiveMutex )
{
  mb::osal::mb_recursive_mutex_t mutex = nullptr;
  mb::osal::createRecursiveMutex( mutex );
  mb::osal::lockRecursiveMutex( mutex );
  mb::osal::unlockRecursiveMutex( mutex );
  mb::osal::destroyRecursiveMutex( mutex );
}

TEST( MutexSTLTests, TryLockRecursiveMutex )
{
  mb::osal::mb_recursive_mutex_t mutex = nullptr;
  mb::osal::createRecursiveMutex( mutex );
  CHECK( mb::osal::tryLockRecursiveMutex( mutex ) );
  mb::osal::unlockRecursiveMutex( mutex );
  mb::osal::destroyRecursiveMutex( mutex );
}

TEST( MutexSTLTests, TryLockRecursiveMutexWithTimeout )
{
  mb::osal::mb_recursive_mutex_t mutex = nullptr;
  mb::osal::createRecursiveMutex( mutex );
  CHECK( mb::osal::tryLockRecursiveMutex( mutex, 100 ) );
  mb::osal::unlockRecursiveMutex( mutex );
  mb::osal::destroyRecursiveMutex( mutex );
}

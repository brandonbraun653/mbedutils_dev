/******************************************************************************
 *  File Name:
 *    test_smphr_stl.cpp
 *
 *  Description:
 *    Tests the STL based semaphore implementation for Mbedutils
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/interfaces/smphr_intf.hpp>

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( SemaphoreTests )
{
  void setup() override
  {
    mb::osal::initSmphrDriver();
  }

  void teardown() override
  {
    // Cleanup code if needed
  }
};
/* clang-format on */

TEST( SemaphoreTests, TestCreateAndDestroySemaphore )
{
  mb::osal::mb_smphr_t semaphore;
  CHECK( mb::osal::createSmphr( semaphore, 10, 5 ) );
  CHECK( semaphore != nullptr );
  mb::osal::destroySmphr( semaphore );
  CHECK( semaphore == nullptr );
}

TEST( SemaphoreTests, TestAllocateAndDeallocateSemaphore )
{
  mb::osal::mb_smphr_t semaphore;
  CHECK( mb::osal::allocateSemaphore( semaphore, 10, 5 ) );
  CHECK( semaphore != nullptr );
  mb::osal::deallocateSemaphore( semaphore );
  CHECK( semaphore == nullptr );
}

TEST( SemaphoreTests, TestAcquireAndReleaseSemaphore )
{
  mb::osal::mb_smphr_t semaphore;
  mb::osal::createSmphr( semaphore, 10, 1 );
  mb::osal::acquireSmphr( semaphore );
  CHECK_EQUAL( 0, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::releaseSmphr( semaphore );
  CHECK_EQUAL( 1, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::destroySmphr( semaphore );
}

TEST( SemaphoreTests, TestTryAcquireSemaphore )
{
  mb::osal::mb_smphr_t semaphore;
  mb::osal::createSmphr( semaphore, 10, 1 );
  CHECK( mb::osal::tryAcquireSmphr( semaphore ) );
  CHECK_EQUAL( 0, mb::osal::getSmphrAvailable( semaphore ) );
  CHECK_FALSE( mb::osal::tryAcquireSmphr( semaphore ) );
  mb::osal::releaseSmphr( semaphore );
  CHECK_EQUAL( 1, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::destroySmphr( semaphore );
}

TEST( SemaphoreTests, TestTryAcquireSemaphoreWithTimeout )
{
  mb::osal::mb_smphr_t semaphore;
  mb::osal::createSmphr( semaphore, 10, 1 );
  CHECK( mb::osal::tryAcquireSmphr( semaphore, 100 ) );
  CHECK_EQUAL( 0, mb::osal::getSmphrAvailable( semaphore ) );
  CHECK_FALSE( mb::osal::tryAcquireSmphr( semaphore, 100 ) );
  mb::osal::releaseSmphr( semaphore );
  CHECK_EQUAL( 1, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::destroySmphr( semaphore );
}

TEST( SemaphoreTests, TestGetSemaphoreAvailable )
{
  mb::osal::mb_smphr_t semaphore;
  mb::osal::createSmphr( semaphore, 10, 5 );
  CHECK_EQUAL( 5, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::acquireSmphr( semaphore );
  CHECK_EQUAL( 4, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::releaseSmphr( semaphore );
  CHECK_EQUAL( 5, mb::osal::getSmphrAvailable( semaphore ) );
  mb::osal::destroySmphr( semaphore );
}

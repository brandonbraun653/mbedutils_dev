/******************************************************************************
 *  File Name:
 *    test_key_value_db.cpp
 *
 *  Description:
 *    Test cases for key_value_db.cpp
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/drivers/database/key_value_db.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "assert_expect.hpp"


using namespace CppUMockGen;


/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

TEST_GROUP( key_value_db )
{
  void setup()
  {
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }
};

TEST( key_value_db, fail )
{
  CHECK_TRUE( false );
}

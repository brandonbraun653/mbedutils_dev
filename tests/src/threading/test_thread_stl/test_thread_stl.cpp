/******************************************************************************
 *  File Name:
 *    test_thread_stl.cpp
 *
 *  Description:
 *    Higher level tests for the thread module using the STL interfaces.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/

#include "CppUTest/CommandLineTestRunner.h"

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

/* clang-format off */
TEST_GROUP( thread_stl )
{
  void setup()
  {
  }

  void teardown()
  {
  }
};  /* clang-format on */


TEST( thread_stl, nominal_thread_creation )
{
  CHECK( true );
}

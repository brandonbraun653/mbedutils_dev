/******************************************************************************
 *  File Name:
 *    test_tasking.cpp
 *
 *  Description:
 *    Test the mb::thread::Task class
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

/* clang-format off */
TEST_GROUP( thread_tasking )
{
  void setup()
  {
  }

  void teardown()
  {
  }
};  /* clang-format on */


TEST( thread_tasking, nominal_thread_creation )
{
  CHECK( true );
}

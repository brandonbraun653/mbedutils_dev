/******************************************************************************
 *  File Name:
 *    test_message_passing.cpp
 *
 *  Description:
 *    Test the inter-thread message passing capabilities
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
TEST_GROUP( thread_messaging )
{
  void setup()
  {
  }

  void teardown()
  {
  }
};  /* clang-format on */


TEST( thread_messaging, hello )
{
  CHECK( true );
}

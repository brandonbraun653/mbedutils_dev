/******************************************************************************
 *  File Name:
 *    test_nor_flash.cpp
 *
 *  Description:
 *    Test cases for nor_flash.cpp
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/

#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

#include "CppUTest/TestHarness.h"

using namespace mb::memory::nor;

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( nor_flash )
{
  DeviceDriver norDriver;

  void setup()
  {
  }

  void teardown()
  {
  }
};


TEST( nor_flash, test_nor_flash )
{
  CHECK( true );
}

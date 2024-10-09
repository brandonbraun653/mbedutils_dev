/******************************************************************************
 *  File Name:
 *    test_db_kv_mgr.cpp
 *
 *  Description:
 *    Test cases for the key-value database module.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>
#include <cstddef>
#include <array>
#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <mbedutils/database.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/CommandLineTestRunner.h"

#include "assert_expect.hpp"
#include "atexit_expect.hpp"
#include "atexit_harness.hpp"
#include "mutex_intf_expect.hpp"
#include "nor_flash_expect.hpp"
#include "nor_flash_file.hpp"
#include "test_kv_db_harness.hpp"


using namespace mb::db;
using namespace CppUMockGen;

/*-----------------------------------------------------------------------------
Static Data
-----------------------------------------------------------------------------*/

static KVRAMData                     s_kv_cache_backing;
static fake::memory::nor::FileFlash *s_flash_0_driver;
static fake::memory::nor::FileFlash *s_flash_1_driver;

extern "C"
{
  const fal_flash_dev fdb_nor_flash0 = {
    .name     = "nor_flash_0",
    .addr     = 0x00000000,
    .len      = 8 * 1024 * 1024,
    .blk_size = 4096,
    .ops      = {
             .init = []( void ) -> int { return 0; },
        .read                        = []( long offset, uint8_t *buf, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_0_driver->read( offset, buf, size ) ) ? 0 : -1;
        },
        .write                       = []( long offset, const uint8_t *buf, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_0_driver->write( offset, buf, size ) ) ? 0 : -1;
        },
        .erase                       = []( long offset, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_0_driver->erase( offset, size ) ) ? 0 : -1;
        },
    },
    .write_gran                      = 1
  };

  const fal_flash_dev fdb_nor_flash1 = {
    .name     = "nor_flash_1",
    .addr     = 0x00000000,
    .len      = 16 * 1024 * 1024,
    .blk_size = 4096,
    .ops      = {
             .init = []( void ) -> int { return 0; },
        .read                        = []( long offset, uint8_t *buf, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_1_driver->read( offset, buf, size ) ) ? 0 : -1;
        },
        .write                       = []( long offset, const uint8_t *buf, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_1_driver->write( offset, buf, size ) ) ? 0 : -1;
        },
        .erase                       = []( long offset, size_t size ) -> int {
          return ( mb::memory::Status::ERR_OK == s_flash_1_driver->erase( offset, size ) ) ? 0 : -1;
        },
    },
    .write_gran                      = 1
  };
}

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

/*-----------------------------------------------------------------------------
KVNode Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_mt )
{
  void setup()
  {

  }

  void teardown()
  {

  }
};


TEST( db_mt, multi_writer_single_reader )
{
 CHECK( false );
}
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
#include <cstdint>
#include <cstddef>
#include <array>
#include <etl/array.h>
#include <etl/vector.h>
#include <mbedutils/drivers/database/key_value_db.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "assert_expect.hpp"
#include "nor_flash_expect.hpp"


using namespace CppUMockGen;

/*-----------------------------------------------------------------------------
Static Data
-----------------------------------------------------------------------------*/

static mb::memory::nor::DeviceDriver* s_flash_0_driver;
static mb::memory::nor::DeviceDriver* s_flash_1_driver;

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


struct KVRAMData
{
  // Put generated NanoPB struct declarations here
};

static constexpr auto _unsorted_kv_dsc = std::to_array<KVNode_t>( {
  KVNode_t{ /* Fill this out */ },
});


// TODO: Sort the KVNode_t array at compile time
auto compare_kv_nodes( const KVNode_t &lhs, const KVNode_t &rhs ) -> bool
{
  return lhs.key < rhs.key;
}


// static const etl::vector<KVNode_t, _unsorted_kv_dsc.size()> _sorted_kv_dsc = _unsorted_kv_dsc;

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

using namespace mb::db;

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

TEST_GROUP( key_value_db )
{
  PersistentKVDB test_kvdb;

  void setup()
  {
    expect::mb$::memory$::nor$::DeviceDriver$::DeviceDriver$ctor( 2 );
    s_flash_0_driver = new mb::memory::nor::DeviceDriver();
    s_flash_1_driver = new mb::memory::nor::DeviceDriver();

    mock().clear();
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    mock().checkExpectations();

    delete s_flash_0_driver;
    delete s_flash_1_driver;
    mock().clear();
  }
};

TEST( key_value_db, construction_of_invalid_database_fails )
{
  auto config = PersistentKVDB::Config();

  config.dev_name             = "nor_flash_32";
  config.partition_name       = "partition_0";
  config.default_kv_table.kvs = nullptr;
  config.default_kv_table.num = 0;

  CHECK_FALSE( test_kvdb.init( config ) );
}

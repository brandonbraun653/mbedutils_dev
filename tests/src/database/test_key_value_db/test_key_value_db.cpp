/******************************************************************************
 *  File Name:
 *    test_db_kv_mgr.cpp
 *
 *  Description:
 *    Test cases for db_kv_mgr.cpp
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
#include <mbedutils/database.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "assert_expect.hpp"
#include "nor_flash_expect.hpp"
#include "test_kv_db.pb.h"


using namespace mb::db;
using namespace CppUMockGen;

/*-----------------------------------------------------------------------------
Enumerations
-----------------------------------------------------------------------------*/

enum KVAppKeys : HashKey
{
  KEY_SIMPLE_POD_DATA,
  KEY_KINDA_COMPLEX_POD_DATA,
  KEY_ETL_STRING_DATA,

  KEY_ENUM_COUNT
};


/*-----------------------------------------------------------------------------
Structures
-----------------------------------------------------------------------------*/

struct KVRAMData
{
  SimplePODData       simple_pod_data;        /**< KEY_SIMPLE_POD_DATA */
  KindaComplexPODData kinda_complex_pod_data; /**< KEY_KINDA_COMPLEX_POD_DATA */
  etl::string<32>     etl_string_data;        /**< KEY_ETL_STRING_DATA */
};

/*-----------------------------------------------------------------------------
Static Data
-----------------------------------------------------------------------------*/

static KVRAMData                      s_kv_cache_backing;
static RamKVDBStorage<20, 512>        s_kv_ram_storage;
static NvmKVDBStorage<20, 512>        s_kv_nvm_storage;
static mb::memory::nor::DeviceDriver *s_flash_0_driver;
static mb::memory::nor::DeviceDriver *s_flash_1_driver;

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
RAM Key-Value Database Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_kv_ram )
{
  RamKVDB test_kvdb;

  void setup()
  {
    mock().clear();
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }
};

TEST( db_kv_ram, configure_nominally )
{
  auto config             = RamKVDB::Config();
  config.node_storage     = &s_kv_ram_storage.nodes;
  config.transcode_buffer = { s_kv_ram_storage.transcode_buffer };

  CHECK( DB_ERR_NONE == test_kvdb.configure( config ) );
}


/*-----------------------------------------------------------------------------
NVM Key-Value Database Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_kv_nvm )
{
  NvmKVDB test_kvdb;

  void setup()
  {
    expect::mb$::memory$::nor$::DeviceDriver$::DeviceDriver$ctor( 2 );
    s_flash_0_driver = new mb::memory::nor::DeviceDriver();
    s_flash_1_driver = new mb::memory::nor::DeviceDriver();

    mock().clear();
    mock().ignoreOtherCalls();

    RamKVDB::Config ram_config;
    ram_config.node_storage     = &s_kv_nvm_storage.nodes;
    ram_config.transcode_buffer = etl::span<uint8_t>( s_kv_nvm_storage.transcode_buffer );

    s_kv_nvm_storage.ramdb.configure( ram_config );
    s_kv_nvm_storage.ramdb.insert( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                     .datacache = &s_kv_cache_backing.simple_pod_data,
                                     .pbFields  = SimplePODData_fields,
                                     .pbSize    = SimplePODData_size,
                                     .flags     = KV_FLAG_DEFAULT_PERSISTENT } );

    s_kv_nvm_storage.ramdb.insert( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                     .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                     .pbFields  = KindaComplexPODData_fields,
                                     .pbSize    = KindaComplexPODData_size,
                                     .flags     = KV_FLAG_DEFAULT_PERSISTENT } );

    s_kv_nvm_storage.ramdb.insert( {} );
  }

  void teardown()
  {
    mock().checkExpectations();

    delete s_flash_0_driver;
    delete s_flash_1_driver;
    mock().clear();
  }
};

TEST( db_kv_nvm, construction_of_invalid_database_fails )
{
  auto config = NvmKVDB::Config();

  config.dev_name       = "nor_flash_32";
  config.partition_name = "partition_0";

  CHECK_FALSE( test_kvdb.configure( config ) );
}

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

  KEY_ENUM_COUNT
};


/*-----------------------------------------------------------------------------
Structures
-----------------------------------------------------------------------------*/

struct KVRAMData
{
  SimplePODData       simple_pod_data;        /**< KEY_SIMPLE_POD_DATA */
  KindaComplexPODData kinda_complex_pod_data; /**< KEY_KINDA_COMPLEX_POD_DATA */
};

/*-----------------------------------------------------------------------------
Static Data
-----------------------------------------------------------------------------*/

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


static KVRAMData                     s_kv_ram_data;
static PersistenKVDBStorage<20, 512> s_kv_storage;

static constexpr auto _unsorted_kv_dsc = std::to_array<KVParamNode>( {
    KVParamNode{ .hashKey      = KEY_SIMPLE_POD_DATA,
                 .updator      = {},
                 .validator    = {},
                 .sanitizer    = {},
                 .serializer   = {},
                 .deserializer = {},
                 .pbRAMCopy    = &s_kv_ram_data.simple_pod_data,
                 .pbDescriptor = SimplePODData_fields,
                 .pbSize       = SimplePODData_size,
                 .flags        = KV_FLAG_DEFAULT_PERSISTENT },

    KVParamNode{ .hashKey      = KEY_KINDA_COMPLEX_POD_DATA,
                 .updator      = {},
                 .validator    = {},
                 .sanitizer    = {},
                 .serializer   = {},
                 .deserializer = {},
                 .pbRAMCopy    = &s_kv_ram_data.kinda_complex_pod_data,
                 .pbDescriptor = KindaComplexPODData_fields,
                 .pbSize       = KindaComplexPODData_size,
                 .flags        = KV_FLAG_DEFAULT_PERSISTENT },

    KVParamNode{ .hashKey      = std::numeric_limits<HashKey>::max(),
                 .updator      = {},
                 .validator    = {},
                 .sanitizer    = {},
                 .serializer   = {},
                 .deserializer = {},
                 .pbRAMCopy    = nullptr,
                 .pbDescriptor = nullptr,
                 .pbSize       = 0,
                 .flags        = 0 },

} );

using ParameterList = std::array<KVParamNode, _unsorted_kv_dsc.size()>;
static constexpr ParameterList ParamSorter( const ParameterList &list )
  {
    auto result = list;
    std::sort( result.begin(), result.end(), []( const KVParamNode &a, const KVParamNode &b ) -> bool { return a.hashKey < b.hashKey; } );
    return result;
  }

static const ParameterList ParamInfo = ParamSorter( _unsorted_kv_dsc );


/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

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

  config.dev_name       = "nor_flash_32";
  config.partition_name = "partition_0";

  CHECK_FALSE( test_kvdb.configure( config ) );
}

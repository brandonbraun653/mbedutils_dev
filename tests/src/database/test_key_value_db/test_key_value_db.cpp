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
#include "nor_flash_expect.hpp"
#include "nor_flash_file.hpp"
#include "test_harness.hpp"
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
  KEY_GYRO_DATA,

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
  GyroSensorData      gyro_data;              /**< KEY_GYRO_DATA */
};

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

TEST_GROUP( kv_node )
{
  KVNode        test_node;
  KVNodeHarness harness;

  void setup()
  {
    /*-------------------------------------------------------------------------
    Reset test data
    -------------------------------------------------------------------------*/
    harness.reset();

    mock().clear();
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }
};

TEST( kv_node, nominal_construction )
{
  CHECK( MAX_HASH_KEY == test_node.hashKey );
  CHECK( nullptr == test_node.datacache );
  CHECK( nullptr == test_node.pbFields );
  CHECK( 0 == test_node.dataSize );
  CHECK( 0 == test_node.flags );
}

TEST( kv_node, nominal_sanitize )
{
  test_node.sanitizer = harness.sanitize_delegate;
  sanitize( test_node );
  CHECK_EQUAL( 1, harness.sanitize_callback_calls );
}

TEST( kv_node, nominal_validity )
{
  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.hashKey   = KEY_SIMPLE_POD_DATA;
  test_node.pbFields  = SimplePODData_fields;

  test_node.validator = harness.validate_delegate;
  CHECK( data_is_valid( test_node ) );
  CHECK_EQUAL( 1, harness.validate_callback_calls );
}

TEST( kv_node, validity_with_no_validator )
{
  CHECK( false == data_is_valid( test_node ) );
}

TEST( kv_node, nominal_write )
{
  test_node.writer = harness.write_delegate;
  CHECK( node_write( test_node, nullptr, 0 ) );
  CHECK_EQUAL( 1, harness.write_callback_calls );
}

TEST( kv_node, nominal_read )
{
  test_node.reader = harness.read_delegate;
  CHECK_EQUAL( 0, node_read( test_node, nullptr, 0 ) );
  CHECK_EQUAL( 1, harness.read_callback_calls );
}

TEST( kv_node, read_with_no_reader )
{
  CHECK( false == node_read( test_node, nullptr, 0 ) );
}

TEST( kv_node, write_with_no_writer )
{
  CHECK( false == node_write( test_node, nullptr, 0 ) );
}

TEST( kv_node, nominal_transcode )
{
  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = SimplePODData_fields;

  s_kv_cache_backing.simple_pod_data.value = 0x55;

  auto size = node_serialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() );
  CHECK( node_deserialize( test_node, harness.transcode_buffer.data(), size ) );

  CHECK( 0x55 == s_kv_cache_backing.simple_pod_data.value );
}

TEST( kv_node, serialize_bad_arguments )
{
  CHECK( -1 == node_serialize( test_node, nullptr, 55 ) );
  CHECK( -1 == node_serialize( test_node, harness.transcode_buffer.data(), 0 ) );

  test_node.pbFields = nullptr;
  CHECK( -1 == node_serialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() ) );
}

TEST( kv_node, serialize_bad_configuration )
{
  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = StringData_fields;    // Wrong fields for the data type

  test_node.datacache = nullptr;
  CHECK( -1 == node_serialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() ) );
}

TEST( kv_node, deserialize_bad_arguments )
{
  CHECK( false == node_deserialize( test_node, nullptr, 55 ) );

  test_node.datacache = nullptr;
  test_node.pbFields  = SimplePODData_fields;
  CHECK( false == node_deserialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() ) );

  test_node.pbFields = nullptr;
  CHECK( false == node_deserialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() ) );
}

TEST( kv_node, deserialize_bad_configuration )
{
  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = StringData_fields;    // Wrong fields for the data type

  CHECK( false == node_deserialize( test_node, harness.transcode_buffer.data(), harness.transcode_buffer.size() ) );
}

TEST( kv_node, kv_writer_memcpy )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = SimplePODData_fields;

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  CHECK( kv_writer_memcpy( test_node, &data_to_copy, SimplePODData_size ) );
  CHECK( 0x55 == s_kv_cache_backing.simple_pod_data.value );
}

TEST( kv_node, kv_writer_memcpy_via_delegate )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = SimplePODData_fields;
  test_node.writer    = WriteFunc::create<kv_writer_memcpy>();

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  CHECK( node_write( test_node, &data_to_copy, SimplePODData_size ) );
  CHECK( 0x55 == s_kv_cache_backing.simple_pod_data.value );
}

TEST( kv_node, kv_writer_char_to_etl_string )
{
  /* Reset test data */
  s_kv_cache_backing.etl_string_data.clear();

  test_node.datacache = &s_kv_cache_backing.etl_string_data;
  test_node.dataSize  = StringData_size;
  test_node.pbFields  = StringData_fields;
  test_node.writer    = WriteFunc::create<kv_writer_char_to_etl_string>();

  etl::string<32> data_to_copy = "Hello, World!";

  CHECK( node_write( test_node, data_to_copy.c_str(), data_to_copy.size() ) );
  CHECK( 0 == data_to_copy.compare( s_kv_cache_backing.etl_string_data ) );
}

TEST( kv_node, kv_reader_memcpy )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = SimplePODData_fields;
  test_node.writer    = WriteFunc::create<kv_writer_memcpy>();
  test_node.reader    = ReadFunc::create<kv_reader_memcpy>();

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  /* Write as data */
  CHECK( node_write( test_node, &data_to_copy, SimplePODData_size ) );

  /* Read the data out */
  SimplePODData data_to_read;
  CHECK( node_read( test_node, &data_to_read, SimplePODData_size ) );
  CHECK( 0x55 == data_to_read.value );
}

TEST( kv_node, kv_reader_memcpy_invalid_inputs )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = nullptr;
  test_node.dataSize  = 0;
  test_node.pbFields  = SimplePODData_fields;

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  CHECK( -1 == kv_reader_memcpy( test_node, nullptr, SimplePODData_size ) );
  CHECK( -1 == kv_reader_memcpy( test_node, &data_to_copy, 0 ) );
  CHECK( -1 == kv_reader_memcpy( test_node, &data_to_copy, SimplePODData_size ) );
}

TEST( kv_node, kv_reader_memcpy_with_validity_status_is_invalid )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = &s_kv_cache_backing.simple_pod_data;
  test_node.dataSize  = SimplePODData_size;
  test_node.pbFields  = SimplePODData_fields;
  test_node.writer    = WriteFunc::create<kv_writer_memcpy>();
  test_node.reader    = ReadFunc::create<kv_reader_memcpy>();

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  CHECK( node_write( test_node, &data_to_copy, SimplePODData_size ) );

  SimplePODData data_to_read;
  CHECK( node_read( test_node, &data_to_read, SimplePODData_size ) );
  CHECK( 0x55 == data_to_read.value );
}

TEST( kv_node, kv_reader_etl_string_to_char )
{
  /* Reset test data */
  s_kv_cache_backing.etl_string_data.clear();

  test_node.datacache = &s_kv_cache_backing.etl_string_data;
  test_node.dataSize  = StringData_size;
  test_node.pbFields  = StringData_fields;
  test_node.writer    = WriteFunc::create<kv_writer_char_to_etl_string>();
  test_node.reader    = ReadFunc::create<kv_reader_etl_string_to_char>();

  etl::string<32> data_to_copy = "Hello, World!";

  CHECK( node_write( test_node, data_to_copy.c_str(), data_to_copy.size() ) );

  char data_to_read[ 32 ];
  memset( data_to_read, 0, sizeof( data_to_read ) );

  CHECK( node_read( test_node, data_to_read, sizeof( data_to_read ) ) );
  CHECK( 0 == data_to_copy.compare( data_to_read ) );
}

TEST( kv_node, kv_reader_etl_string_to_char_invalid_inputs )
{
  /* Reset test data */
  s_kv_cache_backing.etl_string_data.clear();

  test_node.datacache = nullptr;
  test_node.dataSize  = 0;
  test_node.pbFields  = StringData_fields;

  char data_to_read[ 32 ];
  memset( data_to_read, 0, sizeof( data_to_read ) );

  CHECK( -1 == kv_reader_etl_string_to_char( test_node, data_to_read, sizeof( data_to_read ) ) );
  CHECK( -1 == kv_reader_etl_string_to_char( test_node, nullptr, sizeof( data_to_read ) ) );
  CHECK( -1 == kv_reader_etl_string_to_char( test_node, data_to_read, 0 ) );
}

TEST( kv_node, kv_writer_memcpy_invalid_inputs )
{
  /* Reset test data */
  s_kv_cache_backing.simple_pod_data.value = 0;

  test_node.datacache = nullptr;
  test_node.dataSize  = 0;
  test_node.pbFields  = SimplePODData_fields;

  SimplePODData data_to_copy;
  data_to_copy.value = 0x55;

  CHECK( false == kv_writer_memcpy( test_node, nullptr, SimplePODData_size ) );
  CHECK( false == kv_writer_memcpy( test_node, &data_to_copy, 0 ) );
  CHECK( false == kv_writer_memcpy( test_node, &data_to_copy, SimplePODData_size ) );
}

TEST( kv_node, kv_writer_char_to_etl_string_invalid_inputs )
{
  /* Reset test data */
  s_kv_cache_backing.etl_string_data.clear();

  test_node.datacache = nullptr;
  test_node.dataSize  = 0;
  test_node.pbFields  = StringData_fields;

  etl::string<32> data_to_copy = "Hello, World!";

  CHECK( false == kv_writer_char_to_etl_string( test_node, data_to_copy.c_str(), data_to_copy.size() ) );
  CHECK( false == kv_writer_char_to_etl_string( test_node, nullptr, data_to_copy.size() ) );
  CHECK( false == kv_writer_char_to_etl_string( test_node, data_to_copy.c_str(), 0 ) );
}

/*-----------------------------------------------------------------------------
RAM Key-Value Database Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_kv_ram )
{
  RamKVDB                   test_kvdb;
  RamKVDB::Config           test_config;
  RamKVDB::Storage<20, 512> test_storage;

  void setup()
  {
    /*-------------------------------------------------------------------------
    Configure a basic RAM database
    -------------------------------------------------------------------------*/
    test_storage.nodes.clear();
    test_storage.nodes.push_back( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                    .writer    = KVWriter_Memcpy,
                                    .reader    = KVReader_Memcpy,
                                    .datacache = &s_kv_cache_backing.simple_pod_data,
                                    .pbFields  = SimplePODData_fields,
                                    .dataSize  = SimplePODData_size,
                                    .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.nodes.push_back( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                    .writer    = KVWriter_Memcpy,
                                    .reader    = KVReader_Memcpy,
                                    .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                    .pbFields  = KindaComplexPODData_fields,
                                    .dataSize  = KindaComplexPODData_size,
                                    .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.nodes.push_back( { .hashKey   = KEY_ETL_STRING_DATA,
                                    .writer    = KVWriter_EtlString,
                                    .reader    = KVReader_EtlString,
                                    .datacache = &s_kv_cache_backing.etl_string_data,
                                    .pbFields  = StringData_fields,
                                    .dataSize  = StringData_size,
                                    .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_config.node_storage     = &test_storage.nodes;
    test_config.transcode_buffer = test_storage.transcode_buffer;

    /*-------------------------------------------------------------------------
    Initialize the database
    -------------------------------------------------------------------------*/
    CHECK( DB_ERR_NONE == test_kvdb.configure( test_config ) );

    mock().clear();
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    /*-------------------------------------------------------------------------
    Call empty methods to ensure coverage
    -------------------------------------------------------------------------*/
    test_kvdb.sync();
    test_kvdb.flush();
    test_kvdb.deinit();

    /*-------------------------------------------------------------------------
    Verify all expectations
    -------------------------------------------------------------------------*/
    mock().checkExpectations();
    mock().clear();
  }
};

TEST( db_kv_ram, configure_nominally )
{
  /*---------------------------------------------------------------------------
  Setup the configuration
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.node_storage     = &test_storage.nodes;
  config.transcode_buffer = { test_storage.transcode_buffer };

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  CHECK( DB_ERR_NONE == kvdb.configure( config ) );
}

TEST( db_kv_ram, configure_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Missing node storage
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.node_storage     = nullptr;
  config.transcode_buffer = { test_storage.transcode_buffer };

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing transcode buffer
  ---------------------------------------------------------------------------*/
  config.node_storage     = &test_storage.nodes;
  config.transcode_buffer = {};

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Bad KVNode in the storage
  ---------------------------------------------------------------------------*/
  config.node_storage     = &test_storage.nodes;
  config.transcode_buffer = { test_storage.transcode_buffer };

  test_storage.nodes.push_back( {} );
  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );
}

TEST( db_kv_ram, configure_transcode_buffer_too_small )
{
  /*---------------------------------------------------------------------------
  Setup the configuration
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.node_storage     = &test_storage.nodes;
  config.transcode_buffer = { test_storage.transcode_buffer.data(), 1 };

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  CHECK( DB_ERR_TRANSCODE_BUFFER_TOO_SMALL == kvdb.configure( config ) );
}

TEST( db_kv_ram, init )
{
  // KVDB was configured with some default data in the setup() method.
  CHECK( test_kvdb.init() );
}

TEST( db_kv_ram, find_insert_exist_remove )
{
  /*---------------------------------------------------------------------------
  First, check to make sure the item doesn't already exist
  ---------------------------------------------------------------------------*/
  CHECK( nullptr == test_kvdb.find( KEY_GYRO_DATA ) );

  /*---------------------------------------------------------------------------
  Insert a new item
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_GYRO_DATA;
  new_node.writer    = WriteFunc::create<kv_writer_memcpy>();
  new_node.reader    = ReadFunc::create<kv_reader_memcpy>();
  new_node.datacache = &s_kv_cache_backing.gyro_data;
  new_node.pbFields  = GyroSensorData_fields;
  new_node.dataSize  = GyroSensorData_size;
  new_node.flags     = KV_FLAG_DEFAULT_VOLATILE;

  CHECK( test_kvdb.insert( new_node ) );

  /*---------------------------------------------------------------------------
  Verify the item now exists
  ---------------------------------------------------------------------------*/
  CHECK( nullptr != test_kvdb.find( KEY_GYRO_DATA ) );
  CHECK( test_kvdb.exists( KEY_GYRO_DATA ) );

  /*---------------------------------------------------------------------------
  Verify attempts to reinsert the same key fail
  ---------------------------------------------------------------------------*/
  CHECK( false == test_kvdb.insert( new_node ) );

  /*---------------------------------------------------------------------------
  Remove the item
  ---------------------------------------------------------------------------*/
  test_kvdb.remove( KEY_GYRO_DATA );

  /*---------------------------------------------------------------------------
  Verify the item no longer exists
  ---------------------------------------------------------------------------*/
  CHECK( nullptr == test_kvdb.find( KEY_GYRO_DATA ) );
  CHECK( false == test_kvdb.exists( KEY_GYRO_DATA ) );
}

TEST( db_kv_ram, read_write_a_nonexistant_node )
{
  /*---------------------------------------------------------------------------
  Read a non-existant node
  ---------------------------------------------------------------------------*/
  GyroSensorData data;

  CHECK( -1 == test_kvdb.read( KEY_ENUM_COUNT, &data, sizeof( data ) ) );
  CHECK( -1 == test_kvdb.write( KEY_ENUM_COUNT, &data, sizeof( data ) ) );
}

TEST( db_kv_ram, write_read_data )
{
  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  SimplePODData data_to_write;
  data_to_write.value = 0x55;

  CHECK( sizeof( data_to_write ) == test_kvdb.write( KEY_SIMPLE_POD_DATA, &data_to_write, sizeof( data_to_write ) ) );

  /*---------------------------------------------------------------------------
  Read the data back out
  ---------------------------------------------------------------------------*/
  SimplePODData data_to_read;
  data_to_read.value = 0;

  CHECK( sizeof( data_to_read ) == test_kvdb.read( KEY_SIMPLE_POD_DATA, &data_to_read, sizeof( data_to_read ) ) );
  CHECK( 0x55 == data_to_read.value );
}

TEST( db_kv_ram, encode_decode_a_nonexistant_node )
{
  uint8_t encoded_data[ 64 ];
  CHECK( -1 == test_kvdb.encode( KEY_ENUM_COUNT, encoded_data, sizeof( encoded_data ) ) );
  CHECK( -1 == test_kvdb.decode( KEY_ENUM_COUNT, encoded_data, sizeof( encoded_data ) ) );
}

TEST( db_kv_ram, encode_decode_data )
{
  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  SimplePODData data_to_write;
  data_to_write.value = 0x55;

  CHECK( sizeof( data_to_write ) == test_kvdb.write( KEY_SIMPLE_POD_DATA, &data_to_write, sizeof( data_to_write ) ) );

  /*---------------------------------------------------------------------------
  Encode the data
  ---------------------------------------------------------------------------*/
  uint8_t encoded_data[ 64 ];
  int     encoded_size = test_kvdb.encode( KEY_SIMPLE_POD_DATA, encoded_data, sizeof( encoded_data ) );

  CHECK( encoded_size > 0 );

  /*---------------------------------------------------------------------------
  Write new data to ensure the decode process is working
  ---------------------------------------------------------------------------*/
  data_to_write.value = 0xAA;
  CHECK( sizeof( data_to_write ) == test_kvdb.write( KEY_SIMPLE_POD_DATA, &data_to_write, sizeof( data_to_write ) ) );

  /*---------------------------------------------------------------------------
  Decode the data
  ---------------------------------------------------------------------------*/
  SimplePODData data_to_read;
  int           decoded_size = test_kvdb.decode( KEY_SIMPLE_POD_DATA, encoded_data, encoded_size );

  CHECK( decoded_size > 0 );

  /*-----------------------------------------------------------------------------
  Read the data back out
  -----------------------------------------------------------------------------*/
  CHECK( test_kvdb.read( KEY_SIMPLE_POD_DATA, &data_to_read, sizeof( data_to_read ) ) );
  CHECK( 0x55 == data_to_read.value );
}

/*-----------------------------------------------------------------------------
NVM Key-Value Database Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_kv_nvm )
{
  NvmKVDB                                 test_kvdb;
  NvmKVDB::Config                         test_config;
  NvmKVDB::Storage<20, 512>               test_storage;
  harness::system::atexit::CallbackCopier atexit_callback_copier;

  static constexpr char *const test_dflt_dev_name   = "nor_flash_0";
  static constexpr char *const test_dflt_partition  = "kv_db";

  void setup()
  {
    s_flash_0_driver = new fake::memory::nor::FileFlash();
    s_flash_1_driver = new fake::memory::nor::FileFlash();

    mock().clear();
    mock().ignoreOtherCalls();
    mock().installCopier( "mb::system::atexit::Callback", atexit_callback_copier );

    /*-------------------------------------------------------------------------
    Configure the flash devices
    -------------------------------------------------------------------------*/
    mb::memory::nor::DeviceConfig flash_0_cfg;
    flash_0_cfg.dev_attr.block_size = fdb_nor_flash0.blk_size;
    flash_0_cfg.dev_attr.size       = fdb_nor_flash0.len;

    s_flash_0_driver->open( "flash_0_test.bin", flash_0_cfg );

    mb::memory::nor::DeviceConfig flash_1_cfg;
    flash_1_cfg.dev_attr.block_size = fdb_nor_flash1.blk_size;
    flash_1_cfg.dev_attr.size       = fdb_nor_flash1.len;

    s_flash_1_driver->open( "flash_1_test.bin", flash_1_cfg );

    /*-------------------------------------------------------------------------
    Configure a basic RAM database
    -------------------------------------------------------------------------*/
    test_storage.kv_nodes.clear();
    test_storage.kv_nodes.push_back( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.simple_pod_data,
                                       .pbFields  = SimplePODData_fields,
                                       .dataSize  = SimplePODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.kv_nodes.push_back( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                       .pbFields  = KindaComplexPODData_fields,
                                       .dataSize  = KindaComplexPODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.kv_nodes.push_back( { .hashKey   = KEY_ETL_STRING_DATA,
                                       .writer    = KVWriter_EtlString,
                                       .reader    = KVReader_EtlString,
                                       .datacache = &s_kv_cache_backing.etl_string_data,
                                       .pbFields  = StringData_fields,
                                       .dataSize  = StringData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    /*-------------------------------------------------------------------------
    Configure the RAM database
    -------------------------------------------------------------------------*/
    RamKVDB::Config ram_config;
    ram_config.node_storage     = &test_storage.kv_nodes;
    ram_config.transcode_buffer = test_storage.transcode_buffer;

    CHECK( DB_ERR_NONE == test_storage.kv_ram_db.configure( ram_config ) );

    /*-------------------------------------------------------------------------
    Configure the NVM database
    -------------------------------------------------------------------------*/
    test_config.dev_name  = test_dflt_dev_name;
    test_config.part_name = test_dflt_partition;
    test_config.ram_kvdb  = &test_storage.kv_ram_db;

    CHECK( DB_ERR_NONE == test_kvdb.configure( test_config ) );
  }

  void teardown()
  {
    mock().checkExpectations();

    s_flash_0_driver->close();
    s_flash_1_driver->close();

    delete s_flash_0_driver;
    delete s_flash_1_driver;
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST( db_kv_nvm, construction_of_invalid_database_fails )
{
  NvmKVDB::Config config;

  /*---------------------------------------------------------------------------
  Missing device name
  ---------------------------------------------------------------------------*/
  config.dev_name  = "";
  config.part_name = "hello";
  config.ram_kvdb  = test_config.ram_kvdb;

  CHECK( DB_ERR_BAD_ARG == test_kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing partition name
  ---------------------------------------------------------------------------*/
  config.dev_name  = "hello";
  config.part_name = "";
  config.ram_kvdb  = test_config.ram_kvdb;

  CHECK( DB_ERR_BAD_ARG == test_kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing RAM KVDB
  ---------------------------------------------------------------------------*/
  config.dev_name  = "hello";
  config.part_name = "hello";
  config.ram_kvdb  = nullptr;

  CHECK( DB_ERR_BAD_ARG == test_kvdb.configure( config ) );
}

/**
 * @brief Validates the NVM database can be initialized with test setup defaults
 */
TEST( db_kv_nvm, init_with_test_defaults )
{
  expect::mb$::system$::atexit$::registerCallback( harness::system::atexit::stub_atexit_do_nothing, IgnoreParameter(),
                                                   true );

  CHECK( test_kvdb.init() );
}


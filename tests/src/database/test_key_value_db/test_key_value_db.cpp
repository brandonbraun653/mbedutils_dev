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
#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <mbedutils/database.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include "CppUTestExt/MockSupportPlugin.h"
#include <CppUTest/CommandLineTestRunner.h>

#include "assert_expect.hpp"
#include "atexit_expect.hpp"
#include "atexit_harness.hpp"
#include "mbedutils/drivers/database/db_kv_util.hpp"
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
  RamKVDB          test_kvdb;
  RamKVDB::Config  test_config;
  Storage<20, 512> test_storage;

  void setup()
  {
    /*-------------------------------------------------------------------------
    Configure a basic RAM database
    -------------------------------------------------------------------------*/
    test_storage.node_dsc.clear();
    test_storage.node_dsc.push_back( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.simple_pod_data,
                                       .pbFields  = SimplePODData_fields,
                                       .dataSize  = SimplePODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.node_dsc.push_back( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                       .pbFields  = KindaComplexPODData_fields,
                                       .dataSize  = KindaComplexPODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.node_dsc.push_back( { .hashKey   = KEY_ETL_STRING_DATA,
                                       .writer    = KVWriter_EtlString,
                                       .reader    = KVReader_EtlString,
                                       .datacache = &s_kv_cache_backing.etl_string_data,
                                       .pbFields  = StringData_fields,
                                       .dataSize  = StringData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_config.ext_node_dsc         = &test_storage.node_dsc;
    test_config.ext_transcode_buffer = test_storage.transcode_buffer;

    /*-------------------------------------------------------------------------
    Initialize the database
    -------------------------------------------------------------------------*/
    expect::mb$::osal$::createRecursiveMutex( IgnoreParameter(), true );
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
  config.ext_node_dsc         = &test_storage.node_dsc;
  config.ext_transcode_buffer = { test_storage.transcode_buffer };

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createRecursiveMutex( IgnoreParameter(), true );
  CHECK( DB_ERR_NONE == kvdb.configure( config ) );
}

TEST( db_kv_ram, configure_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Missing node storage
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.ext_node_dsc         = nullptr;
  config.ext_transcode_buffer = { test_storage.transcode_buffer };

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing transcode buffer
  ---------------------------------------------------------------------------*/
  config.ext_node_dsc         = &test_storage.node_dsc;
  config.ext_transcode_buffer = {};

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Bad KVNode in the storage
  ---------------------------------------------------------------------------*/
  config.ext_node_dsc         = &test_storage.node_dsc;
  config.ext_transcode_buffer = { test_storage.transcode_buffer };

  test_storage.node_dsc.push_back( {} );
  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );
}

TEST( db_kv_ram, configure_transcode_buffer_too_small )
{
  /*---------------------------------------------------------------------------
  Setup the configuration
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.ext_node_dsc         = &test_storage.node_dsc;
  config.ext_transcode_buffer = { test_storage.transcode_buffer.data(), 1 };

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  CHECK( DB_ERR_TRANSCODE_BUFFER_TOO_SMALL == kvdb.configure( config ) );
}

TEST( db_kv_ram, init )
{
  /*---------------------------------------------------------------------------
  Calling configure twice should fail
  ---------------------------------------------------------------------------*/
  CHECK( DB_ERR_NOT_AVAILABLE == test_kvdb.configure( test_config ) );
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

TEST_GROUP( db_kv_nvm_setup )
{
  NvmKVDB test_kvdb;

  void setup()
  {
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }
};

TEST( db_kv_nvm_setup, check_function_calls_have_no_side_effects_before_init )
{
  /*---------------------------------------------------------------------------
  If any calls occur other than a return, we'll learn about it from mock.
  ---------------------------------------------------------------------------*/
  test_kvdb.deinit();
  test_kvdb.remove( 0 );
  CHECK( false == test_kvdb.exists( 0 ) );
  CHECK( -1 == test_kvdb.read( 0, nullptr, 0 ) );
  CHECK( -1 == test_kvdb.write( 0, nullptr, 0 ) );
}


TEST_GROUP( db_kv_nvm )
{
  NvmKVDB                                 test_kvdb;
  NvmKVDB::Config                         test_config;
  Storage<20, 512>                        test_storage;
  harness::system::atexit::CallbackCopier atexit_callback_copier;

  const std::string test_dflt_dev_name  = "nor_flash_0";
  const std::string test_dflt_partition = "kv_db";

  void setup()
  {
    /*-------------------------------------------------------------------------
    Initialize the virtual backing memory
    -------------------------------------------------------------------------*/
    s_kv_cache_backing.clear();
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

    std::remove( "flash_0_test.bin" );
    s_flash_0_driver->open( "flash_0_test.bin", flash_0_cfg );

    mb::memory::nor::DeviceConfig flash_1_cfg;
    flash_1_cfg.dev_attr.block_size = fdb_nor_flash1.blk_size;
    flash_1_cfg.dev_attr.size       = fdb_nor_flash1.len;

    std::remove( "flash_1_test.bin" );
    s_flash_1_driver->open( "flash_1_test.bin", flash_1_cfg );

    /*-------------------------------------------------------------------------
    Inject some default KV test nodes
    -------------------------------------------------------------------------*/
    test_storage.node_dsc.clear();
    test_storage.node_dsc.push_back( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.simple_pod_data,
                                       .pbFields  = SimplePODData_fields,
                                       .dataSize  = SimplePODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.node_dsc.push_back( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                       .writer    = KVWriter_Memcpy,
                                       .reader    = KVReader_Memcpy,
                                       .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                       .pbFields  = KindaComplexPODData_fields,
                                       .dataSize  = KindaComplexPODData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_storage.node_dsc.push_back( { .hashKey   = KEY_ETL_STRING_DATA,
                                       .writer    = KVWriter_EtlString,
                                       .reader    = KVReader_EtlString,
                                       .datacache = &s_kv_cache_backing.etl_string_data,
                                       .pbFields  = StringData_fields,
                                       .dataSize  = StringData_size,
                                       .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    /*-------------------------------------------------------------------------
    Configure the NVM database
    -------------------------------------------------------------------------*/
    test_config.dev_name             = test_dflt_dev_name.c_str();
    test_config.part_name            = test_dflt_partition.c_str();
    test_config.ext_node_dsc         = &test_storage.node_dsc;
    test_config.ext_transcode_buffer = test_storage.transcode_buffer;

    expect::mb$::osal$::createRecursiveMutex( IgnoreParameter(), true );
    CHECK( DB_ERR_NONE == test_kvdb.configure( test_config ) );

    /*-------------------------------------------------------------------------
    Initialize the NVM database
    -------------------------------------------------------------------------*/
    expect::mb$::system$::atexit$::registerCallback( harness::system::atexit::stub_atexit_do_nothing, IgnoreParameter(), true );

    CHECK( test_kvdb.init() );
  }

  void teardown()
  {
    mock().checkExpectations();

    /*-------------------------------------------------------------------------
    Tear down the NVM database
    -------------------------------------------------------------------------*/
    test_kvdb.deinit();

    /*-------------------------------------------------------------------------
    Destroy virtual backing memory
    -------------------------------------------------------------------------*/
    s_flash_0_driver->close();
    s_flash_1_driver->close();

    delete s_flash_0_driver;
    delete s_flash_1_driver;

    /*-------------------------------------------------------------------------
    Tear down the test data
    -------------------------------------------------------------------------*/
    mock().clear();
    mock().removeAllComparatorsAndCopiers();
  }
};

TEST( db_kv_nvm, init_unable_to_acquire_mutex_resources )
{
  NvmKVDB nvm_db;
  RamKVDB ram_db;

  mock().clear();
  mock().ignoreOtherCalls();

  /*-------------------------------------------------------------------------
  Configure the NVM database
  -------------------------------------------------------------------------*/
  test_config.dev_name             = test_dflt_dev_name.c_str();
  test_config.part_name            = test_dflt_partition.c_str();
  test_config.ext_node_dsc         = &test_storage.node_dsc;
  test_config.ext_transcode_buffer = test_storage.transcode_buffer;

  /*---------------------------------------------------------------------------
  Force a failure in the call to RamKVDB::init()
  ---------------------------------------------------------------------------*/
  CHECK( !nvm_db.init() );

  /*---------------------------------------------------------------------------
  Now allow the RamKVDB::init() to pass, but the NvmKVDB::init() to fail
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::createRecursiveMutex( IgnoreParameter(), true );
  CHECK( DB_ERR_NONE == nvm_db.configure( test_config ) );

  expect::mb$::system$::atexit$::registerCallback( harness::system::atexit::stub_atexit_do_nothing, IgnoreParameter(), true );
  CHECK( nvm_db.init() );
}

TEST( db_kv_nvm, construction_of_invalid_database_fails )
{
  NvmKVDB         kvdb;
  NvmKVDB::Config config;

  /*---------------------------------------------------------------------------
  Missing device name
  ---------------------------------------------------------------------------*/
  config.dev_name  = "";
  config.part_name = "hello";

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing partition name
  ---------------------------------------------------------------------------*/
  config.dev_name  = "hello";
  config.part_name = "";

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing RAM KVDB
  ---------------------------------------------------------------------------*/
  config.dev_name  = "hello";
  config.part_name = "hello";
  // TODO

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  All member methods should fail
  ---------------------------------------------------------------------------*/
  CHECK( false == kvdb.init() );
  CHECK( false == kvdb.insert( {} ) );
  CHECK( nullptr == kvdb.find( 0 ) );
}

TEST( db_kv_nvm, insert_bad_node_fails )
{
  KVNode new_node;
  CHECK( false == test_kvdb.insert( new_node ) );
}

TEST( db_kv_nvm, remove_a_node )
{
  CHECK( test_kvdb.exists( KEY_SIMPLE_POD_DATA ) );
  test_kvdb.remove( KEY_SIMPLE_POD_DATA );
  CHECK( false == test_kvdb.exists( KEY_SIMPLE_POD_DATA ) );
}

TEST( db_kv_nvm, simple_write_then_read )
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

TEST( db_kv_nvm, check_for_nonexistant_node )
{
  /*---------------------------------------------------------------------------
  Standard checks for non-existant nodes
  ---------------------------------------------------------------------------*/
  CHECK( false == test_kvdb.exists( std::numeric_limits<HashKey>::max() ) );

  /*---------------------------------------------------------------------------
  Unable to read due to non-existant node
  ---------------------------------------------------------------------------*/
  uint32_t data;
  CHECK( -1 == test_kvdb.read( std::numeric_limits<HashKey>::max(), &data, sizeof( data ) ) );

  /*---------------------------------------------------------------------------
  Unable to write due to non-existant node
  ---------------------------------------------------------------------------*/
  CHECK( -1 == test_kvdb.write( std::numeric_limits<HashKey>::max(), &data, sizeof( data ) ) );
}

TEST( db_kv_nvm, validate_ram_cache_nonserializable_data_interactions )
{
  KVNode new_node;
  new_node.hashKey   = KEY_RAM_CACHE_ONLY_DATA;
  new_node.datacache = &s_kv_cache_backing.ram_cache_only_data;
  new_node.dataSize  = sizeof( s_kv_cache_backing.ram_cache_only_data );
  new_node.pbFields  = nullptr;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.flags     = KV_FLAG_DEFAULT_VOLATILE;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_RAM_CACHE_ONLY_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  FixedSizeNonSerializableData write_data;
  write_data.random();

  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_RAM_CACHE_ONLY_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Read the data back out
  ---------------------------------------------------------------------------*/
  FixedSizeNonSerializableData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  CHECK( sizeof( read_data ) == test_kvdb.read( KEY_RAM_CACHE_ONLY_DATA, &read_data, sizeof( read_data ) ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, validate_persistent_nonserializable_data_interactions )
{
  KVNode new_node;
  new_node.hashKey   = KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA;
  new_node.datacache = &s_kv_cache_backing.fixed_on_device_data;
  new_node.dataSize  = sizeof( s_kv_cache_backing.fixed_on_device_data );
  new_node.pbFields  = nullptr;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_READ_THROUGH | KV_FLAG_CACHE_POLICY_WRITE_BACK;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  FixedSizeNonSerializableData write_data;
  write_data.random();

  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Flush to ensure we commit
  ---------------------------------------------------------------------------*/
  CHECK( test_kvdb.find( KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA )->flags & KV_FLAG_DIRTY );
  test_kvdb.flush();

  /*---------------------------------------------------------------------------
  Read the data back out
  ---------------------------------------------------------------------------*/
  FixedSizeNonSerializableData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  CHECK( sizeof( read_data ) == test_kvdb.read( KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA, &read_data, sizeof( read_data ) ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, calling_configure_after_configuration )
{
  // The setup step has already called "configure" successfully once
  CHECK( DB_ERR_NOT_AVAILABLE == test_kvdb.configure( test_config ) );
}

TEST( db_kv_nvm, rw_policy_0 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Delayed write back using RAM cache
    - Read through to only pull data from NVM
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_BACK | KV_FLAG_CACHE_POLICY_READ_THROUGH;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = 0x55;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = 0x55;
  write_data.data.bytes[ 1 ] = 0x55;
  write_data.data.bytes[ 2 ] = 0x55;

  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Validate the dirty bit is set, indicating the data is not in sync with NVM
  ---------------------------------------------------------------------------*/
  auto *node = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  CHECK( nullptr != node );
  CHECK( node->flags & KV_FLAG_DIRTY );

  /*---------------------------------------------------------------------------
  Perform a read, which should pull directly from NVM and not reflect the new
  state just yet. This call could return anything.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 != memcmp( &write_data, &read_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Flush the data to NVM, synchronizing the cache with the NVM
  ---------------------------------------------------------------------------*/
  test_kvdb.flush();

  /*---------------------------------------------------------------------------
  Perform the read again, which should now reflect the new state
  ---------------------------------------------------------------------------*/
  memset( &read_data, 0, sizeof( read_data ) );
  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_1 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Immediate write through to NVM
    - Read through to only pull data from NVM
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_THROUGH;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Validate the dirty bit is NOT set.
  ---------------------------------------------------------------------------*/
  auto *node = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  CHECK( nullptr != node );
  CHECK( false == ( node->flags & KV_FLAG_DIRTY ) );

  /*---------------------------------------------------------------------------
  Perform the read, pulling directly from NVM and reflect the new state.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_2 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Immediate write through to NVM
    - Read only from cache
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_CACHE;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data. First validate it's not in the cache.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( 0 != memcmp( &s_kv_cache_backing.variable_pod_data, &write_data, sizeof( write_data ) ) );
  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Validate the dirty bit is NOT set.
  ---------------------------------------------------------------------------*/
  auto *node = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  CHECK( nullptr != node );
  CHECK( false == ( node->flags & KV_FLAG_DIRTY ) );

  /*---------------------------------------------------------------------------
  Check the RAM cache directly to ensure the data was committed
  ---------------------------------------------------------------------------*/
  CHECK( 0 == memcmp( &s_kv_cache_backing.variable_pod_data, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Do the read. This is a less interesting test as you can't tell explicitly if
  the desired path was taken due to no mocks in the call tree.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_3 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Write policy doesn't matter
    - Read sync to ensure RAM cache is up to date with any changes in NVM
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_SYNC;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );
  test_kvdb.flush();

  /*---------------------------------------------------------------------------
  Reset the cache to ensure the read sync is working. We should pull from NVM.
  ---------------------------------------------------------------------------*/
  auto stored_node = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  memset( stored_node->datacache, 0, stored_node->dataSize );
  CHECK( 0 != memcmp( stored_node->datacache, &write_data, stored_node->dataSize ) );

  /*---------------------------------------------------------------------------
  Perform the read-sync. This should pull from NVM and update the cache.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_4 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy: CONFLICTING
    - Immediate write through to NVM
    - Read from cache AND NVM
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_CACHE |
                   KV_FLAG_CACHE_POLICY_READ_THROUGH;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( 0 != memcmp( &s_kv_cache_backing.variable_pod_data, &write_data, sizeof( write_data ) ) );
  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Perform the read. Should fail as the policy is conflicting.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  CHECK( -1 == test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) ) );
  CHECK( 0 != memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_5 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy: MISSING
    - Immediate write through to NVM
    - Oops, forgot to set a read policy
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( 0 != memcmp( &s_kv_cache_backing.variable_pod_data, &write_data, sizeof( write_data ) ) );
  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Perform the read. Should fail as the policy is missing.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  CHECK( -1 == test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) ) );
  CHECK( 0 != memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_6 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy: SANITIZE ON WRITE
    - Write back | sanitize
    - Read cache
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.sanitizer = KVSanitizer_VariableSizedPODData;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags =
      KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_BACK | KV_FLAG_CACHE_POLICY_READ_CACHE | KV_FLAG_SANITIZE_ON_WRITE;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data. Sanitizer will overwrite this to zero.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  /* write_data gets sanitized, so we need to copy original state for comparing */
  VariableSizedPODData write_data_copy = write_data;

  test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) );

  /*---------------------------------------------------------------------------
  Perform the read
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;

  CHECK( test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) ) );
  CHECK( 0 != memcmp( &write_data_copy, &read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_7 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy: SANITIZE ON READ
    - Write back
    - Read cache | sanitize
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.sanitizer = KVSanitizer_VariableSizedPODData;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags =
      KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_BACK | KV_FLAG_CACHE_POLICY_READ_CACHE | KV_FLAG_SANITIZE_ON_READ;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  VariableSizedPODData write_data_copy = write_data;

  test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) );

  /* Ensure no on-write sanitization happened */
  CHECK( 0 == memcmp( &write_data, &write_data_copy, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Perform the read
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  VariableSizedPODData exp_read_data;
  memcpy( &read_data, &write_data, sizeof( write_data ) );
  memset( &exp_read_data, 0, sizeof( exp_read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &read_data, &exp_read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_8 )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy: SANITIZE ON READ
    - Write through
    - Read through | sanitize
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.sanitizer = KVSanitizer_VariableSizedPODData;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags =
      KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_THROUGH | KV_FLAG_SANITIZE_ON_READ;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  VariableSizedPODData write_data_copy = write_data;

  test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) );

  /* Ensure no on-write sanitization happened */
  CHECK( 0 == memcmp( &write_data, &write_data_copy, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Perform the read
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  VariableSizedPODData exp_read_data;
  memcpy( &read_data, &write_data, sizeof( write_data ) );
  memset( &exp_read_data, 0, sizeof( exp_read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &read_data, &exp_read_data, sizeof( write_data ) ) );
}

TEST( db_kv_nvm, rw_policy_missing )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Write policy doesn't exist
    - Read policy doesn't exist
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT;

  // Insert will attempt to write to NVM, which will fail due to no write policy
  CHECK( false == test_kvdb.insert( new_node ) );

  // Temporarily insert the node with policies just to register it
  new_node.flags = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_READ_THROUGH | KV_FLAG_CACHE_POLICY_WRITE_BACK;
  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  // Remove the flags on the node
  auto *node  = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  node->flags = KV_FLAG_PERSISTENT;
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data. This should fail as there is no write policy.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  CHECK( -1 == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Read the data back out. This should fail as well due to no read policy.
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  CHECK( -1 == test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) ) );
}

TEST( db_kv_nvm, encode_decode_data )
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

TEST( db_kv_nvm, sync )
{
  /*---------------------------------------------------------------------------
  Construct a new node with a complex caching policy:
    - Immediate write to NVM
    - Read only from the cache
  ---------------------------------------------------------------------------*/
  KVNode new_node;
  new_node.hashKey   = KEY_VARIABLE_SIZED_POD_DATA;
  new_node.writer    = KVWriter_Memcpy;
  new_node.reader    = KVReader_Memcpy;
  new_node.datacache = &s_kv_cache_backing.variable_pod_data;
  new_node.pbFields  = VariableSizedPODData_fields;
  new_node.dataSize  = VariableSizedPODData_size;
  new_node.flags     = KV_FLAG_PERSISTENT | KV_FLAG_CACHE_POLICY_WRITE_THROUGH | KV_FLAG_CACHE_POLICY_READ_CACHE;

  CHECK( test_kvdb.insert( new_node ) );
  CHECK( test_kvdb.exists( KEY_VARIABLE_SIZED_POD_DATA ) );

  /*---------------------------------------------------------------------------
  Write some data to NVM
  ---------------------------------------------------------------------------*/
  VariableSizedPODData write_data;
  memset( &write_data, 0, sizeof( write_data ) );

  write_data.value           = rand() % 0xFF;
  write_data.data.size       = 3;
  write_data.data.bytes[ 0 ] = rand() % 0xFF;
  write_data.data.bytes[ 1 ] = rand() % 0xFF;
  write_data.data.bytes[ 2 ] = rand() % 0xFF;

  CHECK( sizeof( write_data ) == test_kvdb.write( KEY_VARIABLE_SIZED_POD_DATA, &write_data, sizeof( write_data ) ) );

  /*---------------------------------------------------------------------------
  Modify the cache to ensure it's out of sync with NVM
  ---------------------------------------------------------------------------*/
  auto *node = test_kvdb.find( KEY_VARIABLE_SIZED_POD_DATA );
  memset( node->datacache, 0, node->dataSize );
  CHECK( 0 != memcmp( node->datacache, &write_data, node->dataSize ) );

  /*---------------------------------------------------------------------------
  Sync the cache with NVM
  ---------------------------------------------------------------------------*/
  test_kvdb.sync();

  /*---------------------------------------------------------------------------
  Validate the cache is now in sync with NVM
  ---------------------------------------------------------------------------*/
  VariableSizedPODData read_data;
  memset( &read_data, 0, sizeof( read_data ) );

  test_kvdb.read( KEY_VARIABLE_SIZED_POD_DATA, &read_data, sizeof( read_data ) );
  CHECK( 0 == memcmp( &write_data, &read_data, sizeof( write_data ) ) );
}

/*-----------------------------------------------------------------------------
Utility Functions
-----------------------------------------------------------------------------*/

TEST_GROUP( db_utility ){};

TEST( db_utility, flashdb_error_to_string )
{
  STRCMP_EQUAL( "No error", fdb_err_to_str( FDB_NO_ERR ) );
  STRCMP_EQUAL( "Erase error", fdb_err_to_str( FDB_ERASE_ERR ) );
  STRCMP_EQUAL( "Write error", fdb_err_to_str( FDB_WRITE_ERR ) );
  STRCMP_EQUAL( "Read error", fdb_err_to_str( FDB_READ_ERR ) );
  STRCMP_EQUAL( "Partition not found", fdb_err_to_str( FDB_PART_NOT_FOUND ) );
  STRCMP_EQUAL( "Key-Value name error", fdb_err_to_str( FDB_KV_NAME_ERR ) );
  STRCMP_EQUAL( "Key-Value name exists", fdb_err_to_str( FDB_KV_NAME_EXIST ) );
  STRCMP_EQUAL( "Saved full", fdb_err_to_str( FDB_SAVED_FULL ) );
  STRCMP_EQUAL( "Initialization failed", fdb_err_to_str( FDB_INIT_FAILED ) );
  STRCMP_EQUAL( "Unknown error", fdb_err_to_str( static_cast<fdb_err_t>( 0xFF ) ) );
}

TEST( db_utility, hashing_utilities )
{
  /*---------------------------------------------------------------------------
  String hashing and consistency
  ---------------------------------------------------------------------------*/
  CHECK( hash( "hello!" ) );
  CHECK( hash( "hello!" ) == hash( "hello!" ) );

  /*---------------------------------------------------------------------------
  Object hashing and consistency
  ---------------------------------------------------------------------------*/
  FixedSizeNonSerializableData data;
  data.random();

  CHECK( hash( &data, sizeof( data ) ) );
  CHECK( hash( &data, sizeof( data ) ) == hash( &data, sizeof( data ) ) );
}

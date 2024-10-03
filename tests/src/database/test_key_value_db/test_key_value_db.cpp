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

static KVRAMData                      s_kv_cache_backing;
static RamKVDB::Storage<20, 512>      s_kv_ram_storage;
static NvmKVDB::Storage<20, 512>      s_kv_nvm_storage;
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
KVNode Tests
-----------------------------------------------------------------------------*/

/**
 * @brief Test harness for KVNode testing
 */
class KVNodeHarness
{
public:
  etl::span<uint8_t> transcode_buffer;

  SanitizeFunc sanitize_delegate;
  size_t       sanitize_callback_calls;

  ValidateFunc validate_delegate;
  size_t       validate_callback_calls;

  WriteFunc write_delegate;
  size_t    write_callback_calls;

  ReadFunc read_delegate;
  size_t   read_callback_calls;

  void reset()
  {
    sanitize_callback_calls = 0;
    sanitize_delegate       = SanitizeFunc::create<KVNodeHarness, &KVNodeHarness::_cb_sanitize>( *this );

    validate_callback_calls = 0;
    validate_delegate       = ValidateFunc::create<KVNodeHarness, &KVNodeHarness::_cb_validate>( *this );

    write_callback_calls = 0;
    write_delegate       = WriteFunc::create<KVNodeHarness, &KVNodeHarness::_cb_write>( *this );

    read_callback_calls = 0;
    read_delegate       = ReadFunc::create<KVNodeHarness, &KVNodeHarness::_cb_read>( *this );

    _txcode_storage.fill( 0 );
    transcode_buffer = { _txcode_storage.data(), _txcode_storage.size() };
  }

private:
  etl::array<uint8_t, 512> _txcode_storage;

  bool _cb_validate( const KVNode &node )
  {
    validate_callback_calls++;
    return true;
  }

  void _cb_sanitize( KVNode &node )
  {
    sanitize_callback_calls++;
  }

  bool _cb_write( KVNode &node, const void *data, const size_t size )
  {
    write_callback_calls++;
    return true;
  }

  int _cb_read( const KVNode &node, void *data, const size_t size )
  {
    read_callback_calls++;
    return static_cast<int>( size );
  }
};

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
  RamKVDB         test_kvdb;
  RamKVDB::Config test_config;

  void setup()
  {
    /*-------------------------------------------------------------------------
    Configure a basic RAM database
    -------------------------------------------------------------------------*/
    s_kv_ram_storage.nodes.clear();
    s_kv_ram_storage.nodes.push_back( { .hashKey   = KEY_SIMPLE_POD_DATA,
                                        .writer    = KVWriter_Memcpy,
                                        .reader    = KVReader_Memcpy,
                                        .datacache = &s_kv_cache_backing.simple_pod_data,
                                        .pbFields  = SimplePODData_fields,
                                        .dataSize  = SimplePODData_size,
                                        .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    s_kv_ram_storage.nodes.push_back( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                        .writer    = KVWriter_Memcpy,
                                        .reader    = KVReader_Memcpy,
                                        .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                        .pbFields  = KindaComplexPODData_fields,
                                        .dataSize  = KindaComplexPODData_size,
                                        .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    s_kv_ram_storage.nodes.push_back( { .hashKey   = KEY_ETL_STRING_DATA,
                                        .writer    = KVWriter_EtlString,
                                        .reader    = KVReader_EtlString,
                                        .datacache = &s_kv_cache_backing.etl_string_data,
                                        .pbFields  = StringData_fields,
                                        .dataSize  = StringData_size,
                                        .flags     = KV_FLAG_DEFAULT_VOLATILE } );

    test_config.node_storage     = &s_kv_ram_storage.nodes;
    test_config.transcode_buffer = s_kv_ram_storage.transcode_buffer;

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
  config.node_storage     = &s_kv_ram_storage.nodes;
  config.transcode_buffer = { s_kv_ram_storage.transcode_buffer };

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
  config.transcode_buffer = { s_kv_ram_storage.transcode_buffer };

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Missing transcode buffer
  ---------------------------------------------------------------------------*/
  config.node_storage     = &s_kv_ram_storage.nodes;
  config.transcode_buffer = {};

  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );

  /*---------------------------------------------------------------------------
  Bad KVNode in the storage
  ---------------------------------------------------------------------------*/
  config.node_storage     = &s_kv_ram_storage.nodes;
  config.transcode_buffer = { s_kv_ram_storage.transcode_buffer };

  s_kv_ram_storage.nodes.push_back( {} );
  CHECK( DB_ERR_BAD_ARG == kvdb.configure( config ) );
}

TEST( db_kv_ram, configure_transcode_buffer_too_small )
{
  /*---------------------------------------------------------------------------
  Setup the configuration
  ---------------------------------------------------------------------------*/
  RamKVDB         kvdb;
  RamKVDB::Config config;
  config.node_storage     = &s_kv_ram_storage.nodes;
  config.transcode_buffer = { s_kv_ram_storage.transcode_buffer.data(), 1 };

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
                                     .dataSize  = SimplePODData_size,
                                     .flags     = KV_FLAG_DEFAULT_PERSISTENT } );

    s_kv_nvm_storage.ramdb.insert( { .hashKey   = KEY_KINDA_COMPLEX_POD_DATA,
                                     .datacache = &s_kv_cache_backing.kinda_complex_pod_data,
                                     .pbFields  = KindaComplexPODData_fields,
                                     .dataSize  = KindaComplexPODData_size,
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

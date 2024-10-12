/******************************************************************************
 *  File Name:
 *    test_kv_db_harness.hpp
 *
 *  Description:
 *    KVDB test harness
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef KVDB_TESTING_HARNESS_HPP
#define KVDB_TESTING_HARNESS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <random>
#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <mbedutils/database.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>
#include <mbedutils/drivers/system/atexit.hpp>
#include "test_kv_db.pb.h"

using namespace mb::db;

/*-----------------------------------------------------------------------------
Enumerations
-----------------------------------------------------------------------------*/

/**
 * @brief Enumerated keys for interacting with the KVDB
 */
enum KVAppKeys : HashKey
{
  KEY_SIMPLE_POD_DATA,
  KEY_KINDA_COMPLEX_POD_DATA,
  KEY_ETL_STRING_DATA,
  KEY_GYRO_DATA,
  KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA,
  KEY_VARIABLE_SIZE_NON_SERIALIZABLE_DATA,
  KEY_VARIABLE_SIZED_POD_DATA,
  KEY_RAM_CACHE_ONLY_DATA,

  KEY_ENUM_COUNT
};

/*-----------------------------------------------------------------------------
Structures
-----------------------------------------------------------------------------*/

union ExtraData
{
  int32_t  int_val;
  uint32_t uint_val;
  float    float_val;
  double   double_val;
  char     char_val;
};


struct FixedSizeNonSerializableData
{
  uint32_t  value;
  float     float_val;
  uint8_t   _pad[ 16 ];
  char      char_data;
  double    double_val;
  int16_t   short_val;
  uint64_t  long_val;
  bool      bool_flag;
  ExtraData extra_data;

  void random()
  {
    std::random_device                      rd;
    std::mt19937                            gen( rd() );
    std::uniform_int_distribution<uint32_t> dist_uint32;
    std::uniform_real_distribution<float>   dist_float( 0.0f, 1.0f );
    std::uniform_real_distribution<double>  dist_double( 0.0, 1.0 );
    std::uniform_int_distribution<int16_t>  dist_int16;
    std::uniform_int_distribution<uint64_t> dist_uint64;
    std::uniform_int_distribution<int>      dist_char( 0, 255 );
    std::uniform_int_distribution<int>      dist_bool( 0, 1 );

    value              = dist_uint32( gen );
    float_val          = dist_float( gen );
    char_data          = static_cast<char>( dist_char( gen ) );
    double_val         = dist_double( gen );
    short_val          = dist_int16( gen );
    long_val           = dist_uint64( gen );
    bool_flag          = dist_bool( gen );
    extra_data.int_val = dist_uint32( gen );
  }
};

/**
 * @brief Data to be stored that doesn't have a NanoPB descriptor.
 *
 * This would represent some application data that is never sent off the
 * device and the user didn't really want to write a NanoPB descriptor for.
 * It will be a less efficient storage mechanism, but it's useful.
 */
struct VariableSizeNonSerializableData
{
  uint32_t value;
  float    float_val;
  uint8_t  string_len;
  char     string_data[ 127 ]; /* Variable sized data. This is what NanoPB could help with. */
};

/**
 * @brief RAM memory backing for the KVDB
 */
struct KVRAMData
{
  SimplePODData                   simple_pod_data;         /**< KEY_SIMPLE_POD_DATA */
  KindaComplexPODData             kinda_complex_pod_data;  /**< KEY_KINDA_COMPLEX_POD_DATA */
  etl::string<32>                 etl_string_data;         /**< KEY_ETL_STRING_DATA */
  GyroSensorData                  gyro_data;               /**< KEY_GYRO_DATA */
  FixedSizeNonSerializableData    fixed_on_device_data;    /**< KEY_FIXED_SIZE_NON_SERIALIZABLE_DATA */
  VariableSizeNonSerializableData variable_on_device_data; /**< KEY_VARIABLE_SIZE_NON_SERIALIZABLE_DATA */
  VariableSizedPODData            variable_pod_data;       /**< KEY_VARIABLE_SIZED_POD_DATA */
  FixedSizeNonSerializableData    ram_cache_only_data;     /**< KEY_RAM_CACHE_ONLY_DATA */

  void clear()
  {
    simple_pod_data        = SimplePODData_init_default;
    kinda_complex_pod_data = KindaComplexPODData_init_default;
    etl_string_data.clear();
    gyro_data               = GyroSensorData_init_default;
    fixed_on_device_data    = {};
    variable_on_device_data = {};
    variable_pod_data       = VariableSizedPODData_init_default;
    ram_cache_only_data     = {};
  }
};

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/

/**
 * @brief Sanitize callback for VariableSizedPODData to set all values to zero
 */
static void sanitize_callback_VariableSizedPODData( KVNode &node, void *data, const size_t size )
{
  VariableSizedPODData *view = reinterpret_cast<VariableSizedPODData *>( data );

  view->value     = 0;
  view->data.size = 0;
  memset( view->data.bytes, 0, sizeof( view->data.bytes ) );
}

static constexpr auto KVSanitizer_VariableSizedPODData = SanitizeFunc::create<sanitize_callback_VariableSizedPODData>();

/*-----------------------------------------------------------------------------
Classes
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

  bool _cb_validate( const KVNode &node, void *data, const size_t size )
  {
    validate_callback_calls++;
    return true;
  }

  void _cb_sanitize( KVNode &node, void *data, const size_t size )
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


#endif /* !KVDB_TESTING_HARNESS_HPP */

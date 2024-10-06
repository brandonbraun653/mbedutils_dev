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
};

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


#endif  /* !KVDB_TESTING_HARNESS_HPP */

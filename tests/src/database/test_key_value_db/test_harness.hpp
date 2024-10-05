/******************************************************************************
 *  File Name:
 *    test_harness.hpp
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

using namespace mb::db;


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

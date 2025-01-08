/******************************************************************************
 *  File Name:
 *    test_logging_sink_tsdb.cpp
 *
 *  Description:
 *    Test cases for the TimeSeries Database logging sink.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/

#include <cstdint>
#include <mbedutils/database.hpp>
#include <mbedutils/logging.hpp>

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <CppUTest/CommandLineTestRunner.h>

#include "CppUTest/UtestMacros.h"
#include "assert_expect.hpp"
#include "time_intf_expect.hpp"
#include "nor_flash_file.hpp"

using namespace mb::db;
using namespace CppUMockGen;

/*-----------------------------------------------------------------------------
Static Data
-----------------------------------------------------------------------------*/

static fake::memory::nor::FileFlash *s_flash_0_driver;

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
}

/*-----------------------------------------------------------------------------
Public Functions
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  return RUN_ALL_TESTS( argc, argv );
}

/*-----------------------------------------------------------------------------
TSDBSink Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( tsdb_sink )
{
  mb::logging::TSDBSink* test_sink;

  void setup()
  {
    /*-------------------------------------------------------------------------
    Configure the flash devices
    -------------------------------------------------------------------------*/
    s_flash_0_driver = new fake::memory::nor::FileFlash();

    mb::memory::nor::DeviceConfig flash_0_cfg;
    flash_0_cfg.dev_attr.block_size = fdb_nor_flash0.blk_size;
    flash_0_cfg.dev_attr.size       = fdb_nor_flash0.len;

    std::remove( "flash_0_test.bin" );
    s_flash_0_driver->open( "flash_0_test.bin", flash_0_cfg );

    /*-------------------------------------------------------------------------
    Prepare mocks for the test
    -------------------------------------------------------------------------*/
    mock().clear();
    mock().ignoreOtherCalls();
  }

  void teardown()
  {
    /*-------------------------------------------------------------------------
    Verify test expectations
    -------------------------------------------------------------------------*/
    mock().checkExpectations();

    /*-------------------------------------------------------------------------
    Destroy virtual backing memory
    -------------------------------------------------------------------------*/
    s_flash_0_driver->close();
    delete s_flash_0_driver;

    /*-------------------------------------------------------------------------
    Tear down the test data
    -------------------------------------------------------------------------*/
    mock().clear();
  }
};

TEST( tsdb_sink, configure_with_invalid_arguments )
{
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;

  /*---------------------------------------------------------------------------
  Test Case: Empty device name
  ---------------------------------------------------------------------------*/
  config.dev_name      = "";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_FAIL );

  /*---------------------------------------------------------------------------
  Test Case: Empty partition name
  ---------------------------------------------------------------------------*/
  config.dev_name      = "nor_flash_0";
  config.part_name     = "";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_FAIL );

  /*---------------------------------------------------------------------------
  Test Case: Zero max log size
  ---------------------------------------------------------------------------*/
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 0;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_FAIL );

  delete test_sink;
}

TEST( tsdb_sink, configure_with_valid_arguments )
{
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;

  /*---------------------------------------------------------------------------
  Test Case: Valid configuration
  ---------------------------------------------------------------------------*/
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );

  delete test_sink;
}

TEST( tsdb_sink, close )
{
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;

  /*---------------------------------------------------------------------------
  Test Case: Close before configuration
  ---------------------------------------------------------------------------*/
  CHECK( test_sink->close() == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Close after valid configuration
  ---------------------------------------------------------------------------*/
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );
  CHECK( test_sink->close() == mb::logging::ErrCode::ERR_OK );

  delete test_sink;
}

TEST( tsdb_sink, flush )
{
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;

  /*---------------------------------------------------------------------------
  Test Case: Flush before configuration
  ---------------------------------------------------------------------------*/
  CHECK( test_sink->flush() == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Flush after valid configuration
  ---------------------------------------------------------------------------*/
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );
  CHECK( test_sink->flush() == mb::logging::ErrCode::ERR_OK );

  delete test_sink;
}

TEST( tsdb_sink, insert_bad_args )
{
  /*---------------------------------------------------------------------------
  Configure the sink
  ---------------------------------------------------------------------------*/
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Not enabled
  ---------------------------------------------------------------------------*/
  test_sink->enabled = false;
  CHECK( test_sink->insert( mb::logging::Level::LVL_DEBUG, "hello", 5 ) == mb::logging::ErrCode::ERR_FAIL );

  /*---------------------------------------------------------------------------
  Test Case: Log Level too high
  ---------------------------------------------------------------------------*/
  test_sink->enabled = true;
  test_sink->logLevel = mb::logging::Level::LVL_INFO;
  CHECK( test_sink->insert( mb::logging::Level::LVL_DEBUG, "hello", 5 ) == mb::logging::ErrCode::ERR_FAIL );

  /*-------------------------------------------------------------------------
  Test Case: Null message
  -------------------------------------------------------------------------*/
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, nullptr, 5 ) == mb::logging::ErrCode::ERR_FAIL );

  /*-------------------------------------------------------------------------
  Test Case: Zero length message
  -------------------------------------------------------------------------*/
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, "hello", 0 ) == mb::logging::ErrCode::ERR_FAIL );

  delete test_sink;
}

TEST( tsdb_sink, insert_nominal )
{
  /*---------------------------------------------------------------------------
  Configure the sink
  ---------------------------------------------------------------------------*/
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = nullptr;

  test_sink->configure( config );
  test_sink->enabled  = true;
  test_sink->logLevel = mb::logging::Level::LVL_INFO;
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Valid message
  ---------------------------------------------------------------------------*/
  expect::mb$::time$::micros( 5000 );
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, "hello", 5 ) == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Insert a second message
  ---------------------------------------------------------------------------*/
  expect::mb$::time$::micros( 5001 );
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, "hello", 5 ) == mb::logging::ErrCode::ERR_OK );

  delete test_sink;
}

/*-----------------------------------------------------------------------------
Test Case: Read back data
-----------------------------------------------------------------------------*/

static size_t s_simple_read_back_count = 0;
static bool cb_simple_read_back_reverse( const void *const message, const size_t length )
{
  s_simple_read_back_count++;
  switch( s_simple_read_back_count )
  {
    case 1:
      CHECK( memcmp( message, "goodbye", length ) == 0 );
      break;

    case 2:
      CHECK( memcmp( message, "hello", length ) == 0 );
      break;

    default:
      FAIL( "Unexpected read count" );
      break;
  }

  return false; // Keep reading the next log
}

static bool cb_simple_read_back_forward( const void *const message, const size_t length )
{
  s_simple_read_back_count++;
  switch( s_simple_read_back_count )
  {
    case 1:
      CHECK( memcmp( message, "hello", length ) == 0 );
      break;

    case 2:
      CHECK( memcmp( message, "goodbye", length ) == 0 );
      break;

    default:
      FAIL( "Unexpected read count" );
      break;
  }

  return false; // Keep reading the next log
}

TEST( tsdb_sink, simple_read_back )
{
  /*---------------------------------------------------------------------------
  Configure the sink
  ---------------------------------------------------------------------------*/
  test_sink = new mb::logging::TSDBSink();
  CHECK( test_sink != nullptr );

  mb::logging::TSDBSink::Config config;
  config.dev_name      = "nor_flash_0";
  config.part_name     = "logging";
  config.max_log_size  = 256;
  config.reader_buffer = new uint8_t[ 512 ];

  test_sink->configure( config );
  test_sink->enabled  = true;
  test_sink->logLevel = mb::logging::Level::LVL_INFO;
  CHECK( test_sink->open() == mb::logging::ErrCode::ERR_OK );

  expect::mb$::time$::micros( 5000 );
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, "hello", 5 ) == mb::logging::ErrCode::ERR_OK );
  expect::mb$::time$::micros( 6035 );
  CHECK( test_sink->insert( mb::logging::Level::LVL_INFO, "goodbye", 8 ) == mb::logging::ErrCode::ERR_OK );

  /*---------------------------------------------------------------------------
  Test Case: Read back all messages in reverse
  ---------------------------------------------------------------------------*/
  s_simple_read_back_count = 0;
  auto cb1 = mb::logging::TSDBSink::LogReader::create<cb_simple_read_back_reverse>();

  test_sink->read( cb1, false );
  CHECK( s_simple_read_back_count == 2 );

  /*---------------------------------------------------------------------------
  Test Case: Read back all messages forward
  ---------------------------------------------------------------------------*/
  s_simple_read_back_count = 0;
  auto cb2 = mb::logging::TSDBSink::LogReader::create<cb_simple_read_back_forward>();

  test_sink->read( cb2, true );
  CHECK( s_simple_read_back_count == 2 );

  delete[] config.reader_buffer;
  delete test_sink;
}

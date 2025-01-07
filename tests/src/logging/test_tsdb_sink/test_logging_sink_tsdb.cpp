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

#include <mbedutils/database.hpp>
#include <mbedutils/logging.hpp>

#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <CppUTest/CommandLineTestRunner.h>

#include "CppUMockGen.hpp"
#include "assert_expect.hpp"
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

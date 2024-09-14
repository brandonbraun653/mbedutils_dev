/******************************************************************************
 *  File Name:
 *    test_nor_adesto.cpp
 *
 *  Description:
 *    Test cases for nor_adesto.cpp
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/drivers/threading/thread.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash_device.hpp>

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "gpio_intf_expect.hpp"
#include "spi_intf_expect.hpp"
#include "time_intf_expect.hpp"

using namespace mb::memory;
using namespace mb::memory::nor;

/*-----------------------------------------------------------------------------
Constants
-----------------------------------------------------------------------------*/

static constexpr uint16_t at25sf_ready_flag = 0x0000;
static constexpr uint16_t at25sf_busy_flag  = 0x0001;

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    return RUN_ALL_TESTS(argc, argv);
}


TEST_GROUP( nor_adesto )
{
  DeviceConfig cfg;
  uint8_t      fake_status_register_byte1[ 3 ][ 3 ];
  uint8_t      fake_status_register_byte2[ 3 ][ 3 ];

  void setup()
  {
    memset( fake_status_register_byte1, 0, sizeof( fake_status_register_byte1 ) );
    memset( fake_status_register_byte2, 0, sizeof( fake_status_register_byte2 ) );
    memset( &cfg, 0, sizeof( cfg ) );

    cfg.dev_attr.block_size = 4096;
    cfg.dev_attr.read_size  = 256;
    cfg.dev_attr.write_size = 256;
    cfg.dev_attr.size       = 0x1000000;
    // cfg.dev_attr.start_addr = 0x00000000;
    // cfg.dev_attr.end_addr   = 0x00FFFFFF;
    cfg.dev_attr.erase_latency = 100;
    //cfg.dev_attr.write_latency = 5;
  }

  void teardown()
  {
    mock().checkExpectations();
    mock().clear();
  }

  void setup_static_status_register_return_value( const int call_num, const DeviceConfig &cfg, const uint16_t ret_val )
  {
    /*-------------------------------------------------------------------------
    Set the expectations for the transaction
    -------------------------------------------------------------------------*/
    expect::mb$::hw$::spi$::intf$::lock( 1, cfg.spi_port );
    expect::mb$::hw$::spi$::intf$::unlock( 1, cfg.spi_port );
    expect::mb$::hw$::gpio$::intf$::write( 2, cfg.spi_cs_port, cfg.spi_cs_pin, mb::hw::gpio::State_t::STATE_LOW );
    expect::mb$::hw$::gpio$::intf$::write( 2, cfg.spi_cs_port, cfg.spi_cs_pin, mb::hw::gpio::State_t::STATE_HIGH );

    /*-------------------------------------------------------------------------
    Set the data to be returned for the status register read
    -------------------------------------------------------------------------*/
    fake_status_register_byte1[ call_num ][ 0 ] = 0x00;   // Dummy byte of the transfer
    fake_status_register_byte1[ call_num ][ 1 ] = ret_val & 0xFF;

    fake_status_register_byte2[ call_num ][ 0 ] = 0x00;   // Dummy byte of the transfer
    fake_status_register_byte2[ call_num ][ 1 ] = ( ret_val >> 8 ) & 0xFF;

    /* First call to transfer */
    mock().expectOneCall( "mb::hw::spi::intf::transfer" )
          .withParameter( "port", cfg.spi_port )
          .withOutputParameterReturning( "rx", fake_status_register_byte1[ call_num ], 2 )
          .withParameter( "length", 2 )
          .ignoreOtherParameters();

    /* Second call to transfer */
    mock().expectOneCall( "mb::hw::spi::intf::transfer" )
          .withParameter( "port", cfg.spi_port )
          .withOutputParameterReturning( "rx", fake_status_register_byte2[ call_num ], 2 )
          .withParameter( "length", 2 )
          .ignoreOtherParameters();
  }
};


TEST( nor_adesto, at25sfxxx_pend_event__unsupported_operation )
{
  // Inject an operation that is not supported
  CHECK( Status::ERR_NOT_SUPPORTED == device::adesto_at25sfxxx_pend_event( DeviceConfig(), Event::MEM_ERROR, 0 ) );
}


TEST( nor_adesto, at25sfxxx_pend_event__device_is_not_busy )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  expect::mb$::time$::millis( 1, 100 );

  this->setup_static_status_register_return_value( 0, cfg, at25sf_ready_flag );

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  auto result = device::adesto_at25sfxxx_pend_event( cfg, Event::MEM_ERASE_COMPLETE, 50 );

  /*---------------------------------------------------------------------------
  Verify
  ---------------------------------------------------------------------------*/
  CHECK( result == Status::ERR_OK );
}


TEST( nor_adesto, at25sfxx_pend_event__device_busy_timeout )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  expect::mb$::time$::millis( 1, 100 ); // Initializes start time
  expect::mb$::time$::millis( 1, 110 ); // Cause a second iteration
  expect::mb$::time$::millis( 1, 150 ); // Causes the timeout to occur

  mock().expectOneCall( "mb::time::delayMilliseconds" ).ignoreOtherParameters();

  this->setup_static_status_register_return_value( 0, cfg, at25sf_busy_flag );
  this->setup_static_status_register_return_value( 1, cfg, at25sf_busy_flag );

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  auto result = device::adesto_at25sfxxx_pend_event( cfg, Event::MEM_ERASE_COMPLETE, 25 );

  /*---------------------------------------------------------------------------
  Verify
  ---------------------------------------------------------------------------*/
  CHECK( result == Status::ERR_TIMEOUT );
}


TEST( nor_adesto, at25sfxx_pend_event__device_busy_then_ready )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  expect::mb$::time$::millis( 1, 100 ); // Initializes start time
  expect::mb$::time$::millis( 1, 110 ); // Cause a second iteration
  expect::mb$::time$::millis( 1, 120 ); // Cause a third iteration

  mock().expectNCalls( 2, "mb::time::delayMilliseconds" ).ignoreOtherParameters();

  this->setup_static_status_register_return_value( 0, cfg, at25sf_busy_flag );
  this->setup_static_status_register_return_value( 1, cfg, at25sf_busy_flag );
  this->setup_static_status_register_return_value( 2, cfg, at25sf_ready_flag );

  /*---------------------------------------------------------------------------
  Call FUT
  ---------------------------------------------------------------------------*/
  auto result = device::adesto_at25sfxxx_pend_event( cfg, Event::MEM_WRITE_COMPLETE, 50 );

  /*---------------------------------------------------------------------------
  Verify
  ---------------------------------------------------------------------------*/
  CHECK( result == Status::ERR_OK );
}

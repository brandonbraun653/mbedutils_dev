/******************************************************************************
 *  File Name:
 *    test_nor_flash.cpp
 *
 *  Description:
 *    Test cases for nor_flash.cpp
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/

#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash_device.hpp>
#include <mbedutils/drivers/memory/nvm/jedec_cfi_cmds.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "assert_expect.hpp"
#include "gpio_intf_expect.hpp"
#include "spi_intf_expect.hpp"
#include "mutex_intf_expect.hpp"
#include "nor_flash_device_expect.hpp"

using namespace mb::memory::nor;
using namespace CppUMockGen;


/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    return RUN_ALL_TESTS(argc, argv);
}

TEST_GROUP( nor_flash )
{
  DeviceDriver norDriver;
  DeviceConfig cfg;
  uint32_t input_data;
  uint32_t output_data;

  void setup()
  {
    mock().ignoreOtherCalls();

    /*-------------------------------------------------------------------------
    Initialize the device configuration
    -------------------------------------------------------------------------*/
    memset( &cfg, 0, sizeof( cfg ) );

    cfg.dev_attr.block_size    = 4096;
    cfg.dev_attr.read_size     = 256;
    cfg.dev_attr.write_size    = 256;
    cfg.dev_attr.size          = 0x1000000;
    cfg.dev_attr.erase_latency = 100;
    cfg.pend_event_cb          = device::adesto_at25sfxxx_pend_event;

    /*-------------------------------------------------------------------------
    Initialize the input/output_data buffers
    -------------------------------------------------------------------------*/
    input_data  = 0;
    output_data = 0;

    this->expect_open();
    norDriver.open( cfg );
  }


  void teardown()
  {
    norDriver.close();
    mock().checkExpectations();
    mock().clear();
  }


  void expect_write_enable( const DeviceConfig &cfg )
  {
    using namespace mb::hw;
    using namespace mb::memory;

    /*-------------------------------------------------------------------------
    Set the expectations for "issue_write_enable"
    -------------------------------------------------------------------------*/
    expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
    expect::mb$::hw$::spi$::intf$::write( 1, &cfi::WRITE_ENABLE, cfi::WRITE_ENABLE_OPS_LEN );
    expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  }


  void expect_open()
  {
    /*-------------------------------------------------------------------------
    Set the expectations for "open"
    -------------------------------------------------------------------------*/
    expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
    expect::mb$::assert$::log_assert_failure( true, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(), true );
    expect::mb$::osal$::createRecursiveMutex( IgnoreParameter(), true );
  }
};


TEST( nor_flash, transfer_bad_arguments )
{
  /*-------------------------------------------------------------------------
  Initialize
  -------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  /*-------------------------------------------------------------------------
  Test
  -------------------------------------------------------------------------*/
  norDriver.transfer( nullptr, &output_data, sizeof( output_data ) );
  norDriver.transfer( &input_data, nullptr, sizeof( input_data ) );
  norDriver.transfer( &input_data, &output_data, 0 );
}

TEST( nor_flash, transfer_normal )
{
  using namespace mb::hw;

  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  expect::mb$::osal$::lockMutex( IgnoreParameter() );
  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::transfer( cfg.spi_port, &input_data, &output_data, sizeof( input_data ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::osal$::unlockMutex( IgnoreParameter() );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  norDriver.transfer( &input_data, &output_data, sizeof( input_data ) );
}
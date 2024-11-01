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

#include <mbedutils/drivers/memory/nvm/jedec_cfi_cmds.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash_device.hpp>
#include <mbedutils/drivers/threading/thread.hpp>

#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>
#include "assert_expect.hpp"
#include "gpio_intf_expect.hpp"
#include "spi_intf_expect.hpp"
#include "mutex_intf_expect.hpp"
#include "nor_flash_device_expect.hpp"

using namespace mb::hw;
using namespace mb::memory;
using namespace mb::memory::nor;
using namespace CppUMockGen;


class DeviceConfigComparator : public MockNamedValueComparator
{
public:
  bool isEqual( const void *object1, const void *object2 ) override
  {
    const mb::memory::nor::DeviceConfig *config1 = static_cast<const mb::memory::nor::DeviceConfig *>( object1 );
    const mb::memory::nor::DeviceConfig *config2 = static_cast<const mb::memory::nor::DeviceConfig *>( object2 );
    // Implement your comparison logic here
    return 0 == memcmp( config1, config2, sizeof( mb::memory::nor::DeviceConfig ) );
  }

  SimpleString valueToString( const void *object ) override
  {
    const mb::memory::nor::DeviceConfig *config = static_cast<const mb::memory::nor::DeviceConfig *>( object );
    // Implement your logic to convert DeviceConfig to string
    return SimpleString( "DeviceConfig" );
  }
};

/*-----------------------------------------------------------------------------
Tests
-----------------------------------------------------------------------------*/

int main( int argc, char **argv )
{
  DeviceConfigComparator comparator;
  mock().installComparator( "mb::memory::nor::DeviceConfig", comparator );

  return RUN_ALL_TESTS( argc, argv );
}

TEST_GROUP( nor_flash )
{
  DeviceDriver* norDriver;
  DeviceConfig cfg;
  uint32_t     input_data;
  uint32_t     output_data;

  void setup()
  {
    mock().ignoreOtherCalls();

    /*-------------------------------------------------------------------------
    Initialize the device configuration
    -------------------------------------------------------------------------*/
    memset( &cfg, 0, sizeof( cfg ) );

    cfg.dev_attr.block_size         = 4096;
    cfg.dev_attr.read_size          = 256;
    cfg.dev_attr.write_size         = 256;
    cfg.dev_attr.erase_size         = 4096;
    cfg.dev_attr.size               = 0x1000000;
    cfg.dev_attr.erase_latency      = 100;
    cfg.dev_attr.erase_chip_latency = 10000;
    cfg.dev_attr.write_latency      = 5;
    cfg.pend_event_cb               = device::adesto_at25sfxxx_pend_event;

    /*-------------------------------------------------------------------------
    Initialize the input/output_data buffers
    -------------------------------------------------------------------------*/
    input_data  = 0;
    output_data = 0;

    norDriver = new DeviceDriver();
    norDriver->open( cfg );
  }

  void teardown()
  {
    norDriver->close();
    delete norDriver;
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
    expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(),
                                          static_cast<size_t>( cfi::WRITE_ENABLE_OPS_LEN ) );
    expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  }

};

TEST( nor_flash, open_bad_arguments )
{
  /*-------------------------------------------------------------------------
  Initialize
  -------------------------------------------------------------------------*/
  DeviceConfig invalid_cfg;
  memset( &invalid_cfg, 0, sizeof( invalid_cfg ) );

  /* Reset the test harness */
  norDriver->close();
  mock().clear();
  mock().ignoreOtherCalls();

  expect::mb$::assert$::format_and_log_assert_failure(
    false, IgnoreParameter(), IgnoreParameter(), IgnoreParameter(),
   "Unable to open NOR device, invalid state", false
   );

  /*-------------------------------------------------------------------------
  Test
  -------------------------------------------------------------------------*/
  norDriver->open( invalid_cfg );
}

TEST( nor_flash, unable_to_call_driver_functions_when_not_open )
{
  /*-------------------------------------------------------------------------
  Initialize
  -------------------------------------------------------------------------*/
  norDriver->close();
  mock().clear();
  mock().ignoreOtherCalls();

  /*-------------------------------------------------------------------------
  Test
  -------------------------------------------------------------------------*/
  Status result;
  result = norDriver->write( 0, &input_data, sizeof( input_data ) );
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );

  result = norDriver->read( 0, &output_data, sizeof( output_data ) );
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );

  result = norDriver->erase( 0 );
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );

  result = norDriver->erase();
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );

  result = norDriver->flush();
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );

  result = norDriver->transfer( &input_data, &output_data, sizeof( input_data ) );
  CHECK_EQUAL( Status::ERR_BAD_STATE, result );
}

TEST( nor_flash, transfer_bad_arguments )
{
  /*-------------------------------------------------------------------------
  Initialize
  -------------------------------------------------------------------------*/
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  /*-------------------------------------------------------------------------
  Test
  -------------------------------------------------------------------------*/
  CHECK_EQUAL( Status::ERR_BAD_ARG, norDriver->transfer( nullptr, &output_data, sizeof( output_data ) ) );
  CHECK_EQUAL( Status::ERR_BAD_ARG, norDriver->transfer( &input_data, nullptr, sizeof( input_data ) ) );
  CHECK_EQUAL( Status::ERR_BAD_ARG, norDriver->transfer( &input_data, &output_data, 0 ) );
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
  norDriver->transfer( &input_data, &output_data, sizeof( input_data ) );
}

TEST( nor_flash, write_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/

  /* No data */
  result = Status::ERR_OK;
  result = norDriver->write( 0, nullptr, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );

  /* No length */
  result = Status::ERR_OK;
  result = norDriver->write( 0, &input_data, 0 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );

  /* Size bigger than device */
  result = Status::ERR_OK;
  result = norDriver->write( 0, &input_data, 0x1000001 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );

  /* Size bigger than page */
  result = Status::ERR_OK;
  result = norDriver->write( 0, &input_data, 257 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, write_block_index_too_large )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  size_t req_block_access = ( cfg.dev_attr.size / cfg.dev_attr.write_size ) + 1;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->write( req_block_access, 0, &input_data, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, write_block_offset_exceeds_block_size )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  size_t req_block_access = cfg.dev_attr.size / cfg.dev_attr.write_size;
  size_t req_offset       = cfg.dev_attr.block_size + 1;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->write( req_block_access, req_offset, &input_data, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, write_adress_intf )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status   result;
  uint64_t address = 0x1000;

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  this->expect_write_enable( cfg );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::PAGE_PROGRAM_OPS_LEN ) );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, &input_data, sizeof( input_data ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::memory$::nor$::device$::adesto_at25sfxxx_pend_event( IgnoreParameter(), Event::MEM_WRITE_COMPLETE,
                                                                    cfg.dev_attr.write_latency, Status::ERR_OK );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->write( address, &input_data, sizeof( input_data ) );
}

TEST( nor_flash, read_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/

  /* No data */
  result = Status::ERR_OK;
  result = norDriver->read( 0, nullptr, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );

  /* No length */
  result = Status::ERR_OK;
  result = norDriver->read( 0, &output_data, 0 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );

  /* Size bigger than device */
  result = Status::ERR_OK;
  result = norDriver->read( 0, &output_data, 0x1000001 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, read_block_index_too_large )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  size_t req_block_access = ( cfg.dev_attr.size / cfg.dev_attr.read_size ) + 1;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->read( req_block_access, 0, &output_data, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, read_block_offset_exceeds_block_size )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  size_t req_block_access = cfg.dev_attr.size / cfg.dev_attr.read_size;
  size_t req_offset       = cfg.dev_attr.block_size + 1;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->read( req_block_access, req_offset, &output_data, 55 );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, read_adress_intf )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status   result;
  uint64_t address = 0x1000;

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::READ_ARRAY_HS_OPS_LEN ) );
  expect::mb$::hw$::spi$::intf$::read( 1, cfg.spi_port, &output_data, sizeof( output_data ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->read( address, &output_data, sizeof( output_data ) );
}

TEST( nor_flash, erase_bad_arguments )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  mock().expectNoCall( "mb::hw::spi::intf::lock" );

  size_t erase_block_idx = ( cfg.dev_attr.size / cfg.dev_attr.erase_size ) + 1;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = norDriver->erase( erase_block_idx );
  CHECK_EQUAL( Status::ERR_BAD_ARG, result );
}

TEST( nor_flash, erase_32k_block_size )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  size_t block_idx = 0;

  norDriver->close();
  cfg.dev_attr.erase_size = 32 * 1024;
  norDriver->open( cfg );

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  this->expect_write_enable( cfg );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::BLOCK_ERASE_OPS_LEN ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::memory$::nor$::device$::adesto_at25sfxxx_pend_event( IgnoreParameter(), Event::MEM_ERASE_COMPLETE,
                                                                    cfg.dev_attr.erase_latency, Status::ERR_OK );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->erase( block_idx );
  CHECK_EQUAL( Status::ERR_OK, result );
}

TEST( nor_flash, erase_64k_block_size )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  size_t block_idx = 0;

  norDriver->close();
  cfg.dev_attr.erase_size = 64 * 1024;
  norDriver->open( cfg );

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  this->expect_write_enable( cfg );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::BLOCK_ERASE_OPS_LEN ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::memory$::nor$::device$::adesto_at25sfxxx_pend_event( IgnoreParameter(), Event::MEM_ERASE_COMPLETE,
                                                                    cfg.dev_attr.erase_latency, Status::ERR_OK );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->erase( block_idx );
  CHECK_EQUAL( Status::ERR_OK, result );
}

TEST( nor_flash, erase_128k_invalid_block_size )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  size_t block_idx = 0;

  norDriver->close();
  cfg.dev_attr.erase_size = 128 * 1024;
  norDriver->open( cfg );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->erase( block_idx );
  CHECK_EQUAL( Status::ERR_BAD_CFG, result );
}

TEST( nor_flash, erase_block_intf_normal )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;
  size_t block_idx = 0;

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  this->expect_write_enable( cfg );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::BLOCK_ERASE_OPS_LEN ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::memory$::nor$::device$::adesto_at25sfxxx_pend_event( IgnoreParameter(), Event::MEM_ERASE_COMPLETE,
                                                                    cfg.dev_attr.erase_latency, Status::ERR_OK );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->erase( block_idx );
  CHECK_EQUAL( Status::ERR_OK, result );
}

TEST( nor_flash, erase_chip_intf )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;

  expect::mb$::hw$::spi$::intf$::lock( cfg.spi_port );
  this->expect_write_enable( cfg );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_LOW );
  expect::mb$::hw$::spi$::intf$::write( 1, cfg.spi_port, IgnoreParameter(), static_cast<size_t>( cfi::CHIP_ERASE_OPS_LEN ) );
  expect::mb$::hw$::gpio$::intf$::write( 1, cfg.spi_cs_port, cfg.spi_cs_pin, gpio::State_t::STATE_HIGH );
  expect::mb$::hw$::spi$::intf$::unlock( cfg.spi_port );
  expect::mb$::memory$::nor$::device$::adesto_at25sfxxx_pend_event( IgnoreParameter(), Event::MEM_ERASE_COMPLETE,
                                                                    cfg.dev_attr.erase_chip_latency, Status::ERR_OK );

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = Status::ERR_OK;
  result = norDriver->erase();
  CHECK_EQUAL( Status::ERR_OK, result );
}

TEST( nor_flash, flush )
{
  /*---------------------------------------------------------------------------
  Initialize
  ---------------------------------------------------------------------------*/
  Status result;

  /*---------------------------------------------------------------------------
  Test
  ---------------------------------------------------------------------------*/
  result = norDriver->flush();
  CHECK_EQUAL( Status::ERR_OK, result );
}

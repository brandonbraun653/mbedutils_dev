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
#include <array>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <etl/array.h>
#include <etl/span.h>
#include <etl/vector.h>
#include <mbedutils/database.hpp>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>
#include <memory>
#include <thread>

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/CommandLineTestRunner.h"

#include "assert_expect.hpp"
#include "atexit_expect.hpp"
#include "atexit_harness.hpp"
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
  MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
  return RUN_ALL_TESTS( argc, argv );
}


/*-----------------------------------------------------------------------------
KVNode Tests
-----------------------------------------------------------------------------*/

TEST_GROUP( db_mt )
{
  NvmKVDB                                   test_kvdb;
  NvmKVDB::Config                           test_config;
  Storage<20, 512>                          test_storage;
  harness::system::atexit::CallbackCopier   atexit_callback_copier;
  std::vector<std::unique_ptr<std::thread>> test_threads;
  std::mutex                                test_thread_cv_mtx;
  std::condition_variable                   test_thread_cv;
  int                                       test_thread_count;
  int                                       test_thread_iterations;

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

    CHECK( DB_ERR_NONE == test_kvdb.configure( test_config ) );

    /*-------------------------------------------------------------------------
    Initialize the NVM database
    -------------------------------------------------------------------------*/
    expect::mb$::system$::atexit$::registerCallback( harness::system::atexit::stub_atexit_do_nothing, IgnoreParameter(), true );

    CHECK( test_kvdb.init() );

    /*-------------------------------------------------------------------------
    Clear the thread vector
    -------------------------------------------------------------------------*/
    test_threads.clear();
    test_thread_count      = rand() % 20 + 5;
    test_thread_iterations = rand() % 20 + 5;
  }

  void teardown()
  {
    /*-------------------------------------------------------------------------
    Stop all the test threads
    -------------------------------------------------------------------------*/
    for( auto &thread : test_threads )
    {
      if( thread->joinable() )
      {
        thread->join();
      }
    }

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

  /**
   * @brief Test function for multi-threaded reading from the NVM Key-Value Database.
   *
   * This function is intended to be run in a separate thread to test the
   * multi-threaded read capability of the NVM Key-Value Database. It reads a
   * value associated with a given key from the database and checks if the
   * read value matches the expected value.
   *
   * @param kvdb Pointer to the NVM Key-Value Database instance.
   * @param key The key associated with the value to be read.
   * @param value The expected value to be read from the database.
   */
  void test_thread_multi_reader( NvmKVDB * kvdb, KVAppKeys key, uint32_t value, uint32_t iterations )
  {
    std::unique_lock<std::mutex> lock( test_thread_cv_mtx );
    test_thread_cv.wait( lock );

    for( uint32_t i = 0; i < iterations; i++ )
    {
      uint32_t readback = 0;
      CHECK( kvdb->read( key, &readback, sizeof( readback ) ) );
      CHECK_EQUAL( value, readback );
    }
  }

  /**
   * @brief Test function for multi-threaded writing to the NVM Key-Value Database.
   *
   * This function is intended to be run in a separate thread to test the
   * multi-threaded write capability of the NVM Key-Value Database. It writes a
   * value associated with a given key to the database and checks if the write
   * operation was successful.
   *
   * @param kvdb Pointer to the NVM Key-Value Database instance.
   * @param key The key associated with the value to be written.
   * @param value The value to be written to the database.
   */
  void test_thread_multi_writer( NvmKVDB * kvdb, KVAppKeys key, void *data, size_t size, uint32_t iterations )
  {
    std::unique_lock<std::mutex> lock( test_thread_cv_mtx );
    test_thread_cv.wait( lock );

    for( uint32_t i = 0; i < iterations; i++ )
    {
      CHECK( kvdb->write( key, data, size ) );
    }
  }
};


TEST( db_mt, multi_reader_no_writer )
{
  /*---------------------------------------------------------------------------
  Set up the test data
  ---------------------------------------------------------------------------*/
  SimplePODData simple_pod_data;
  simple_pod_data.value = 42;

  CHECK( test_kvdb.write( KEY_SIMPLE_POD_DATA, &simple_pod_data, sizeof( simple_pod_data ) ) );

  /*---------------------------------------------------------------------------
  Start a bunch of reader threads
  ---------------------------------------------------------------------------*/
  for( int i = 0; i < test_thread_count; ++i )
  {
    auto thread_func = std::bind( &TEST_GROUP_CppUTestGroupdb_mt::test_thread_multi_reader, this, &test_kvdb,
                                  KEY_SIMPLE_POD_DATA, simple_pod_data.value, test_thread_iterations );
    test_threads.push_back( std::make_unique<std::thread>( thread_func ) );
  }

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  test_thread_cv.notify_all();
}

TEST( db_mt, multi_writer_no_reader )
{
  /*---------------------------------------------------------------------------
  Set up the test data
  ---------------------------------------------------------------------------*/
  SimplePODData simple_pod_data;
  simple_pod_data.value = 42;

  /*---------------------------------------------------------------------------
  Start a bunch of writer threads
  ---------------------------------------------------------------------------*/
  for( int i = 0; i < test_thread_count; ++i )
  {
    auto thread_func = std::bind( &TEST_GROUP_CppUTestGroupdb_mt::test_thread_multi_writer, this, &test_kvdb,
                                  KEY_SIMPLE_POD_DATA, &simple_pod_data, sizeof( simple_pod_data ), test_thread_iterations );
    test_threads.push_back( std::make_unique<std::thread>( thread_func ) );
  }

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  test_thread_cv.notify_all();
}

/**
 * @brief Test case for multi-threaded read/write operations on the NVM Key-Value Database.
 *
 * In this particular test case, I'm not interested in data consistency but more
 * so the ability of the database to handle multiple read/write operations without
 * crashing or throwing exceptions.
 */
TEST( db_mt, multi_writer_multi_reader )
{
  /*---------------------------------------------------------------------------
  Set up the test data
  ---------------------------------------------------------------------------*/
  SimplePODData simple_pod_data;
  simple_pod_data.value = 42;

  /*---------------------------------------------------------------------------
  Start a bunch of writer threads
  ---------------------------------------------------------------------------*/
  for( int i = 0; i < test_thread_count; ++i )
  {
    auto thread_func = std::bind( &TEST_GROUP_CppUTestGroupdb_mt::test_thread_multi_writer, this, &test_kvdb,
                                  KEY_SIMPLE_POD_DATA, &simple_pod_data, sizeof( simple_pod_data ), test_thread_iterations );
    test_threads.push_back( std::make_unique<std::thread>( thread_func ) );
  }

  /* Start the write threads to prime the cache with the expected values */
  test_thread_cv.notify_all();

  /*---------------------------------------------------------------------------
  Start a bunch of reader threads
  ---------------------------------------------------------------------------*/
  for( int i = 0; i < test_thread_count; ++i )
  {
    auto thread_func = std::bind( &TEST_GROUP_CppUTestGroupdb_mt::test_thread_multi_reader, this, &test_kvdb,
                                  KEY_SIMPLE_POD_DATA, simple_pod_data.value, test_thread_iterations );
    test_threads.push_back( std::make_unique<std::thread>( thread_func ) );
  }

  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
  test_thread_cv.notify_all();
}

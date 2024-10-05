/******************************************************************************
 *  File Name:
 *    nor_flash_file.cpp
 *
 *  Description:
 *    File backed NOR flash device driver. Intended for testing purposes.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include "nor_flash_file.hpp"

namespace fake::memory::nor
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/

  FileFlash::FileFlash() : mFilePath(), mConfig(), mFile()
  {
  }


  FileFlash::~FileFlash()
  {
    close();
  }


  void FileFlash::open( const std::string &file_path, const mb::memory::nor::DeviceConfig &cfg )
  {
    mFilePath = std::string( file_path.data(), file_path.size() );
    mConfig   = cfg;

    // Create the file if it doesn't exist
    if( !std::filesystem::exists( mFilePath ) )
    {
      std::ofstream create_file( mFilePath, std::ios::binary );
      if( !create_file )
      {
        std::cerr << "Failed to create file: " << mFilePath << std::endl;
        return;
      }
      create_file.close();

      // Initialize the file with 0xFF (erased state for NOR flash)
      std::vector<uint8_t> erased_data( mConfig.dev_attr.size, 0xFF );
      std::ofstream        init_file( mFilePath, std::ios::binary );
      init_file.write( reinterpret_cast<char *>( erased_data.data() ), erased_data.size() );
      init_file.close();
    }

    mFile.open( mFilePath, std::ios::in | std::ios::out | std::ios::binary );
    if( !mFile )
    {
      std::cerr << "Failed to open file: " << mFilePath << std::endl;
    }
  }


  void FileFlash::close()
  {
    if( mFile.is_open() )
    {
      mFile.close();
    }
  }


  mb::memory::Status FileFlash::write( const size_t block_idx, const size_t offset, const void *const data,
                                       const size_t length )
  {
    uint64_t address = block_idx * mConfig.dev_attr.block_size + offset;
    return write( address, data, length );
  }


  mb::memory::Status FileFlash::write( const uint64_t address, const void *const data, const size_t length )
  {
    if( !mFile.is_open() )
    {
      return mb::memory::Status::ERR_NOT_READY;
    }

    if( address + length > mConfig.dev_attr.size )
    {
      return mb::memory::Status::ERR_BAD_ARG;
    }

    mFile.seekp( address, std::ios::beg );
    mFile.write( static_cast<const char *>( data ), length );

    if( mFile.fail() )
    {
      return mb::memory::Status::ERR_GENERIC;
    }

    return mb::memory::Status::ERR_OK;
  }


  mb::memory::Status FileFlash::read( const size_t block_idx, const size_t offset, void *const data, const size_t length )
  {
    uint64_t address = block_idx * mConfig.dev_attr.block_size + offset;
    return read( address, data, length );
  }


  mb::memory::Status FileFlash::read( const uint64_t address, void *const data, const size_t length )
  {
    if( !mFile.is_open() )
    {
      return mb::memory::Status::ERR_NOT_READY;
    }

    if( address + length > mConfig.dev_attr.size )
    {
      return mb::memory::Status::ERR_BAD_ARG;
    }

    mFile.seekg( address, std::ios::beg );
    mFile.read( static_cast<char *>( data ), length );

    if( mFile.fail() )
    {
      return mb::memory::Status::ERR_GENERIC;
    }

    return mb::memory::Status::ERR_OK;
  }


  mb::memory::Status FileFlash::erase( const uint64_t address, const size_t size )
  {
    if( !mFile.is_open() )
    {
      return mb::memory::Status::ERR_NOT_READY;
    }

    if( address + size > mConfig.dev_attr.size )
    {
      return mb::memory::Status::ERR_BAD_ARG;
    }

    std::vector<uint8_t> erased_data( size, 0xFF );
    mFile.seekp( address, std::ios::beg );
    mFile.write( reinterpret_cast<char *>( erased_data.data() ), size );

    if( mFile.fail() )
    {
      return mb::memory::Status::ERR_GENERIC;
    }

    return mb::memory::Status::ERR_OK;
  }


  mb::memory::Status FileFlash::erase( const size_t block_idx )
  {
    uint64_t address = block_idx * mConfig.dev_attr.block_size;
    return erase( address, mConfig.dev_attr.block_size );
  }


  mb::memory::Status FileFlash::erase()
  {
    return erase( 0, mConfig.dev_attr.size );
  }


  mb::memory::Status FileFlash::flush()
  {
    if( !mFile.is_open() )
    {
      return mb::memory::Status::ERR_NOT_READY;
    }

    mFile.flush();

    if( mFile.fail() )
    {
      return mb::memory::Status::ERR_GENERIC;
    }

    return mb::memory::Status::ERR_OK;
  }

}    // namespace fake::memory::nor
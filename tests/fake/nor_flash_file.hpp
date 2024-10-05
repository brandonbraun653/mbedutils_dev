/******************************************************************************
 *  File Name:
 *    nor_flash_file.hpp
 *
 *  Description:
 *    File backed NOR flash device driver. Intended for testing purposes on a
 *    Linux based machine.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef MBEDUTILS_TESTING_FAKE_NOR_FLASH_FILE_HPP
#define MBEDUTILS_TESTING_FAKE_NOR_FLASH_FILE_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <string>
#include <fstream>
#include <mbedutils/drivers/memory/nvm/nor_flash.hpp>

namespace fake::memory::nor
{
  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/

  /**
   * @brief NOR Flash Memory Driver
   *
   * This is a low level driver for interfacing with NOR flash memory devices.
   * All operations are blocking for the current thread, but the driver is
   * thread safe and can be used in a multi-threaded environment.
   */
   class FileFlash : public mb::memory::block_device::IBlockDeviceDriver, public mb::thread::Lockable<FileFlash>
   {
   public:
     FileFlash();
     ~FileFlash();
     void open( const std::string &file_path, const mb::memory::nor::DeviceConfig &cfg );
     void close();

     /*-------------------------------------------------------------------------
     Block Device Interface
     -------------------------------------------------------------------------*/
     mb::memory::Status write( const size_t block_idx, const size_t offset, const void *const data,
                               const size_t length ) final override;
     mb::memory::Status write( const uint64_t address, const void *const data, const size_t length ) final override;
     mb::memory::Status read( const size_t block_idx, const size_t offset, void *const data,
                              const size_t length ) final override;
     mb::memory::Status read( const uint64_t address, void *const data, const size_t length ) final override;
     mb::memory::Status erase( const uint64_t address, const size_t size ) final override;
     mb::memory::Status erase( const size_t block_idx ) final override;
     mb::memory::Status erase() final override;
     mb::memory::Status flush() final override;

   private:
     friend class mb::thread::Lockable<FileFlash>;

     std::string                   mFilePath; /**< Path to the file backing the flash device */
     std::fstream                  mFile;     /**< File stream for reading/writing */
     mb::memory::nor::DeviceConfig mConfig;   /**< Device configuration attributes */
   };

}  // namespace fake::mb::memory::nor

#endif  /* !MBEDUTILS_TESTING_FAKE_NOR_FLASH_FILE_HPP */

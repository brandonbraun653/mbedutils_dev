/******************************************************************************
 *  File Name:
 *    fal_cfg.h
 *
 *  Description:
 *    FlashDB "Flash Abstraction Library" Configuration header for testing
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#ifndef MBEDUTILS_TESTING_FAL_CFG_H
#define MBEDUTILS_TESTING_FAL_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <fdb_cfg.h>

/*-----------------------------------------------------------------------------
Literals
-----------------------------------------------------------------------------*/

#define FAL_PRINTF

extern const struct fal_flash_dev fdb_nor_flash0;
extern const struct fal_flash_dev fdb_nor_flash1;
#define FAL_FLASH_DEV_TABLE \
  {                         \
    &fdb_nor_flash0,        \
    &fdb_nor_flash1,        \
  }

    /*                       partition,        device,     start,    length     */
#define FAL_PART_TABLE                                                            \
  {                                                                               \
    { FAL_PART_MAGIC_WORD,     "kv_db", "nor_flash_0",         0, 1024*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,        "bl", "nor_flash_0",         0,   64*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,       "app", "nor_flash_0",   64*1024,  704*1024, 0 }, \
    { FAL_PART_MAGIC_WORD, "easyflash", "nor_flash_0",         0, 1024*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,  "download", "nor_flash_0", 1024*1024, 1024*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,     "kv_db", "nor_flash_1",         0, 1024*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,        "bl", "nor_flash_1",         0,   64*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,       "app", "nor_flash_1",   64*1024,  704*1024, 0 }, \
    { FAL_PART_MAGIC_WORD, "easyflash", "nor_flash_1",         0, 1024*1024, 0 }, \
    { FAL_PART_MAGIC_WORD,  "download", "nor_flash_1", 1024*1024, 1024*1024, 0 }, \
  }

#ifdef __cplusplus
}
#endif
#endif  /* MBEDUTILS_TESTING_FAL_CFG_H */

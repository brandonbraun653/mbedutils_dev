/******************************************************************************
 *  File Name:
 *    harness_atexit.hpp
 *
 *  Description:
 *    Testing harness utilities for the atexit module
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef MBEDUTILS_TEST_HARNESS_ATEXIT_HPP
#define MBEDUTILS_TEST_HARNESS_ATEXIT_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/drivers/system/atexit.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTestExt/MockSupportPlugin.h"

namespace harness::system::atexit
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Empty callback to do nothing
   */
  static void stub_atexit_callback()
  {
  }

  static auto stub_atexit_do_nothing = mb::system::atexit::Callback::create<stub_atexit_callback>();

  /*---------------------------------------------------------------------------
  Classes
  ---------------------------------------------------------------------------*/

  /**
   * @brief Custom copier for the atexit callback type
   *
   * This is necessary because the atexit callback is a delegate and the default
   * copier does not know how to handle it.
   *
   * @example
   * -- Declaration --
   *  CallbackCopier atexit_callback_copier;
   *
   * -- Setup --
   * mock().installCopier( "mb::system::atexit::Callback", atexit_callback_copier );
   *
   * -- Tear down --
   * mock().removeAllComparatorsAndCopiers();
   */
  class CallbackCopier : public MockNamedValueCopier
  {
  public:
    void copy( void *out, const void *in ) override
    {
      *( mb::system::atexit::Callback * )out = *( const mb::system::atexit::Callback * )in;
    }
  };
}    // namespace harness::mb::system::atexit

#endif /* !MBEDUTILS_TEST_HARNESS_ATEXIT_HPP */

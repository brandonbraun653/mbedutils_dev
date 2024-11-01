/******************************************************************************
 *  File Name:
 *    test_runtime_harness.hpp
 *
 *  Description:
 *    Provides a runtime-agnostic test harness for STL and FreeRTOS based tests
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef MBEDUTILS_TEST_RUNTIME_HARNESS_HPP
#define MBEDUTILS_TEST_RUNTIME_HARNESS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/threading.hpp>
#include <CppUTest/CommandLineTestRunner.h>


namespace TestHarness
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief High level function to run all tests in this test suite.
   *
   * This is what should be called from main() in order to run all tests. It
   * provides the correct runtime to be used depending on compiler arguments.
   *
   * @param argc  Number of arguments
   * @param argv  Argument list
   * @return int  Test result
   */
  int runTests( int argc, char **argv );

}    // namespace TestHarness

#endif /* !MBEDUTILS_TEST_RUNTIME_HARNESS_HPP */

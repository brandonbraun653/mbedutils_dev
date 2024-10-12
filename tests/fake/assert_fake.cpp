/******************************************************************************
 *  File Name:
 *    assert_fake.cpp
 *
 *  Description:
 *    Fake implementation of the assertion interface to be used in testing.
 *    This will cause tests to fail if an assertion is triggered when it should
 *    not have been.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/interfaces/util_intf.hpp>

#include "CppUTest/TestHarness.h"
#include <CppUTestExt/MockSupport.h>

namespace mb::assert
{
  bool log_assert_failure( const bool predicate, const bool halt, const char *const file, const int line )
  {
    /*-------------------------------------------------------------------------
    Register the call with the mock framework so we can set expectations
    -------------------------------------------------------------------------*/
    mock()
        .actualCall( "mb::assert::log_assert_failure" )
        .withBoolParameter( "predicate", predicate )
        .withBoolParameter( "halt", halt )
        .withStringParameter( "file", file )
        .withIntParameter( "line", line )
        .returnBoolValue();

    /*-------------------------------------------------------------------------
    Format the message and check the predicate. Enforce the assert on halt
    because those represent true system failures that need to be addressed.
    -------------------------------------------------------------------------*/
    char buffer[ 512 ];
    snprintf( buffer, sizeof( buffer ), "Assertion failed at %s:%d", file, line );

    if( halt )
    {
      CHECK_TEXT( predicate, buffer );
    }

    return predicate;
  }

  bool format_and_log_assert_failure( const bool predicate, const bool halt, const char *const file, const int line,
                                      const char *fmt, ... )
  {
    /*-------------------------------------------------------------------------
    Register the call with the mock framework so we can set expectations
    -------------------------------------------------------------------------*/
    mock()
        .actualCall( "mb::assert::format_and_log_assert_failure" )
        .withBoolParameter( "predicate", predicate )
        .withBoolParameter( "halt", halt )
        .withStringParameter( "file", file )
        .withIntParameter( "line", line )
        .withStringParameter( "fmt", fmt )
        .returnBoolValue();

    /*-------------------------------------------------------------------------
    Format the message and check the predicate. Enforce the assert on halt
    because those represent true system failures that need to be addressed.
    -------------------------------------------------------------------------*/
    char    buffer[ 512 ];
    va_list args;
    va_start( args, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, args );
    va_end( args );

    if( halt )
    {
      CHECK_TEXT( predicate, buffer );
    }

    return predicate;
  }
}    // namespace mb::assert

/******************************************************************************
 *  File Name:
 *    assert_fake.cpp
 *
 *  Description:
 *    Fake implementation of the assertion interface to be used in testing
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/interfaces/util_intf.hpp>

#include "CppUTest/TestHarness.h"

namespace mb::assert
{
  bool log_assert_failure( const bool predicate, const bool halt, const char *const file, const int line )
  {
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

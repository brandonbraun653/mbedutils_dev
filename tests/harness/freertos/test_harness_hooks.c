/******************************************************************************
 *  File Name:
 *    test_harness_hooks.c
 *
 *  Description:
 *    FreeRTOS hook functions for the test harness
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <stdint.h>

void vAssertCalled( const char *const pcFileName, unsigned long ulLine )
{
  ( void ) pcFileName;
  ( void ) ulLine;

  while( 1 )
  {
    /* Do nothing */
  }
}

void vApplicationDaemonTaskStartupHook( void )
{
  /* Do nothing */
}

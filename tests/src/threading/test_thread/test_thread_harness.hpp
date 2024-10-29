/******************************************************************************
 *  File Name:
 *    test_thread_harness.hpp
 *
 *  Description:
 *    Harness for testing the thread module in mbedutils
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#include "mbedutils/drivers/threading/message.hpp"
#include <cstdint>
#ifndef MBEDUTILS_TEST_THREAD_HARNESS_HPP
#define MBEDUTILS_TEST_THREAD_HARNESS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <mbedutils/threading.hpp>

#include "CppUTestExt/MockSupport.h"

/*-----------------------------------------------------------------------------
Mock Comparator
-----------------------------------------------------------------------------*/

class TaskConfigComparator : public MockNamedValueComparator
{
public:
  bool isEqual( const void *object1, const void *object2 ) override
  {
    const mb::thread::Task::Config *cfg1 = static_cast<const mb::thread::Task::Config *>( object1 );
    const mb::thread::Task::Config *cfg2 = static_cast<const mb::thread::Task::Config *>( object2 );
    return cfg1->id == cfg2->id && cfg1->name == cfg2->name && cfg1->priority == cfg2->priority &&
           cfg1->msg_queue_inst == cfg2->msg_queue_inst;
  }

  SimpleString valueToString( const void *object ) override
  {
    const mb::thread::Task::Config *cfg = static_cast<const mb::thread::Task::Config *>( object );
    return StringFromFormat( "Task::Config{id=%ld, name=%s, priority=%d, msg_queue=%p}", cfg->id, cfg->name.data(), cfg->priority,
                             cfg->msg_queue_inst );
  }
};

/*-----------------------------------------------------------------------------
Structures
-----------------------------------------------------------------------------*/

struct TestTaskMessage
{
  mb::thread::MessageId id;
  int                   data;
  uint32_t              timestamp;
  uint32_t              a;
  uint8_t               b;

  union
  {
    int   int_value;
    float float_value;
  } union_data;
};

#endif  /* !MBEDUTILS_TEST_THREAD_HARNESS_HPP */

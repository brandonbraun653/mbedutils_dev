/*
 * This file has been auto-generated by CppUMockGen v0.6.
 *
 * Contents will NOT be preserved if it is regenerated!!!
 *
 * Generation options: -s c++20
 */

#include "../../mbedutils/include/mbedutils/interfaces/time_intf.hpp"

#include <CppUTestExt/MockSupport.h>

size_t mb::time::millis()
{
    return static_cast<size_t>(mock().actualCall("mb::time::millis").returnUnsignedLongIntValue());
}

size_t mb::time::micros()
{
    return static_cast<size_t>(mock().actualCall("mb::time::micros").returnUnsignedLongIntValue());
}

void mb::time::delayMilliseconds(const size_t val)
{
    mock().actualCall("mb::time::delayMilliseconds").withUnsignedLongIntParameter("val", val);
}

void mb::time::delayMicroseconds(const size_t val)
{
    mock().actualCall("mb::time::delayMicroseconds").withUnsignedLongIntParameter("val", val);
}

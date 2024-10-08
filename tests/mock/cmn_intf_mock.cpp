/*
 * This file has been auto-generated by CppUMockGen v0.6.
 *
 * Contents will NOT be preserved if it is regenerated!!!
 *
 * Generation options: -s c++20
 */

#include "../../mbedutils/include/mbedutils/interfaces/cmn_intf.hpp"

#include <CppUTestExt/MockSupport.h>

size_t mb::hw::max_drivers(const mb::hw::Driver driver)
{
    return static_cast<size_t>(mock().actualCall("mb::hw::max_drivers").withIntParameter("driver", static_cast<int>(driver)).returnUnsignedLongIntValue());
}

size_t mb::hw::max_driver_index(const mb::hw::Driver driver)
{
    return static_cast<size_t>(mock().actualCall("mb::hw::max_driver_index").withIntParameter("driver", static_cast<int>(driver)).returnUnsignedLongIntValue());
}

bool mb::hw::is_driver_available(const mb::hw::Driver driver, const size_t channel)
{
    return mock().actualCall("mb::hw::is_driver_available").withIntParameter("driver", static_cast<int>(driver)).withUnsignedLongIntParameter("channel", channel).returnBoolValue();
}


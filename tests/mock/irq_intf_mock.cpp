/*
 * This file has been auto-generated by CppUMockGen v0.6.
 *
 * Contents will NOT be preserved if it is regenerated!!!
 *
 * Generation options: -s c++20
 */

#include "../../mbedutils/include/mbedutils/interfaces/irq_intf.hpp"

#include <CppUTestExt/MockSupport.h>

bool mb::irq::in_isr()
{
    return mock().actualCall("mb::irq::in_isr").returnBoolValue();
}

void mb::irq::disable_interrupts()
{
    mock().actualCall("mb::irq::disable_interrupts");
}

void mb::irq::enable_interrupts()
{
    mock().actualCall("mb::irq::enable_interrupts");
}


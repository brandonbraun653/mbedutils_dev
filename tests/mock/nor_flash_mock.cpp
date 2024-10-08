/*
 * This file has been auto-generated by CppUMockGen v0.6.
 *
 * Contents will NOT be preserved if it is regenerated!!!
 *
 * Generation options: -s c++20
 */

#include "../../mbedutils/include/mbedutils/drivers/memory/nvm/nor_flash.hpp"

#include <CppUTestExt/MockSupport.h>

mb::memory::nor::DeviceDriver::DeviceDriver()
{
    mock().actualCall("mb::memory::nor::DeviceDriver::DeviceDriver");
}

mb::memory::nor::DeviceDriver::~DeviceDriver() noexcept
{
    mock().actualCall("mb::memory::nor::DeviceDriver::~DeviceDriver").onObject(this);
}

mb::memory::Status mb::memory::nor::DeviceDriver::write(const size_t block_idx, const size_t offset, const void *const data, const size_t length)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::write").onObject(this).withUnsignedLongIntParameter("block_idx", block_idx).withUnsignedLongIntParameter("offset", offset).withConstPointerParameter("data", data).withUnsignedLongIntParameter("length", length).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::write(const uint64_t address, const void *const data, const size_t length)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::write").onObject(this).withUnsignedLongIntParameter("address", address).withConstPointerParameter("data", data).withUnsignedLongIntParameter("length", length).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::read(const size_t block_idx, const size_t offset, void *const data, const size_t length)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::read").onObject(this).withUnsignedLongIntParameter("block_idx", block_idx).withUnsignedLongIntParameter("offset", offset).withPointerParameter("data", data).withUnsignedLongIntParameter("length", length).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::read(const uint64_t address, void *const data, const size_t length)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::read").onObject(this).withUnsignedLongIntParameter("address", address).withPointerParameter("data", data).withUnsignedLongIntParameter("length", length).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::erase(const uint64_t address, const size_t size)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::erase").onObject(this).withUnsignedLongIntParameter("address", address).withUnsignedLongIntParameter("size", size).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::erase(const size_t block_idx)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::erase").onObject(this).withUnsignedLongIntParameter("block_idx", block_idx).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::erase()
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::erase").onObject(this).returnIntValue());
}

mb::memory::Status mb::memory::nor::DeviceDriver::flush()
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::flush").onObject(this).returnIntValue());
}

void mb::memory::nor::DeviceDriver::open(const mb::memory::nor::DeviceConfig & cfg)
{
    mock().actualCall("mb::memory::nor::DeviceDriver::open").onObject(this).withParameterOfType("mb::memory::nor::DeviceConfig", "cfg", &cfg);
}

void mb::memory::nor::DeviceDriver::close()
{
    mock().actualCall("mb::memory::nor::DeviceDriver::close").onObject(this);
}

mb::memory::Status mb::memory::nor::DeviceDriver::transfer(const void *const cmd, void *const output, const size_t size)
{
    return static_cast<mb::memory::Status>(mock().actualCall("mb::memory::nor::DeviceDriver::transfer").onObject(this).withConstPointerParameter("cmd", cmd).withPointerParameter("output", output).withUnsignedLongIntParameter("size", size).returnIntValue());
}


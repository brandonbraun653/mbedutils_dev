/*
 * This file has been auto-generated by CppUMockGen v0.6.
 *
 * Contents will NOT be preserved if it is regenerated!!!
 *
 * Generation options: -s c++20
 */

#include <CppUMockGen.hpp>

#include "../../mbedutils/include/mbedutils/drivers/memory/nvm/nor_flash.hpp"

#include <CppUTestExt/MockSupport.h>

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& DeviceDriver$ctor();
MockExpectedCall& DeviceDriver$ctor(unsigned int __numCalls__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& DeviceDriver$dtor(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__ = ::CppUMockGen::IgnoreParameter::YES);
MockExpectedCall& DeviceDriver$dtor(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__ = ::CppUMockGen::IgnoreParameter::YES);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& write(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, CppUMockGen::Parameter<const size_t> offset, CppUMockGen::Parameter<const void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
MockExpectedCall& write(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, CppUMockGen::Parameter<const size_t> offset, CppUMockGen::Parameter<const void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& write(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<const void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
MockExpectedCall& write(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<const void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& read(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, CppUMockGen::Parameter<const size_t> offset, CppUMockGen::Parameter<void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
MockExpectedCall& read(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, CppUMockGen::Parameter<const size_t> offset, CppUMockGen::Parameter<void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& read(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
MockExpectedCall& read(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<void *const> data, CppUMockGen::Parameter<const size_t> length, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& erase(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<const size_t> size, mb::memory::Status __return__);
MockExpectedCall& erase(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const uint64_t> address, CppUMockGen::Parameter<const size_t> size, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& erase(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, mb::memory::Status __return__);
MockExpectedCall& erase(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const size_t> block_idx, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& erase(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, mb::memory::Status __return__);
MockExpectedCall& erase(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& flush(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, mb::memory::Status __return__);
MockExpectedCall& flush(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, mb::memory::Status __return__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& open(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const mb::memory::nor::DeviceConfig &> cfg);
MockExpectedCall& open(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const mb::memory::nor::DeviceConfig &> cfg);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& close(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__);
MockExpectedCall& close(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__);
} } } } }

namespace expect { namespace mb$ { namespace memory$ { namespace nor$ { namespace DeviceDriver$ {
MockExpectedCall& transfer(CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const void *const> cmd, CppUMockGen::Parameter<void *const> output, CppUMockGen::Parameter<const size_t> size, mb::memory::Status __return__);
MockExpectedCall& transfer(unsigned int __numCalls__, CppUMockGen::Parameter<const mb::memory::nor::DeviceDriver*> __object__, CppUMockGen::Parameter<const void *const> cmd, CppUMockGen::Parameter<void *const> output, CppUMockGen::Parameter<const size_t> size, mb::memory::Status __return__);
} } } } }


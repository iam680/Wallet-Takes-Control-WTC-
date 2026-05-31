#include "runtime_context.hpp"

namespace runtime
{
    uint64_t baseAddr = 0;
    uint64_t baseSize = 0;

    auto set_base_info(uint64_t address, uint64_t size) -> void
    {
        baseAddr = address;
        baseSize = size;
    }

    auto base_address() -> uint64_t
    {
        return baseAddr;
    }

    auto base_size() -> uint64_t
    {
        return baseSize;
    }
}

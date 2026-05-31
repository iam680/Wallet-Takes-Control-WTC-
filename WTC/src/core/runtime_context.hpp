#pragma once

#include <cstdint>

namespace runtime
{
    // Backward compatibility for legacy code paths still referencing old globals.
    extern uint64_t baseAddr;
    extern uint64_t baseSize;

    auto set_base_info(uint64_t address, uint64_t size) -> void;
    auto base_address() -> uint64_t;
    auto base_size() -> uint64_t;
}

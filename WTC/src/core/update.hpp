#pragma once
#include "runtime_context.hpp"

namespace update
{
    /*
        ViewMatrix
        4C 8D 35 ? ? ? ? 48 8B 86 ? ? ? ?
        CGame
        48 8B 05 ? ? ? ? F2 0F 10 4F 08
        LocalPlayer
        48 39 0D ? ? ? ? 75 0B
        Yaw
        48 8b 05 ? ? ? ? 48 89 84 24 ? ? ? ? 8b 84 24 ? ? ? ? 34
    */

    inline auto run() -> bool
    {
        const auto c_game = TargetProcess->FindSignature(
            "48 8B 05 ? ? ? ? F2 0F 10 4F",
            runtime::base_address(),
            runtime::base_address() + runtime::base_size()
        );

        if (c_game < 0x100000)
        {
            std::cout << "Failed to find c_game signature." << std::endl;
            return false;
        }

        {
            const int32_t disp = TargetProcess->Read<int32_t>(c_game + 3);
            const uint64_t c_game_ptr = c_game + 7 + disp;
            offsets::set_runtime_offset("cgame_offset", c_game_ptr - runtime::base_address());
        }

        const auto c_local = TargetProcess->FindSignature(
            "48 39 0D ? ? ? ? 75 ? 48 C7 05",
            runtime::base_address(),
            runtime::base_address() + runtime::base_size()
        );

        if (c_local < 0x100000)
        {
            std::cout << "Failed to find c_local signature." << std::endl;
            return false;
        }

        {
            const int32_t disp = TargetProcess->Read<int32_t>(c_local + 3);
            const uint64_t c_local_ptr = c_local + 7 + disp;
            offsets::set_runtime_offset("localplayer_offset", c_local_ptr - runtime::base_address());
        }

        return true;
    }
}

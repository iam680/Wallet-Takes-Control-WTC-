#pragma once
#include <cstdint>
#include "..\..\core\runtime_context.hpp"

class c_entity {
public:
    auto init() -> bool {
        this->base_address =
            TargetProcess->Read<std::uintptr_t>(runtime::base_address() + offsets::localplayer_offset);
        return this->base_address != 0;
    }

    // 다른 엔티티용으로 쓸 수 있게 기본 생성자 추가
    c_entity(std::uintptr_t base = 0)
        : base_address(base) {
    }

public:
    auto getGuiState() -> uint8_t {
        return TargetProcess->Read<uint8_t>(
            this->base_address + offsets::localplayer::guiState_offset
        );
    }

    auto getLocalUnit() -> c_unit {
        std::uintptr_t addr =
            TargetProcess->Read<std::uintptr_t>(
                this->base_address + offsets::localplayer::localunit_offset
            );
        return c_unit(addr - 1);
    }

    // ★ 이 엔티티에 닉네임이 있는지 여부만 확인 (봇 필터용)
    auto hasNickname() const -> bool {
        if (!this->base_address)
            return false;

        // Entity::nikename = 0xB8 을 쓰는 것으로 가정
        char first_char =
            TargetProcess->Read<char>(this->base_address + offsets::localplayer::nikename);

        // 첫 글자가 널이면 "닉네임 없음" 으로 간주
        return first_char != '\0';
    }

    auto get_base() const -> std::uintptr_t {
        return base_address;
    }

private:
    std::uintptr_t base_address{};
};

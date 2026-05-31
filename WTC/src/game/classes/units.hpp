#pragma once

class c_movement_ground;

// === НОВОЕ ДОБАВЛЕНИЕ: Тип матрицы вращения ===
struct rotation_matrix_t
{
    // Предполагается, что в памяти лежит порядок [right, forward, up]
    vec3_t right;    // local X (влево/вправо)
    vec3_t forward;  // local Y (вперёд/назад)
    vec3_t up;       // local Z (вверх/вниз)

    inline vec3_t transform(const vec3_t& v) const
    {
        // local: x = right, y = forward, z = up
        // world = right * x + forward * y + up * z
        return vec3_t{
            right.x * v.x + forward.x * v.y + up.x * v.z,
            right.y * v.x + forward.y * v.y + up.y * v.z,
            right.z * v.x + forward.z * v.y + up.z * v.z
        };
    }
};

class c_unit {
public:
    auto get_base() -> uintptr_t {
        return this->base_address;
    }

    c_unit(const std::uintptr_t base_address = 0) : base_address(base_address) {}

public:
    // === Bounding Box (габариты объекта) ===

    // Максимальная точка хитбокса (верхний правый дальний угол)
    // Возможное название в offsets: bbmax_offset
    auto getBBMax() -> vec3_t {
        return TargetProcess->Read<vec3_t>(this->base_address + offsets::unit_offsets::bbmax_offset);
    }

    // Минимальная точка хитбокса (нижний левый ближний угол)
    // Возможное название в offsets: bbmin_offset
    auto getBBMin() -> vec3_t {
        return TargetProcess->Read<vec3_t>(this->base_address + offsets::unit_offsets::bbmin_offset);
    }

    // === Ориентация и движение ===

    // Матрица вращения объекта (направление взгляда/корпуса)
    // Возможное название в offsets: rotation_matrix_offset
    auto getRotationMatrix() -> rotation_matrix_t {
        return TargetProcess->Read<rotation_matrix_t>(this->base_address + offsets::unit_offsets::rotation_matrix_offset);
    }

    // Указатель на компонент наземного движения
    // Возможное название в offsets: groundmovement_offset
    auto get_movement_ground() -> c_movement_ground {
        uintptr_t addr = TargetProcess->Read<uintptr_t>(this->base_address + offsets::unit_offsets::groundmovement_offset);
        return c_movement_ground(addr);
    }

    // Указатель на компонент воздушного движения
    // Возможное название в offsets: airmovement_offset
    auto get_movement_air() -> c_movement_air {
        uintptr_t addr = TargetProcess->Read<uintptr_t>(this->base_address + offsets::unit_offsets::airmovement_offset);
        return c_movement_air(addr);
    }

    // Позиция объекта в мире (X, Y, Z)
    // Возможное название в offsets: position_offset
    auto getPosition() -> vec3_t {
        return TargetProcess->Read<vec3_t>(this->base_address + offsets::unit_offsets::position_offset);
    }

    // === Информация и состояние ===

    // Указатель на структуру с доп. инфо (ник, тип юнита, название техники)
    // Возможное название в offsets: info_offset
    auto getInfo() -> c_info {
        uintptr_t addr = TargetProcess->Read<uintptr_t>(this->base_address + offsets::unit_offsets::info_offset);
        return c_info(addr);
    }

    // Флаг/таймер неуязвимости (респавн-защита, миссия)
    // Возможное название в offsets: invulnerable_offset
    auto getInvulnerable() -> uintptr_t {
        return TargetProcess->Read<uintptr_t>(this->base_address + offsets::unit_offsets::invulnerable_offset);
    }

    // Состояние юнита (0 = alive, 1 = dead, 2 = spectator и т.д.)
    // Возможное название в offsets: unitState_offset
    auto getUnitState() -> uint16_t {
        return TargetProcess->Read<uint16_t>(this->base_address + offsets::unit_offsets::unitState_offset);
    }

    // Номер команды/армии (для фильтрации "свой/чужой")
    // Возможное название в offsets: unitArmyNo_offset
    auto getTeam() -> uint8_t {
        return TargetProcess->Read<uint8_t>(this->base_address + offsets::unit_offsets::unitArmyNo_offset);
    }

    // Прогресс перезарядки (визуальный или логический)
    // Возможное название в offsets: visualReloadProgress_offset
    auto getReloadTime() -> uint8_t {
        return TargetProcess->Read<uint8_t>(this->base_address + offsets::unit_offsets::visualReloadProgress_offset);
    }

private:
    uintptr_t base_address;

};
#pragma once

#include <array>
#include <cmath>
#include <cstdint>

class c_ballistic {
public:
	//auto init( uintptr_t cGame ) -> bool {
	//	this->base_address = TargetProcess->Read< uintptr_t >( cGame + offsets::cgame_offsets::ballistics_offset );
	//	return this->base_address != 0;
	//}
	auto init(uintptr_t cGame) -> bool
	{
		uintptr_t addr = cGame + offsets::cgame_offsets::ballistics_offset;
		this->base_address = TargetProcess->Read<uintptr_t>(addr);
		return (this->base_address != 0);
	}
public:

	auto getVelocity( ) -> float {
		return TargetProcess->Read< float >( this->base_address + offsets::cgame_offsets::ballistic_offsets::velocity );
	}

	auto getMass( ) -> float {
		return TargetProcess->Read< float >( this->base_address + offsets::cgame_offsets::ballistic_offsets::mass );
	}

	auto getCaliber( ) -> float {
		return TargetProcess->Read< float >( this->base_address + offsets::cgame_offsets::ballistic_offsets::caliber );
	}
	
	auto getLength( ) -> float {
		return TargetProcess->Read< float >( this->base_address + offsets::cgame_offsets::ballistic_offsets::length );
	}

	auto getImpactPoint( ) -> vec3_t {
		return readImpactPointWithFallback(offsets::cgame_offsets::ballistic_offsets::bullet_impact_point);
	}

	auto getMaxDistance( ) -> float {
		return TargetProcess->Read< float >( this->base_address + offsets::cgame_offsets::ballistic_offsets::max_dist );
	}

	auto getBombImpactPoint() -> vec3_t {
		return readImpactPointWithFallback(offsets::cgame_offsets::ballistic_offsets::bomb_impact_point);
	}

	auto getCCRPProjectileCode() const -> uint16_t
	{
		const uintptr_t module_base = static_cast<uintptr_t>(TargetProcess->GetBaseAddress("aces.exe"));
		if (!module_base)
			return 0;

		const uintptr_t switch_base = TargetProcess->Read<uintptr_t>(module_base + offsets::ccrp_switch_offset);
		if (!switch_base)
			return 0;

		return TargetProcess->Read<uint16_t>(switch_base + 0x41B0);
	}

private:
	static auto is_valid_impact_point(const vec3_t& point) -> bool
	{
		if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
			return false;

		if (point.empty())
			return false;

		constexpr float kMaxReasonableAbsCoord = 500000.0f;
		return std::fabs(point.x) <= kMaxReasonableAbsCoord &&
			std::fabs(point.y) <= kMaxReasonableAbsCoord &&
			std::fabs(point.z) <= kMaxReasonableAbsCoord;
	}

	auto readImpactPointWithFallback(uintptr_t primary_offset) const -> vec3_t
	{
		// New layout uses bomb_prediction_offset as primary source.
		// Depending on build, value can be stored directly, at +0x20, or via pointer.
		const std::array<uintptr_t, 2> direct_candidates{
			primary_offset,
			primary_offset + 0x20
		};

		for (const uintptr_t offset : direct_candidates)
		{
			const vec3_t point = TargetProcess->Read< vec3_t >(this->base_address + offset);
			if (is_valid_impact_point(point))
				return point;
		}

		const uintptr_t impact_ptr = TargetProcess->Read<uintptr_t>(this->base_address + primary_offset);
		if (impact_ptr > 0x10000)
		{
			const std::array<uintptr_t, 2> ptr_candidates{ impact_ptr, impact_ptr + 0x20 };
			for (const uintptr_t addr : ptr_candidates)
			{
				const vec3_t point = TargetProcess->Read< vec3_t >(addr);
				if (is_valid_impact_point(point))
					return point;
			}
		}

		// Legacy fallback for older dumps (WTC-orig layout).
		const std::array<uintptr_t, 2> legacy_candidates{
			static_cast<uintptr_t>(0x1AEC + 0x20),
			static_cast<uintptr_t>(0x22C8 + 0x20)
		};
		for (const uintptr_t offset : legacy_candidates)
		{
			const vec3_t point = TargetProcess->Read< vec3_t >(this->base_address + offset);
			if (is_valid_impact_point(point))
				return point;
		}

		return {};
	}

private:
	uintptr_t base_address;
};

#pragma once
#include "..\\..\\core\\runtime_context.hpp"
#include <shared_mutex>
#include <atomic>
#include <array>
#include <cmath>
#include <cfloat>
#include <memory>

namespace misc
{
	inline std::vector< c_unit > unitsList;
	inline std::shared_mutex units_mutex;
	enum class projectile_type_t : uint8_t
	{
		bomb = 0,
		rocket = 1
	};

	struct projectile_point_t
	{
		vec3_t position{};
		projectile_type_t type = projectile_type_t::rocket;
	};

	inline std::vector< projectile_point_t > projectile_points;
	inline std::shared_mutex projectile_points_mutex;
	struct frame_state_t
	{
		ViewMatrix_t camera_matrix{};
		vec3_t camera_pos{};
		vec3_t local_pos{};
		vec3_t bomb_impact{};
		GuiState gui_state = GuiState::NONE;
		uint16_t ccrp_projectile_code = 0;
		bool local_is_plane = false;
		bool local_bomb_selected = false;
		bool valid = false;
	};

	struct frame_snapshot_t
	{
		uint64_t frame_id = 0;
		std::chrono::steady_clock::time_point captured_at{};
		frame_state_t frame{};
		std::unordered_map< uintptr_t, vec3_t > positions;
	};
	inline constexpr size_t frame_snapshot_slots_count = 3;
	inline std::array< std::atomic< std::shared_ptr< const frame_snapshot_t > >, frame_snapshot_slots_count > frame_snapshot_slots{};
	inline std::atomic<uint8_t> frame_snapshot_front_slot{ 0 };
	inline std::atomic<uint64_t> frame_snapshot_publish_seq{ 0 };
	inline std::atomic<uint64_t> frame_sequence{ 0 };
	inline std::atomic<uint64_t> prof_update_entity_us{ 0 };
	inline std::atomic<uint64_t> prof_update_fast_us{ 0 };
	inline std::atomic<uint64_t> prof_update_frame_us{ 0 };
	inline std::atomic<uint64_t> prof_esp_us{ 0 };
	inline std::atomic<uint64_t> prof_aimbot_us{ 0 };
	inline std::atomic<uint64_t> prof_main_frame_us{ 0 };
	inline std::atomic<uint64_t> prof_snapshot_age_us{ 0 };
	inline std::atomic<uint64_t> prof_esp_w2s_calls{ 0 };
	inline std::atomic<uint64_t> prof_esp_obb_calls{ 0 };
	inline std::atomic<uint64_t> prof_update_entity_count{ 0 };
	inline std::atomic<uint64_t> prof_update_fast_count{ 0 };
	inline std::atomic<uint64_t> prof_update_frame_count{ 0 };
	inline std::atomic<uint64_t> prof_esp_count{ 0 };
	inline std::atomic<uint64_t> prof_aimbot_calls{ 0 };
	inline std::atomic<uint64_t> prof_main_frame_count{ 0 };

	struct scoped_profile_t
	{
		std::atomic<uint64_t>& output_us;
		std::atomic<uint64_t>* counter;
		std::chrono::steady_clock::time_point start;

		scoped_profile_t(std::atomic<uint64_t>& output, std::atomic<uint64_t>* count = nullptr)
			: output_us(output), counter(count), start(std::chrono::steady_clock::now())
		{
		}

		~scoped_profile_t()
		{
			const auto end = std::chrono::steady_clock::now();
			const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			output_us.store(static_cast<uint64_t>(us), std::memory_order_relaxed);
			if (counter)
				counter->fetch_add(1, std::memory_order_relaxed);
		}
	};

	inline auto snapshot_units() -> std::vector<c_unit>
	{
		std::shared_lock lock(units_mutex);
		return unitsList;
	}

	inline auto snapshot_units_into(std::vector<c_unit>& out) -> void
	{
		std::shared_lock lock(units_mutex);
		out = unitsList;
	}

	inline auto units_count() -> size_t
	{
		std::shared_lock lock(units_mutex);
		return unitsList.size();
	}

	inline auto snapshot_projectiles( ) -> std::vector< projectile_point_t >
	{
		std::shared_lock lock( projectile_points_mutex );
		return projectile_points;
	}

	inline auto snapshot_projectiles_into(std::vector< projectile_point_t >& out) -> void
	{
		std::shared_lock lock( projectile_points_mutex );
		out = projectile_points;
	}

	inline auto snapshot_frame( ) -> std::shared_ptr< const frame_snapshot_t >
	{
		const uint8_t slot = frame_snapshot_front_slot.load( std::memory_order_acquire );
		return frame_snapshot_slots[slot].load( std::memory_order_acquire );
	}

	inline auto reset_runtime_cache( ) -> void
	{
		{
			std::unique_lock units_lock( units_mutex );
			unitsList.clear( );
		}
		{
			std::unique_lock projectile_lock( projectile_points_mutex );
			projectile_points.clear( );
		}
		for ( auto& slot : frame_snapshot_slots )
			slot.store( std::shared_ptr< const frame_snapshot_t >{}, std::memory_order_release );
		frame_snapshot_front_slot.store( 0, std::memory_order_release );
		frame_snapshot_publish_seq.store( 0, std::memory_order_release );
	}

	inline auto UpdateProjectileList( ) -> void
	{
		std::unique_lock lock( projectile_points_mutex );
		projectile_points.clear( );
	}

	inline auto publish_frame_snapshot( frame_snapshot_t&& snapshot ) -> void
	{
		auto next = std::make_shared< frame_snapshot_t >( std::move( snapshot ) );
		const uint64_t seq = frame_snapshot_publish_seq.fetch_add( 1, std::memory_order_acq_rel ) + 1;
		const uint8_t back_slot = static_cast<uint8_t>( seq % frame_snapshot_slots_count );

		frame_snapshot_slots[back_slot].store( std::shared_ptr< const frame_snapshot_t >( std::move( next ) ), std::memory_order_release );
		frame_snapshot_front_slot.store( back_slot, std::memory_order_release );
	}

	inline auto UpdateFrameSnapshot() -> void
	{
		scoped_profile_t prof(prof_update_frame_us, &prof_update_frame_count);
		frame_snapshot_t next{};
		next.captured_at = std::chrono::steady_clock::now();
		next.frame_id = frame_sequence.fetch_add( 1, std::memory_order_relaxed ) + 1;

		const auto gui_state = sdk::cLocalPlayer->getGuiState();
		next.frame.gui_state = static_cast<GuiState>( gui_state );
		if (gui_state != GuiState::ALIVE && gui_state != GuiState::SPEC)
		{
			reset_runtime_cache( );
			publish_frame_snapshot( std::move( next ) );
			return;
		}

		next.frame.camera_matrix = sdk::cGame->camera->getCameraMatrix();
		next.frame.camera_pos = sdk::cGame->camera->getPosition();
		c_unit local = sdk::cLocalPlayer->getLocalUnit();
		next.frame.local_pos = local.getPosition();

		const uintptr_t local_base = local.get_base();
		if (local_base)
		{
			next.frame.local_is_plane = local.getInfo().isPlane();
			if (next.frame.local_is_plane)
			{
				next.frame.ccrp_projectile_code = sdk::cGame->ballistics->getCCRPProjectileCode();
				next.frame.local_bomb_selected = (next.frame.ccrp_projectile_code == 0x439 || next.frame.ccrp_projectile_code == 0);

				next.frame.bomb_impact = sdk::cGame->ballistics->getBombImpactPoint();
				if (next.frame.bomb_impact.empty() && next.frame.local_bomb_selected)
					next.frame.bomb_impact = sdk::cGame->ballistics->getImpactPoint();
			}
		}

		auto units = snapshot_units( );
		next.positions.reserve( units.size( ) );

		VMMDLL_SCATTER_HANDLE hScatter = TargetProcess->CreateScatterHandle( );
		if ( hScatter && !units.empty( ) )
		{
			std::vector< uintptr_t > bases;
			bases.reserve( units.size( ) );

			for ( c_unit& unit : units )
			{
				const uintptr_t base = unit.get_base( );
				if ( !base )
					continue;

				bases.emplace_back( base );
			}

			std::vector< vec3_t > positions( bases.size( ) );
			for ( size_t i = 0; i < bases.size( ); ++i )
			{
				TargetProcess->AddScatterReadRequest(
					hScatter,
					bases[ i ] + offsets::unit_offsets::position_offset,
					&positions[ i ],
					sizeof( vec3_t )
				);
			}

			if ( !bases.empty( ) && TargetProcess->ExecuteReadScatter( hScatter, 0, true ) )
			{
				for ( size_t i = 0; i < bases.size( ); ++i )
				{
					if ( positions[ i ].empty( ) )
						continue;

					next.positions[ bases[ i ] ] = positions[ i ];
				}
			}
			else
			{
				for ( c_unit& unit : units )
				{
					const uintptr_t base = unit.get_base( );
					if ( !base )
						continue;

					vec3_t position = unit.getPosition( );
					if ( position.empty( ) )
						continue;

					next.positions[ base ] = position;
				}
			}

			TargetProcess->CloseScatterHandle( hScatter );
		}
		else
		{
			for ( c_unit& unit : units )
			{
				const uintptr_t base = unit.get_base( );
				if ( !base )
					continue;

				vec3_t position = unit.getPosition( );
				if ( position.empty( ) )
					continue;

				next.positions[ base ] = position;
			}
		}

		next.frame.valid = true;
		publish_frame_snapshot( std::move( next ) );
	}

	inline bool has_player_nickname(c_unit& unit)
	{
		using clock = std::chrono::steady_clock;
		struct nickname_cache_entry_t
		{
			bool has_name = false;
			clock::time_point next_refresh{};
			clock::time_point last_seen{};
		};

		static std::unordered_map<uintptr_t, nickname_cache_entry_t> cache;
		static auto next_gc = clock::now() + std::chrono::seconds(5);

		const uintptr_t unit_base = unit.get_base();
		if (!unit_base)
			return false;

		const auto now = clock::now();
		auto& entry = cache[unit_base];
		if (now >= entry.next_refresh)
		{
			// player_ptr path is unstable, so keep vehicle-name heuristic,
			// but refresh it as "slow" metadata instead of every entity pass.
			const auto name = unit.getInfo().getVehicleName();
			entry.has_name = !name.empty() && name.length() < 64;
			entry.next_refresh = now + std::chrono::milliseconds(1500);
		}
		entry.last_seen = now;

		if (now >= next_gc)
		{
			for (auto it = cache.begin(); it != cache.end(); )
			{
				if (now - it->second.last_seen > std::chrono::seconds(15))
					it = cache.erase(it);
				else
					++it;
			}
			next_gc = now + std::chrono::seconds(5);
		}

		return entry.has_name;
	}

	inline auto is_valid_enemy(c_unit& unit, uint8_t local_team) -> bool
	{
		if ( unit.getUnitState( ) >= 1 ) 
			return false;
		
		const uint8_t unit_team = unit.getTeam( );
		if (unit_team == 0 || unit_team == local_team)
			return false;

		if (!has_player_nickname(unit))
			return false;
		
		return true;
	}

	inline auto UpdateEntityList( ) -> void 
	{
		scoped_profile_t prof(prof_update_entity_us, &prof_update_entity_count);
		std::vector< c_unit > temp_units;

		const auto gui_state = sdk::cLocalPlayer->getGuiState();
		if (gui_state != GuiState::ALIVE && gui_state != GuiState::SPEC) {
			sdk::cLocalPlayer->init( );
			reset_runtime_cache( );
			return;
		}

		VMMDLL_SCATTER_HANDLE hScatter = TargetProcess->CreateScatterHandle( );
		if ( !hScatter )
			return;

		const int unit_count = sdk::cGame->getUnitCount( );
		if (unit_count <= 0)
		{
			TargetProcess->CloseScatterHandle( hScatter );
			return;
		}

		temp_units.reserve(static_cast<size_t>(unit_count));

		const uintptr_t unit_list_base = sdk::cGame->getUnitList( );
		auto scatter_unit = [ & ]( VMMDLL_SCATTER_HANDLE handle, uint32_t count ) -> std::vector< c_unit >
		{
			std::vector< std::uintptr_t > pointers( count );
			std::vector< c_unit > result;
			result.reserve( count );

			for (size_t i = 0; i < count; i++)
				TargetProcess->AddScatterReadRequest( handle, unit_list_base + 0x8 * i, &pointers[ i ], sizeof( std::uintptr_t ) );
			
			if ( !TargetProcess->ExecuteReadScatter( handle ) )
			{
				TargetProcess->CloseScatterHandle( handle );
				return result;
			}

			for ( size_t i = 0; i < count; i++ )
				result.emplace_back( c_unit( pointers.at( i ) ) );

			TargetProcess->CloseScatterHandle( handle );
			return result;
		};

		std::vector< c_unit > units = scatter_unit( hScatter, unit_count );
		const uint8_t local_team = sdk::cLocalPlayer->getLocalUnit().getTeam();
		std::vector< uint16_t > unit_states( units.size( ) );
		std::vector< uint8_t > unit_teams( units.size( ) );
		bool has_basic_scatter_data = false;

		VMMDLL_SCATTER_HANDLE hBasicScatter = TargetProcess->CreateScatterHandle( );
		if ( hBasicScatter )
		{
			for ( size_t i = 0; i < units.size( ); ++i )
			{
				const uintptr_t base = units[ i ].get_base( );
				if ( !base )
					continue;

				TargetProcess->AddScatterReadRequest(
					hBasicScatter,
					base + offsets::unit_offsets::unitState_offset,
					&unit_states[ i ],
					sizeof( uint16_t )
				);

				TargetProcess->AddScatterReadRequest(
					hBasicScatter,
					base + offsets::unit_offsets::unitArmyNo_offset,
					&unit_teams[ i ],
					sizeof( uint8_t )
				);
			}

			has_basic_scatter_data = TargetProcess->ExecuteReadScatter( hBasicScatter, 0, true );
			TargetProcess->CloseScatterHandle( hBasicScatter );
		}
		
		//static bool s_debug_done = false;
		//if (!s_debug_done && !units.empty()) {
		//	auto local = sdk::cLocalPlayer->getLocalUnit();

		//	LOG("\n[=== DEBUG UNIT READS ===]\n");

			// Локальный игрок
		//	auto lpos = local.getPosition();
		//	auto lteam = local.getTeam();
		//	auto lstate = local.getUnitState();
		//	LOG("[LOCAL] pos=(%.1f,%.1f,%.1f) team=%u state=%u base=0x%zx\n",
		//		lpos.x, lpos.y, lpos.z, lteam, lstate, local.get_base());

		//	// Первые 2 юнита — используем существующий vector 'units'
		//	size_t debug_count = units.size() < 2 ? units.size() : 2;
		//	for (size_t i = 0; i < debug_count; i++) {
		//		auto& u = units[i];  // ← берём из существующего vector, не объявляем новый
		//		auto upos = u.getPosition();
		//		auto uteam = u.getTeam();
		//		auto ustate = u.getUnitState();
		//		auto uinfo = u.getInfo();
		//		auto uname = uinfo.getVehicleName();

			//	constexpr uintptr_t player_ptr_off = 0x0D78;
			//	uintptr_t pptr = TargetProcess->Read<uintptr_t>(u.get_base() + player_ptr_off);
			//	char nick0 = pptr ? TargetProcess->Read<char>(pptr + offsets::localplayer::nikename) : 0;

		//		LOG("[UNIT#%zu] pos=(%.1f,%.1f,%.1f) team=%u state=%u name='%s' pptr=0x%zx nick[0]='%c'\n",
		//			i, upos.x, upos.y, upos.z, uteam, ustate,
		//			uname.empty() ? "???" : uname.c_str(), pptr,
		//			(nick0 >= 32 && nick0 < 127) ? nick0 : '?');
		//	}
		//	LOG("[=====================]\n\n");

		//	s_debug_done = true;
//		}

		// === ФИЛЬТР (временно отключён) ===
		// Просто закомментируй if, не трогай цикл
		for ( size_t i = 0; i < units.size( ); ++i )
		{
			c_unit& unit = units[ i ];
			const uint16_t unit_state = has_basic_scatter_data ? unit_states[ i ] : unit.getUnitState( );
			if ( unit_state >= 1 )
				continue;

			const uint8_t unit_team = has_basic_scatter_data ? unit_teams[ i ] : unit.getTeam( );
			if ( unit_team == 0 || unit_team == local_team )
				continue;

			if ( !has_player_nickname( unit ) )
				continue;

			temp_units.emplace_back( unit );

		}

		{
			std::unique_lock lock(units_mutex);
			unitsList = std::move(temp_units);
		}

	}

}

#pragma once

#include "offsets.hpp"
#include "datatypes\vector2.hpp"
#include "datatypes\vector3.hpp"
#include "datatypes\matrix.hpp"

#define OFFSETS_WITH_BA(type, offset) \
    TargetProcess->Read< type >(this->base_address + (offset));

#define OFFSETS(type, offset) \
    TargetProcess->Read< type >((offset));

// classes
#include "classes\ballistic.hpp"
#include "classes\camera.hpp"
#include "classes\game.hpp"
#include "classes\movement.hpp"
#include "classes\info.hpp"
#include "classes\units.hpp"
#include "classes\entity.hpp"

enum GuiState : std::uint8_t {
	NONE = 0,
	MENU = 1,
	ALIVE = 2,
	DEAD = 3,
	SPEC = 6,
	SPAWN_MENU = 8,
	LOADING = 10
};

namespace sdk
{
	inline int screen_width = GetSystemMetrics( SM_CXSCREEN );
	inline int screen_height = GetSystemMetrics( SM_CYSCREEN );
	inline int monitor_count = 1;
	inline int monitor_index = 0;
	inline bool auto_fullscreen_on_start = false;
	inline bool request_monitor_apply = false;
	inline bool request_manual_reload = false;
	inline std::string manual_reload_status = "";
	inline std::vector<std::string> monitor_labels = { "Display 1" };

	inline bool  esp_enabled = true;
	inline bool  aimbot_enabled = false;
	inline float esp_render_distance = 2000.0f;
	inline float esp_box_distance = 2000.0f;
	inline bool  esp_show_boxes = true;
	inline bool  esp_use_3d_boxes = true;
	inline bool  esp_show_vehicle_names = true;
	inline bool  esp_show_distance = true;
	inline bool  esp_show_reload = false;
	inline bool  esp_show_reload_seconds = false;
	inline bool  esp_show_fps = true;
	inline bool  esp_vsync_enabled = false;
	inline int   esp_vsync_fps = 120;
	inline bool  esp_show_reload_dash = false;
	inline bool  esp_show_bomb_impact = true;

	inline bool show_helicopter = true;
	inline bool show_bomber = true;
	inline bool show_fighter = true;
	inline bool show_attacker = true;
	inline bool show_light_tank = true;
	inline bool show_heavy_tank = true;
	inline bool show_medium_tank = true;
	inline bool show_spaa = true;
	inline bool show_gun_boat = true;
	inline bool show_destroyer = true;
	inline bool show_cruiser = true;
	inline bool show_torpedo_boat = true;

	inline ImVec4 color_helicopter = ImVec4( 247.f / 255.f, 163.f / 255.f, 159.f / 255.f, 1.f );
	inline ImVec4 color_bomber = ImVec4( 155.f / 255.f, 165.f / 255.f, 237.f / 255.f, 1.f );
	inline ImVec4 color_fighter = ImVec4( 241.f / 255.f, 158.f / 255.f, 112.f / 255.f, 1.f );
	inline ImVec4 color_attacker = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
	inline ImVec4 color_light_tank = ImVec4( 246.f / 255.f, 234.f / 255.f, 237.f / 255.f, 1.f );
	inline ImVec4 color_heavy_tank = ImVec4( 246.f / 255.f, 95.f / 255.f, 99.f / 255.f, 1.f );
	inline ImVec4 color_medium_tank = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
	inline ImVec4 color_spaa = ImVec4( 184.f / 255.f, 146.f / 255.f, 238.f / 255.f, 1.f );
	inline ImVec4 color_gun_boat = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
	inline ImVec4 color_destroyer = ImVec4( 246.f / 255.f, 95.f / 255.f, 99.f / 255.f, 1.f );
	inline ImVec4 color_cruiser = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
	inline ImVec4 color_torpedo_boat = ImVec4( 184.f / 255.f, 146.f / 255.f, 238.f / 255.f, 1.f );

	inline c_game* cGame = new c_game;
	inline c_entity* cLocalPlayer = new c_entity;

	inline auto init() -> bool 
	{
		if (!cGame->init())
			return false;

		if (!cLocalPlayer->init())
			return false;

		if (!cGame->ballistics->init(cGame->get_base()))
			return false;

		if (!cGame->camera->init(cGame->get_base()))
			return false;

		return true;
	}

}

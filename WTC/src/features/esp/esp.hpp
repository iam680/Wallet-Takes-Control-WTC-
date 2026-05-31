#pragma once

#define NOMINMAX
#include <Windows.h>

#include <array>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>
#include <string_view>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <cstdio>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

// Выберите один из режимов:
#define ROW_MAJOR   1   // точка * матрица (стандарт)
// #define COL_MAJOR   1   // матрица * точка (транспонированная)

struct vec4_t {
    float x, y, z, w;
};

namespace esp
{
    inline void draw_impact_x(const vec2_t& screen_pos, ImU32 color, float size = 8.0f, float thickness = 1.5f)
    {
        g_render->line(screen_pos.x - size, screen_pos.y - size, screen_pos.x + size, screen_pos.y + size, color, thickness);
        g_render->line(screen_pos.x + size, screen_pos.y - size, screen_pos.x - size, screen_pos.y + size, color, thickness);
    }

    struct projection_context_t
    {
        vec3_t camera_pos{};
        double bias_clip_x = 0.0;
        double bias_clip_y = 0.0;
        double bias_clip_w = 0.0;
        bool valid = false;
    };

    struct unit_cached_info_t
    {
        std::string unit_type;
        std::string vehicle_name;
        std::chrono::steady_clock::time_point next_refresh{};
        std::chrono::steady_clock::time_point last_seen{};
    };

    struct unit_type_info_t
    {
        ImU32       color;
        bool        worth_to_draw;
        bool        is_dummy;
        bool        is_plane;
        bool        show_reload;
        bool        draw_box;
    };

    struct unit_box_cache_t
    {
        vec3_t bbmin{};
        vec3_t bbmax{};
        rotation_matrix_t rot{};
        std::chrono::steady_clock::time_point next_bounds_refresh{};
        std::chrono::steady_clock::time_point next_rot_refresh{};
        std::chrono::steady_clock::time_point last_seen{};
    };

    struct model_bounds_cache_t
    {
        vec3_t bbmin{};
        vec3_t bbmax{};
        std::chrono::steady_clock::time_point next_refresh{};
        std::chrono::steady_clock::time_point last_seen{};
    };

    struct unit_reload_cache_t
    {
        uint8_t value = 0;
        std::chrono::steady_clock::time_point next_refresh{};
        std::chrono::steady_clock::time_point last_seen{};
    };

    inline unit_type_info_t get_unit_type_info(std::string_view unit_type)
    {
        if (unit_type == "exp_helicopter")       return { ImGui::ColorConvertFloat4ToU32(sdk::color_helicopter), sdk::show_helicopter,  false, false, true,  false };
        if (unit_type == "exp_bomber")           return { ImGui::ColorConvertFloat4ToU32(sdk::color_bomber), sdk::show_bomber,  false, true,  false, false };
        if (unit_type == "exp_fighter")          return { ImGui::ColorConvertFloat4ToU32(sdk::color_fighter), sdk::show_fighter,  false, true,  false, false };
        if (unit_type == "exp_assault")          return { ImGui::ColorConvertFloat4ToU32(sdk::color_attacker), sdk::show_attacker,  false, true,  false, false };
        if (unit_type == "exp_tank")             return { ImGui::ColorConvertFloat4ToU32(sdk::color_light_tank), sdk::show_light_tank,  false, false, true,  true };
        if (unit_type == "exp_heavy_tank")       return { ImGui::ColorConvertFloat4ToU32(sdk::color_heavy_tank), sdk::show_heavy_tank,  false, false, true,  true };
        if (unit_type == "exp_tank_destroyer")   return { ImGui::ColorConvertFloat4ToU32(sdk::color_medium_tank), sdk::show_medium_tank,  false, false, true,  true };
        if (unit_type == "exp_SPAA")             return { ImGui::ColorConvertFloat4ToU32(sdk::color_spaa), sdk::show_spaa,  false, false, true,  true };
        if (unit_type == "exp_gun_boat")         return { ImGui::ColorConvertFloat4ToU32(sdk::color_gun_boat), sdk::show_gun_boat,  false, false, false, false };
        if (unit_type == "exp_destroyer")        return { ImGui::ColorConvertFloat4ToU32(sdk::color_destroyer), sdk::show_destroyer,  false, false, false, false };
        if (unit_type == "exp_cruiser")          return { ImGui::ColorConvertFloat4ToU32(sdk::color_cruiser), sdk::show_cruiser,  false, false, false, false };
        if (unit_type == "exp_torpedo_gun_boat") return { ImGui::ColorConvertFloat4ToU32(sdk::color_torpedo_boat),  sdk::show_torpedo_boat,  false, false, false, false };

        if (unit_type == "exp_fortification" ||
            unit_type == "exp_structure" ||
            unit_type == "exp_aaa" ||
            unit_type == "dummy_plane" ||
            unit_type == "exp_bridge" ||
            unit_type == "dummy_tank" ||
            unit_type == "dummy_immortal" ||
            unit_type == "structures/dummy_immortal" ||
            unit_type == "ai_bot")
        {
            return { IM_COL32(255, 255, 255, 255), false, true, false, false, false };
        }

        return { IM_COL32(255, 255, 255, 255), false, false, false, true, false };
    }

    inline unit_cached_info_t& get_cached_unit_info(const uintptr_t unit_base, c_unit& unit)
    {
        using clock = std::chrono::steady_clock;
        static std::unordered_map<uintptr_t, unit_cached_info_t> cache;
        static auto next_gc = clock::now() + std::chrono::seconds(5);

        const auto now = clock::now();
        auto& entry = cache[unit_base];

        if (now >= entry.next_refresh)
        {
            auto info = unit.getInfo();
            entry.unit_type = info.getUnitType();
            entry.vehicle_name = info.getVehicleName();
            entry.next_refresh = now + std::chrono::milliseconds(1500);
        }
        entry.last_seen = now;

        if (now >= next_gc)
        {
            for (auto it = cache.begin(); it != cache.end(); )
            {
                if (now - it->second.last_seen > std::chrono::seconds(10))
                    it = cache.erase(it);
                else
                    ++it;
            }
            next_gc = now + std::chrono::seconds(5);
        }

        return entry;
    }

    inline unit_box_cache_t& get_cached_box_info(const uintptr_t unit_base, c_unit& unit, const float distance_m)
    {
        using clock = std::chrono::steady_clock;
        static std::unordered_map<uintptr_t, unit_box_cache_t> cache;
        static auto next_gc = clock::now() + std::chrono::seconds(5);

        const auto now = clock::now();
        auto& entry = cache[unit_base];
        if (now >= entry.next_bounds_refresh)
        {
            entry.bbmin = unit.getBBMin();
            entry.bbmax = unit.getBBMax();
            entry.next_bounds_refresh = now + std::chrono::milliseconds(1000);
        }

        if (now >= entry.next_rot_refresh)
        {
            entry.rot = unit.getRotationMatrix();
            const auto rot_ttl = (distance_m <= 120.0f)
                ? std::chrono::milliseconds(16)
                : std::chrono::milliseconds(33);
            entry.next_rot_refresh = now + rot_ttl;
        }
        entry.last_seen = now;

        if (now >= next_gc)
        {
            for (auto it = cache.begin(); it != cache.end(); )
            {
                if (now - it->second.last_seen > std::chrono::seconds(10))
                    it = cache.erase(it);
                else
                    ++it;
            }
            next_gc = now + std::chrono::seconds(5);
        }

        return entry;
    }

    inline unit_reload_cache_t& get_cached_reload_info(const uintptr_t unit_base, c_unit& unit)
    {
        using clock = std::chrono::steady_clock;
        static std::unordered_map<uintptr_t, unit_reload_cache_t> cache;
        static auto next_gc = clock::now() + std::chrono::seconds(5);

        const auto now = clock::now();
        auto& entry = cache[unit_base];
        if (now >= entry.next_refresh)
        {
            entry.value = unit.getReloadTime();
            entry.next_refresh = now + std::chrono::milliseconds(80);
        }
        entry.last_seen = now;

        if (now >= next_gc)
        {
            for (auto it = cache.begin(); it != cache.end(); )
            {
                if (now - it->second.last_seen > std::chrono::seconds(10))
                    it = cache.erase(it);
                else
                    ++it;
            }
            next_gc = now + std::chrono::seconds(5);
        }

        return entry;
    }

    inline model_bounds_cache_t& get_cached_bounds_info(c_unit& unit, const unit_cached_info_t& cached_info)
    {
        using clock = std::chrono::steady_clock;
        static std::unordered_map<std::string, model_bounds_cache_t> cache;
        static auto next_gc = clock::now() + std::chrono::seconds(5);

        const std::string cache_key = !cached_info.vehicle_name.empty()
            ? cached_info.vehicle_name
            : cached_info.unit_type;
        if (cache_key.empty())
        {
            static model_bounds_cache_t fallback{};
            fallback.bbmin = unit.getBBMin();
            fallback.bbmax = unit.getBBMax();
            fallback.last_seen = clock::now();
            fallback.next_refresh = fallback.last_seen + std::chrono::milliseconds(500);
            return fallback;
        }

        const auto now = clock::now();
        auto& entry = cache[cache_key];
        if (now >= entry.next_refresh)
        {
            entry.bbmin = unit.getBBMin();
            entry.bbmax = unit.getBBMax();
            entry.next_refresh = now + std::chrono::milliseconds(2000);
        }
        entry.last_seen = now;

        if (now >= next_gc)
        {
            for (auto it = cache.begin(); it != cache.end(); )
            {
                if (now - it->second.last_seen > std::chrono::seconds(20))
                    it = cache.erase(it);
                else
                    ++it;
            }
            next_gc = now + std::chrono::seconds(5);
        }

        return entry;
    }

    inline std::array<vec3_t, 8> calculate_bbox_corners_rotated(
        const vec3_t& position,
        const vec3_t& bbmin,
        const vec3_t& bbmax,
        const rotation_matrix_t& rot)
    {
        std::array<vec3_t, 8> local = { {
            { bbmin.x, bbmin.y, bbmin.z },
            { bbmax.x, bbmin.y, bbmin.z },
            { bbmin.x, bbmax.y, bbmin.z },
            { bbmax.x, bbmax.y, bbmin.z },
            { bbmin.x, bbmin.y, bbmax.z },
            { bbmax.x, bbmin.y, bbmax.z },
            { bbmin.x, bbmax.y, bbmax.z },
            { bbmax.x, bbmax.y, bbmax.z }
        } };

        const vec3_t center{
            (bbmin.x + bbmax.x) * 0.5f,
            (bbmin.y + bbmax.y) * 0.5f,
            (bbmin.z + bbmax.z) * 0.5f
        };

        std::array<vec3_t, 8> world{};

        for (size_t i = 0; i < 8; ++i)
        {
            const vec3_t offset{
                local[i].x - center.x,
                local[i].y - center.y,
                local[i].z - center.z
            };

            const vec3_t rotated = rot.transform(offset);

            world[i] = {
                position.x + center.x + rotated.x,
                position.y + center.y + rotated.y,
                position.z + center.z + rotated.z
            };
        }

        return world;
    }

    inline void draw_wireframe_box(
        const std::array<vec2_t, 8>& c,
        ImU32 color,
        float thickness)
    {
        g_render->line(c[0].x, c[0].y, c[1].x, c[1].y, color, thickness);
        g_render->line(c[1].x, c[1].y, c[3].x, c[3].y, color, thickness);
        g_render->line(c[3].x, c[3].y, c[2].x, c[2].y, color, thickness);
        g_render->line(c[2].x, c[2].y, c[0].x, c[0].y, color, thickness);

        g_render->line(c[4].x, c[4].y, c[5].x, c[5].y, color, thickness);
        g_render->line(c[5].x, c[5].y, c[7].x, c[7].y, color, thickness);
        g_render->line(c[7].x, c[7].y, c[6].x, c[6].y, color, thickness);
        g_render->line(c[6].x, c[6].y, c[4].x, c[4].y, color, thickness);

        g_render->line(c[0].x, c[0].y, c[4].x, c[4].y, color, thickness);
        g_render->line(c[1].x, c[1].y, c[5].x, c[5].y, color, thickness);
        g_render->line(c[2].x, c[2].y, c[6].x, c[6].y, color, thickness);
        g_render->line(c[3].x, c[3].y, c[7].x, c[7].y, color, thickness);
    }

    inline bool world_to_screen_counted(
        const vec3_t& point,
        vec2_t& out,
        const ViewMatrix_t& camera_matrix,
        const projection_context_t&,
        uint32_t& w2s_counter)
    {
        ++w2s_counter;

        const ViewMatrix_t& mvp = camera_matrix;

        vec4_t clip;

#if defined(ROW_MAJOR)
        clip.x = point.x * mvp[0][0] + point.y * mvp[1][0] + point.z * mvp[2][0] + mvp[3][0];
        clip.y = point.x * mvp[0][1] + point.y * mvp[1][1] + point.z * mvp[2][1] + mvp[3][1];
        clip.z = point.x * mvp[0][2] + point.y * mvp[1][2] + point.z * mvp[2][2] + mvp[3][2];
        clip.w = point.x * mvp[0][3] + point.y * mvp[1][3] + point.z * mvp[2][3] + mvp[3][3];
#elif defined(COL_MAJOR)
        clip.x = mvp[0][0] * point.x + mvp[0][1] * point.y + mvp[0][2] * point.z + mvp[0][3];
        clip.y = mvp[1][0] * point.x + mvp[1][1] * point.y + mvp[1][2] * point.z + mvp[1][3];
        clip.z = mvp[2][0] * point.x + mvp[2][1] * point.y + mvp[2][2] * point.z + mvp[2][3];
        clip.w = mvp[3][0] * point.x + mvp[3][1] * point.y + mvp[3][2] * point.z + mvp[3][3];
#else
#error "Define ROW_MAJOR or COL_MAJOR"
#endif

        if (clip.w < 0.01f)
            return false;

        static int cached_frame = -1;
        static ImVec2 viewport_pos{};
        static ImVec2 viewport_half_size{};

        const int frame = ImGui::GetFrameCount();
        if (frame != cached_frame)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            if (!viewport)
                return false;

            viewport_pos = viewport->Pos;
            viewport_half_size = { viewport->Size.x * 0.5f, viewport->Size.y * 0.5f };
            cached_frame = frame;
        }

        if (viewport_half_size.x <= 0.0f || viewport_half_size.y <= 0.0f)
            return false;

        const float inv_w = 1.0f / clip.w;
        const float ndc_x = clip.x * inv_w;
        const float ndc_y = clip.y * inv_w;

        out.x = viewport_pos.x + (ndc_x + 1.0f) * viewport_half_size.x;
        out.y = viewport_pos.y + (1.0f - ndc_y) * viewport_half_size.y;

        return true;
    }

    inline bool is_point_in_clip_frustum(
        const vec3_t& point,
        const ViewMatrix_t& camera_matrix,
        const projection_context_t& projection_ctx,
        const float margin = 0.2f)
    {
        const float legacy_clip_x =
            (point.x * camera_matrix[0][0]) +
            (point.y * camera_matrix[1][0]) +
            (point.z * camera_matrix[2][0]) +
            camera_matrix[3][0];
        const float legacy_clip_y =
            (point.x * camera_matrix[0][1]) +
            (point.y * camera_matrix[1][1]) +
            (point.z * camera_matrix[2][1]) +
            camera_matrix[3][1];
        const float legacy_clip_w =
            (point.x * camera_matrix[0][3]) +
            (point.y * camera_matrix[1][3]) +
            (point.z * camera_matrix[2][3]) +
            camera_matrix[3][3];

        const float legacy_limit = 1.0f + margin;
        const bool legacy_visible = legacy_clip_w > 0.001f &&
            std::fabs(legacy_clip_x / legacy_clip_w) <= legacy_limit &&
            std::fabs(legacy_clip_y / legacy_clip_w) <= legacy_limit;

        if (!projection_ctx.valid)
            return legacy_visible;

        const double local_x = static_cast<double>(point.x) - static_cast<double>(projection_ctx.camera_pos.x);
        const double local_y = static_cast<double>(point.y) - static_cast<double>(projection_ctx.camera_pos.y);
        const double local_z = static_cast<double>(point.z) - static_cast<double>(projection_ctx.camera_pos.z);

        const double clip_x =
            local_x * static_cast<double>(camera_matrix[0][0]) +
            local_y * static_cast<double>(camera_matrix[1][0]) +
            local_z * static_cast<double>(camera_matrix[2][0]) +
            projection_ctx.bias_clip_x;

        const double clip_y =
            local_x * static_cast<double>(camera_matrix[0][1]) +
            local_y * static_cast<double>(camera_matrix[1][1]) +
            local_z * static_cast<double>(camera_matrix[2][1]) +
            projection_ctx.bias_clip_y;

        const double clip_w =
            local_x * static_cast<double>(camera_matrix[0][3]) +
            local_y * static_cast<double>(camera_matrix[1][3]) +
            local_z * static_cast<double>(camera_matrix[2][3]) +
            projection_ctx.bias_clip_w;

        if (clip_w <= 0.001f)
            return legacy_visible;

        const double inv_w = 1.0 / clip_w;
        const double ndc_x = clip_x * inv_w;
        const double ndc_y = clip_y * inv_w;
        const double limit = 1.0 + static_cast<double>(margin);
        const bool precise_visible = std::fabs(ndc_x) <= limit && std::fabs(ndc_y) <= limit;
        return precise_visible || legacy_visible;
    }

    inline bool draw_cheap_2d_box(
        const vec3_t& unit_pos,
        const vec3_t& bbmin,
        const vec3_t& bbmax,
        const ViewMatrix_t& camera_matrix,
        const projection_context_t& projection_ctx,
        uint32_t& w2s_counter,
        ImU32 color,
        float* out_max_y = nullptr)
    {
        vec2_t top_sp{};
        vec2_t bottom_sp{};
        vec2_t side_sp{};
        vec2_t center_sp{};

        const float center_y = (bbmin.y + bbmax.y) * 0.5f;
        const float half_width_world = std::max(std::fabs(bbmin.x), std::fabs(bbmax.x));

        const vec3_t top_world{ unit_pos.x, unit_pos.y + bbmax.y, unit_pos.z };
        const vec3_t bottom_world{ unit_pos.x, unit_pos.y + bbmin.y, unit_pos.z };
        const vec3_t side_world{ unit_pos.x + half_width_world, unit_pos.y + center_y, unit_pos.z };
        const vec3_t center_world{ unit_pos.x, unit_pos.y + center_y, unit_pos.z };

        if (!world_to_screen_counted(top_world, top_sp, camera_matrix, projection_ctx, w2s_counter) ||
            !world_to_screen_counted(bottom_world, bottom_sp, camera_matrix, projection_ctx, w2s_counter) ||
            !world_to_screen_counted(side_world, side_sp, camera_matrix, projection_ctx, w2s_counter) ||
            !world_to_screen_counted(center_world, center_sp, camera_matrix, projection_ctx, w2s_counter))
        {
            return false;
        }

        const float height = std::fabs(bottom_sp.y - top_sp.y);
        if (height < 2.0f)
            return false;

        float half_width = std::fabs(side_sp.x - center_sp.x);
        if (half_width < 1.0f)
            half_width = std::max(2.0f, height * 0.28f);

        const float min_x = center_sp.x - half_width;
        const float max_x = center_sp.x + half_width;
        const float min_y = std::min(top_sp.y, bottom_sp.y);
        const float max_y = std::max(top_sp.y, bottom_sp.y);

        g_render->rect(min_x, min_y, max_x - min_x, max_y - min_y, color, 0.0f);
        if (out_max_y)
            *out_max_y = max_y;
        return true;
    }

    struct projected_box_bounds_t
    {
        float min_x = 0.0f;
        float min_y = 0.0f;
        float max_x = 0.0f;
        float max_y = 0.0f;
        bool valid = false;
    };

    inline auto get_projected_box_bounds(
        c_unit& unit,
        const vec3_t& unit_pos,
        const float distance_f,
        const ViewMatrix_t& camera_matrix,
        const projection_context_t& projection_ctx,
        uint32_t& w2s_counter,
        uint32_t& obb_counter) -> projected_box_bounds_t
    {
        projected_box_bounds_t bounds{};
        auto& box_info = get_cached_box_info(unit.get_base(), unit, distance_f);
        ++obb_counter;
        const auto world_corners = calculate_bbox_corners_rotated(unit_pos, box_info.bbmin, box_info.bbmax, box_info.rot);

        float min_x = std::numeric_limits<float>::max();
        float min_y = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float max_y = std::numeric_limits<float>::lowest();

        bool any_visible = false;
        for (const auto& corner : world_corners)
        {
            vec2_t sp;
            if (!world_to_screen_counted(corner, sp, camera_matrix, projection_ctx, w2s_counter))
                continue;

            any_visible = true;

            min_x = std::min(min_x, sp.x);
            min_y = std::min(min_y, sp.y);
            max_x = std::max(max_x, sp.x);
            max_y = std::max(max_y, sp.y);
        }

        if (!any_visible || max_x <= min_x || max_y <= min_y)
            return bounds;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (viewport)
        {
            const float view_min_x = viewport->Pos.x;
            const float view_min_y = viewport->Pos.y;
            const float view_max_x = viewport->Pos.x + viewport->Size.x;
            const float view_max_y = viewport->Pos.y + viewport->Size.y;

            min_x = std::clamp(min_x, view_min_x, view_max_x);
            max_x = std::clamp(max_x, view_min_x, view_max_x);
            min_y = std::clamp(min_y, view_min_y, view_max_y);
            max_y = std::clamp(max_y, view_min_y, view_max_y);
        }

        bounds.min_x = min_x;
        bounds.min_y = min_y;
        bounds.max_x = max_x;
        bounds.max_y = max_y;
        bounds.valid = true;
        return bounds;
    }

    inline bool draw_projected_2d_box_from_obb(
        c_unit& unit,
        const vec3_t& unit_pos,
        const float distance_f,
        const ViewMatrix_t& camera_matrix,
        const projection_context_t& projection_ctx,
        uint32_t& w2s_counter,
        uint32_t& obb_counter,
        ImU32 color,
        float* out_max_y = nullptr)
    {
        const auto bounds = get_projected_box_bounds(unit, unit_pos, distance_f, camera_matrix, projection_ctx, w2s_counter, obb_counter);
        if (!bounds.valid)
            return false;

        g_render->rect(bounds.min_x, bounds.min_y, bounds.max_x - bounds.min_x, bounds.max_y - bounds.min_y, color, 0.0f);
        if (out_max_y)
            *out_max_y = bounds.max_y;
        return true;
    }

    inline void run()
    {
        const auto esp_start = std::chrono::steady_clock::now();
        uint64_t aimbot_us_total = 0;
        uint64_t aimbot_calls = 0;
        uint32_t w2s_calls = 0;
        uint32_t obb_calls = 0;

        if (!sdk::esp_enabled)
            return;

        const auto snapshot = misc::snapshot_frame();
        if (!snapshot)
            return;

        const auto& curr_snapshot = *snapshot;
        const auto& frame = curr_snapshot.frame;
        if (!frame.valid)
            return;

        const auto snapshot_age_us = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - curr_snapshot.captured_at
        ).count();
        if (snapshot_age_us >= 0)
            misc::prof_snapshot_age_us.store(static_cast<uint64_t>(snapshot_age_us), std::memory_order_relaxed);

        const ViewMatrix_t camera_matrix = frame.camera_matrix;
        const vec3_t camera_pos = frame.camera_pos;

        // ВРЕМЕННАЯ ДИАГНОСТИКА – можно удалить после проверки
        {
            static int once = 0;
            if (once == 0) {
                printf("[MATRIX_DEBUG] Last row: %f %f %f %f\n",
                    camera_matrix[3][0], camera_matrix[3][1],
                    camera_matrix[3][2], camera_matrix[3][3]);
                if (fabs(camera_matrix[3][0]) < 0.001f &&
                    fabs(camera_matrix[3][1]) < 0.001f &&
                    fabs(camera_matrix[3][2]) < 0.001f &&
                    fabs(camera_matrix[3][3] - 1.0f) < 0.001f)
                    printf("[MATRIX_DEBUG] Looks like ViewProjection (0,0,0,1)\n");
                else
                    printf("[MATRIX_DEBUG] WARNING: Matrix is NOT ViewProjection!\n");
                once = 1;
            }
        }

        projection_context_t projection_ctx{};
        projection_ctx.camera_pos = camera_pos;
        projection_ctx.bias_clip_x =
            static_cast<double>(camera_pos.x) * static_cast<double>(camera_matrix[0][0]) +
            static_cast<double>(camera_pos.y) * static_cast<double>(camera_matrix[1][0]) +
            static_cast<double>(camera_pos.z) * static_cast<double>(camera_matrix[2][0]) +
            static_cast<double>(camera_matrix[3][0]);
        projection_ctx.bias_clip_y =
            static_cast<double>(camera_pos.x) * static_cast<double>(camera_matrix[0][1]) +
            static_cast<double>(camera_pos.y) * static_cast<double>(camera_matrix[1][1]) +
            static_cast<double>(camera_pos.z) * static_cast<double>(camera_matrix[2][1]) +
            static_cast<double>(camera_matrix[3][1]);
        projection_ctx.bias_clip_w =
            static_cast<double>(camera_pos.x) * static_cast<double>(camera_matrix[0][3]) +
            static_cast<double>(camera_pos.y) * static_cast<double>(camera_matrix[1][3]) +
            static_cast<double>(camera_pos.z) * static_cast<double>(camera_matrix[2][3]) +
            static_cast<double>(camera_matrix[3][3]);
        projection_ctx.valid = false;

        if (frame.gui_state != GuiState::ALIVE && frame.gui_state != GuiState::SPEC)
            return;

        const float esp_render_distance = std::clamp(sdk::esp_render_distance, 1000.0f, 55000.0f);
        const float esp_render_distance_sq = esp_render_distance * esp_render_distance;
        const float esp_box_distance = std::clamp(sdk::esp_box_distance, 500.0f, 3500.0f);
        const float esp_box_distance_sq = esp_box_distance * esp_box_distance;
        constexpr float far_simplified_distance = 10000.0f;
        constexpr float far_simplified_distance_sq = far_simplified_distance * far_simplified_distance;
        ImFont* const esp_font = g_render->fonts().m_esp;

        // ---- local plane bomb impact marker ----
        if (frame.local_is_plane && frame.local_bomb_selected && sdk::esp_show_bomb_impact && !frame.bomb_impact.empty())
        {
            vec2_t sp;
            if (g_render->world_to_screen(frame.bomb_impact, sp, camera_matrix))
            {
                g_render->circle(sp.x, sp.y, 6.0f, IM_COL32(255, 0, 200, 255), 16.0f);
                draw_impact_x(sp, IM_COL32(255, 255, 255, 255));
            }
        }

        thread_local std::vector<c_unit> units;
        misc::snapshot_units_into(units);
        const vec3_t& local_pos = frame.local_pos;

        constexpr float ballistic_render_distance = 3500.0f;
        constexpr float ballistic_render_distance_sq = ballistic_render_distance * ballistic_render_distance;
        if (sdk::aimbot_enabled)
            aimbot::BeginFrame();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        const ImVec2 viewport_center = main_viewport
            ? ImVec2(main_viewport->Pos.x + main_viewport->Size.x * 0.5f, main_viewport->Pos.y + main_viewport->Size.y * 0.5f)
            : ImVec2(0.0f, 0.0f);
        const float aim_fov_radius = main_viewport
            ? (std::min(main_viewport->Size.x, main_viewport->Size.y) * 0.50f)
            : 0.0f;
        const float aim_fov_radius_sq = aim_fov_radius * aim_fov_radius;

        c_unit best_aimbot_unit{};
        vec3_t best_aimbot_pos{};
        vec2_t best_aimbot_screen_pos{};
        float best_aimbot_distance_f = 0.0f;
        ImU32 best_aimbot_color = IM_COL32(255, 255, 255, 255);
        float best_aimbot_score = std::numeric_limits<float>::max();
        bool has_best_aimbot_target = false;

        for (c_unit& unit : units)
        {
            vec3_t unit_pos;
            const auto unit_base = unit.get_base();
            const auto curr_pos_it = curr_snapshot.positions.find(unit_base);
            if (curr_pos_it == curr_snapshot.positions.end())
                continue;

            unit_pos = curr_pos_it->second;

            if (unit_pos.empty())
                continue;

            const float dx = unit_pos.x - local_pos.x;
            const float dy = unit_pos.y - local_pos.y;
            const float dz = unit_pos.z - local_pos.z;
            const float distance_sq = dx * dx + dy * dy + dz * dz;
            if (distance_sq > esp_render_distance_sq)
                continue;

            auto& cached_info = get_cached_unit_info(unit_base, unit);
            const std::string& unit_type = cached_info.unit_type;
            if (unit_type.empty())
                continue;

            const auto unit_type_info = get_unit_type_info(unit_type);
            if (unit_type_info.is_dummy || !unit_type_info.worth_to_draw)
                continue;

            if (!is_point_in_clip_frustum(unit_pos, camera_matrix, projection_ctx))
                continue;

            vec2_t screen_pos;
            if (!world_to_screen_counted(unit_pos, screen_pos, camera_matrix, projection_ctx, w2s_calls))
                continue;

            const float distance_f = std::sqrt(distance_sq);
            const std::string& vehicle_name = cached_info.vehicle_name;
            const float dx_center = screen_pos.x - viewport_center.x;
            const float dy_center = screen_pos.y - viewport_center.y;
            const float screen_dist_sq = dx_center * dx_center + dy_center * dy_center;
            const bool is_inside_fov = (aim_fov_radius_sq > 0.0f) && (screen_dist_sq <= aim_fov_radius_sq);
            bool show_name_for_unit = true;
            if (distance_sq > far_simplified_distance_sq)
                show_name_for_unit = is_inside_fov;

            float label_base_y = 0.f;

            const bool should_draw_any_box = sdk::esp_show_boxes || sdk::esp_use_3d_boxes;
            if (unit_type_info.draw_box && should_draw_any_box)
            {
                const float active_box_distance_sq = esp_box_distance_sq;
                if (distance_sq > active_box_distance_sq)
                {
                    label_base_y = screen_pos.y;
                }
                else
                {
                    if (sdk::esp_use_3d_boxes)
                    {
                        auto& box_info = get_cached_box_info(unit_base, unit, distance_f);
                        ++obb_calls;
                        const auto world_corners =
                            calculate_bbox_corners_rotated(unit_pos, box_info.bbmin, box_info.bbmax, box_info.rot);

                        std::array<vec2_t, 8> sc;
                        bool                  visible = true;

                        for (size_t i = 0; i < 8; ++i)
                        {
                            if (!world_to_screen_counted(world_corners[i], sc[i], camera_matrix, projection_ctx, w2s_calls))
                            {
                                visible = false;
                                break;
                            }
                        }

                        if (visible)
                        {
                            float box_bottom_y = sc[0].y;
                            for (size_t i = 1; i < 8; ++i)
                                box_bottom_y = std::max(box_bottom_y, sc[i].y);

                            label_base_y = box_bottom_y;
                            draw_wireframe_box(sc, unit_type_info.color, 1.f);
                        }
                    }
                    else if (sdk::esp_show_boxes)
                    {
                        float projected_max_y = 0.0f;
                        if (draw_projected_2d_box_from_obb(unit, unit_pos, distance_f, camera_matrix, projection_ctx, w2s_calls, obb_calls, unit_type_info.color, &projected_max_y))
                            label_base_y = projected_max_y;
                    }
                }
            }
            else
            {
                label_base_y = screen_pos.y;
            }

            if (label_base_y > 0.f)
            {
                float text_y = label_base_y + 5.0f;

                if (sdk::esp_show_vehicle_names && show_name_for_unit && !vehicle_name.empty())
                {
                    g_render->text(
                        { screen_pos.x, text_y },
                        unit_type_info.color,
                        0,
                        vehicle_name,
                        esp_font
                    );
                    text_y += 15.0f;
                }

                if (sdk::esp_show_distance) {
                    char  buf[16];
                    std::snprintf(buf, sizeof(buf), "%.0fm", distance_f);

                    g_render->text(
                        { screen_pos.x, text_y },
                        unit_type_info.color,
                        0,
                        buf,
                        esp_font
                    );
                    text_y += 15.0f;
                }
            }

            const bool allow_aimbot_for_unit = unit_type_info.draw_box && !unit_type_info.is_plane;
            if (sdk::aimbot_enabled && allow_aimbot_for_unit && distance_sq <= ballistic_render_distance_sq)
            {
                if (aim_fov_radius_sq > 0.0f)
                {
                    if (screen_dist_sq <= aim_fov_radius_sq)
                    {
                        const float score = screen_dist_sq + (distance_sq * 0.05f);
                        if (!has_best_aimbot_target || score < best_aimbot_score)
                        {
                            best_aimbot_unit = unit;
                            best_aimbot_pos = unit_pos;
                            best_aimbot_screen_pos = screen_pos;
                            best_aimbot_distance_f = distance_f;
                            best_aimbot_color = unit_type_info.color;
                            best_aimbot_score = score;
                            has_best_aimbot_target = true;
                        }
                    }
                }
            }
        }

        if (sdk::esp_show_reload && has_best_aimbot_target)
        {
            const uint8_t reload = get_cached_reload_info(best_aimbot_unit.get_base(), best_aimbot_unit).value;
            constexpr float stat = (10.f / 16);
            constexpr float reload_ticks_max = 16.0f;
            const float seconds = stat * reload;
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%.1fs", seconds);

            const float clamped_reload = std::min(static_cast<float>(reload), reload_ticks_max);
            const float fill_ratio = std::clamp(1.0f - (clamped_reload / reload_ticks_max), 0.0f, 1.0f);

            const auto bounds = get_projected_box_bounds(best_aimbot_unit, best_aimbot_pos, best_aimbot_distance_f, camera_matrix, projection_ctx, w2s_calls, obb_calls);
            if (bounds.valid)
            {
                const float bar_x = bounds.max_x + 4.0f;
                const float bar_top = bounds.min_y;
                const float bar_bottom = bounds.max_y;
                const float bar_height = std::max(1.0f, bar_bottom - bar_top);
                const float fill_height = bar_height * fill_ratio;

                g_render->line(bar_x, bar_top, bar_x, bar_top + bar_height, IM_COL32(35, 35, 35, 220), 1.0f);
                g_render->line(bar_x, bar_top + bar_height, bar_x, bar_top + bar_height - fill_height, IM_COL32(40, 220, 40, 255), 1.0f);

                if (sdk::esp_show_reload_seconds)
                    g_render->text({ bar_x, bar_top - 18.0f }, best_aimbot_color, 0, buf, esp_font);
            }
            else
            {
                if (sdk::esp_show_reload_seconds)
                {
                    const float text_y = best_aimbot_screen_pos.y + 24.0f;
                    g_render->text({ best_aimbot_screen_pos.x, text_y }, best_aimbot_color, 0, buf, esp_font);
                }
            }
        }

        if (sdk::aimbot_enabled && has_best_aimbot_target)
        {
            const auto aimbot_start = std::chrono::steady_clock::now();
            aimbot::run(best_aimbot_unit, best_aimbot_pos, local_pos, camera_matrix);
            const auto aimbot_end = std::chrono::steady_clock::now();
            aimbot_us_total += static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(aimbot_end - aimbot_start).count()
                );
            ++aimbot_calls;
        }

        thread_local std::vector<misc::projectile_point_t> projectiles;
        misc::snapshot_projectiles_into(projectiles);
        const float projectile_min_distance = 3.0f;
        const float projectile_max_distance = std::max(1500.0f, sdk::esp_render_distance);
        for (const auto& projectile : projectiles)
        {
            const float projectile_distance = (projectile.position - local_pos).length();
            if (projectile_distance < projectile_min_distance || projectile_distance > projectile_max_distance)
                continue;

            vec2_t projectile_sp;
            if (!world_to_screen_counted(projectile.position, projectile_sp, camera_matrix, projection_ctx, w2s_calls))
                continue;

            const ImU32 color = (projectile.type == misc::projectile_type_t::bomb)
                ? IM_COL32(255, 165, 0, 255)
                : IM_COL32(255, 0, 0, 255);

            if (g_render->draw_list())
                g_render->draw_list()->AddCircleFilled(ImVec2(projectile_sp.x, projectile_sp.y), 2.0f, color, 16);
        }

        if (sdk::esp_show_fps && g_render->draw_list() && esp_font)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            const ImVec2 anchor = viewport
                ? ImVec2(viewport->Pos.x + 15.0f, viewport->Pos.y + 15.0f)
                : ImVec2(15.0f, 15.0f);

            char fps_buf[32];
            std::snprintf(fps_buf, sizeof(fps_buf), "FPS: %.0f", ImGui::GetIO().Framerate);
            g_render->draw_list()->AddText(esp_font, esp_font->LegacySize, anchor, IM_COL32(255, 255, 255, 255), fps_buf);
        }

        const auto esp_end = std::chrono::steady_clock::now();
        misc::prof_esp_us.store(
            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(esp_end - esp_start).count()),
            std::memory_order_relaxed
        );
        misc::prof_esp_count.fetch_add(1, std::memory_order_relaxed);
        misc::prof_aimbot_us.store(aimbot_us_total, std::memory_order_relaxed);
        misc::prof_aimbot_calls.store(aimbot_calls, std::memory_order_relaxed);
        misc::prof_esp_w2s_calls.store(static_cast<uint64_t>(w2s_calls), std::memory_order_relaxed);
        misc::prof_esp_obb_calls.store(static_cast<uint64_t>(obb_calls), std::memory_order_relaxed);
    }
}
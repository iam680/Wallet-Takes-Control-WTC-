#pragma once
#include <cstdio>
#include <cmath>
#include <Windows.h>

// === НАСТРОЙКИ ОТЛАДКИ ===
#define DEBUG_MATRIX_ENABLED        1       // Включить дебаг матриц
#define DEBUG_MATRIX_FRAMES         300     // Сколько кадров выводить (5 секунд @ 60 FPS)
#define DEBUG_VIEW_MATRIX_ADDR      0x6D08C38  // Адрес из offsets::view_matrix_offset
#define DEBUG_CAMERA_STRUCT_BASE    sdk::cgame_base  // База cgame (подставь свою)
#define DEBUG_CAMERA_MATRIX_OFF     0x1D0   // Смещение внутри камеры
#define DEBUG_CAMERA_POS_OFF        0x7A0   // Позиция камеры

// === Утилиты для дампа памяти ===
namespace debug
{
    inline void dump_bytes(const char* label, uintptr_t addr, int range = 100)
    {
        printf("\n[%s] @ 0x%zX (range ±%d):\n", label, addr, range);
        printf("Offset | +00 +01 +02 +03 +04 +05 +06 +07 | +08 +09 +0A +0B +0C +0D +0E +0F\n");
        printf("-------|--------------------------------|--------------------------------\n");

        for (int i = -range; i <= range; i += 16)
        {
            uintptr_t cur = addr + i;
            uint8_t buf[16] = {};
            TargetProcess->Read(cur, buf, 16);

            printf("%+06X | ", i);
            for (int j = 0; j < 8; ++j) printf("%02X ", buf[j]);
            printf(" | ");
            for (int j = 8; j < 16; ++j) printf("%02X ", buf[j]);
            printf("\n");
        }
    }

    inline void dump_floats(const char* label, uintptr_t addr, int count = 16, int stride = 4)
    {
        printf("\n[%s] @ 0x%zX (%d x %d-byte floats):\n", label, addr, count, stride);
        for (int i = 0; i < count; ++i)
        {
            float val = 0.f;
            if (stride == 4)
                TargetProcess->Read(addr + i * 4, &val, 4);
            else if (stride == 8)
            {
                double dval = 0.0;
                TargetProcess->Read(addr + i * 8, &dval, 8);
                val = static_cast<float>(dval);
            }
            printf("  [%2d] = %+.6f\n", i, val);
        }
    }

    inline void dump_matrix_struct(const char* name, const ViewMatrix_t& m)
    {
        printf("\n[%s] 4x4 Matrix (row-major):\n", name);
        for (int r = 0; r < 4; ++r)
        {
            printf("  Row %d: ", r);
            for (int c = 0; c < 4; ++c)
                printf("%+9.4f ", m[r][c]);
            printf("\n");
        }
        printf("  Last row [3]: %.4f %.4f %.4f %.4f\n",
            m[3][0], m[3][1], m[3][2], m[3][3]);
    }

    inline bool is_likely_view_matrix(const ViewMatrix_t& m)
    {
        // View-матрица обычно имеет:
        // - Последняя строка: [0, 0, 0, 1] или близкие значения
        // - Элементы вращения в пределах [-2, 2] (косинусы/синусы)
        if (std::fabs(m[3][3] - 1.0f) > 0.01f) return false;
        if (std::fabs(m[3][0]) > 10.0f || std::fabs(m[3][1]) > 10.0f || std::fabs(m[3][2]) > 10.0f)
            return false;
        // Проверка на "мировые" координаты в позиции
        float pos_len = std::sqrt(m[3][0] * m[3][0] + m[3][1] * m[3][1] + m[3][2] * m[3][2]);
        return pos_len < 1000.0f; // Камера обычно не улетает на 10км в матрице
    }

    inline bool is_likely_world_matrix(const ViewMatrix_t& m)
    {
        // World-матрица содержит реальные координаты камеры в последней строке
        float pos_len = std::sqrt(m[3][0] * m[3][0] + m[3][1] * m[3][1] + m[3][2] * m[3][2]);
        return pos_len > 100.0f && pos_len < 100000.0f; // Реальные координаты в мире
    }

    // === Главная функция дебага ===
    inline void run_matrix_debug()
    {
        static int frame_count = 0;
        if (!DEBUG_MATRIX_ENABLED || frame_count >= DEBUG_MATRIX_FRAMES)
            return;
        ++frame_count;

        printf("\n=== MATRIX DEBUG FRAME %d ===\n", frame_count);

        // 1. Читаем по view_matrix_offset (статический адрес)
        {
            uintptr_t addr = DEBUG_CAMERA_STRUCT_BASE + DEBUG_VIEW_MATRIX_ADDR;
            ViewMatrix_t matrix{};
            if (TargetProcess->Read(addr, &matrix, sizeof(matrix)))
            {
                dump_matrix_struct("view_matrix_offset (static)", matrix);
                printf("  Type guess: %s\n",
                    is_likely_view_matrix(matrix) ? "✓ View/ViewProjection" :
                    is_likely_world_matrix(matrix) ? "✗ World/Transform" : "? Unknown");
            }
            else
                printf("[!] Failed to read view_matrix_offset @ 0x%zX\n", addr);
        }

        // 2. Читаем по camera_struct + 0x1D0
        {
            uintptr_t cam_base = DEBUG_CAMERA_STRUCT_BASE + offsets::cgame_offsets::camera_offset;
            uintptr_t addr = 0;
            TargetProcess->Read(cam_base, &addr, sizeof(uintptr_t)); // Если это указатель
            if (addr < 0x10000) addr = cam_base; // Если не указатель, используем базу

            addr += DEBUG_CAMERA_MATRIX_OFF;
            ViewMatrix_t matrix{};
            if (TargetProcess->Read(addr, &matrix, sizeof(matrix)))
            {
                dump_matrix_struct("camera_matrix_offset (0x1D0)", matrix);
                printf("  Type guess: %s\n",
                    is_likely_view_matrix(matrix) ? "✓ View/ViewProjection" :
                    is_likely_world_matrix(matrix) ? "✗ World/Transform" : "? Unknown");
            }
            else
                printf("[!] Failed to read camera_matrix_offset @ 0x%zX\n", addr);
        }

        // 3. Дамп сырых байтов вокруг view_matrix_offset
        if (frame_count == 1)
        {
            uintptr_t addr = DEBUG_CAMERA_STRUCT_BASE + DEBUG_VIEW_MATRIX_ADDR;
            dump_bytes("RAW BYTES view_matrix_offset", addr, 64);
            dump_floats("AS FLOATS (4-byte)", addr, 16, 4);
            dump_floats("AS DOUBLES (8-byte)", addr, 8, 8);
        }

        // 4. Тестовая точка: рисуем маркер в 5 метрах "перед" камерой
        if (frame_count <= 10)
        {
            vec3_t cam_pos{};
            uintptr_t pos_addr = DEBUG_CAMERA_STRUCT_BASE + offsets::cgame_offsets::camera_offsets::camera_position_offset;
            TargetProcess->Read(pos_addr, &cam_pos, sizeof(cam_pos));

            // Точка в 5 метрах по локальной оси камеры (предполагаем Z = forward)
            vec3_t test_point = { cam_pos.x, cam_pos.y, cam_pos.z + 5.0f };
            printf("\n[Test Point] World: (%.2f, %.2f, %.2f)\n",
                test_point.x, test_point.y, test_point.z);

            // Попробуем спроецировать через ОБЕ матрицы и сравним
            ViewMatrix_t m1{}, m2{};
            TargetProcess->Read(DEBUG_CAMERA_STRUCT_BASE + DEBUG_VIEW_MATRIX_ADDR, &m1, sizeof(m1));
            uintptr_t cam_base = DEBUG_CAMERA_STRUCT_BASE + offsets::cgame_offsets::camera_offset;
            uintptr_t m2_addr = 0;
            TargetProcess->Read(cam_base, &m2_addr, sizeof(uintptr_t));
            if (m2_addr < 0x10000) m2_addr = cam_base;
            TargetProcess->Read(m2_addr + DEBUG_CAMERA_MATRIX_OFF, &m2, sizeof(m2));

            auto project = [](const vec3_t& p, const ViewMatrix_t& m, int w, int h) -> vec2_t
                {
                    vec4_t clip;
                    clip.x = p.x * m[0][0] + p.y * m[1][0] + p.z * m[2][0] + m[3][0];
                    clip.y = p.x * m[0][1] + p.y * m[1][1] + p.z * m[2][1] + m[3][1];
                    clip.w = p.x * m[0][3] + p.y * m[1][3] + p.z * m[2][3] + m[3][3];
                    if (clip.w < 0.001f) return { -9999, -9999 };
                    float inv_w = 1.f / clip.w;
                    float ndc_x = clip.x * inv_w;
                    float ndc_y = clip.y * inv_w;
                    return {
                        (ndc_x * 0.5f + 0.5f) * w,
                        (1.f - ndc_y * 0.5f - 0.5f) * h
                    };
                };

            int sw = sdk::screen_width > 0 ? sdk::screen_width : 1920;
            int sh = sdk::screen_height > 0 ? sdk::screen_height : 1080;
            vec2_t sp1 = project(test_point, m1, sw, sh);
            vec2_t sp2 = project(test_point, m2, sw, sh);

            printf("  Projected via view_matrix_offset:  (%.1f, %.1f)\n", sp1.x, sp1.y);
            printf("  Projected via camera_matrix_offset: (%.1f, %.1f)\n", sp2.x, sp2.y);
            printf("  Expected: near screen center (~%d, ~%d)\n", sw / 2, sh / 2);
        }

        if (frame_count == DEBUG_MATRIX_FRAMES)
            printf("\n[DEBUG] Matrix debug completed. Check console output.\n");
    }
}
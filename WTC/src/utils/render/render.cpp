#include "..\..\includes.hpp"

void c_render::init( ) {
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg{};
    cfg.OversampleH = 2;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = false;

    cfg.RasterizerMultiply = 2.5f;

    const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesCyrillic();

    m_fonts.m_esp = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\arialbd.ttf",
        16.0f,
        &cfg,
        glyph_ranges
    );

    io.Fonts->AddFontDefault();
}

void c_render::line( const float x1, const float y1, const float x2, const float y2, ImU32 clr, const float thickness ) const {
    m_draw_list->AddLine( ImVec2( x1, y1 ), ImVec2( x2, y2 ), clr, thickness );
}

void c_render::rect( const float x, const float y, const float w, const float h, ImU32 clr, const float rounding ) const {
    m_draw_list->AddRect( ImVec2( x, y ), ImVec2( x + w, y + h ), clr, rounding );
}

void c_render::filled_rect( const float x, const float y, const float w, const float h, ImU32 color, const float rounding, const int flags ) const {
    m_draw_list->AddRectFilled( ImVec2( x, y ), ImVec2( x + w, y + h ), color, rounding, flags );
}

void c_render::filled_rect_gradient( const float x, const float y, const float w, const float h, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left ) const {
    m_draw_list->AddRectFilledMultiColor( ImVec2( x, y ), ImVec2( x + w, y + h ), col_upr_left, col_upr_right, col_bot_right, col_bot_left );
}

void c_render::triangle( const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, ImU32 clr, const float thickness ) const {
    m_draw_list->AddTriangle( ImVec2( x1, y1 ), ImVec2( x2, y2 ), ImVec2( x3, y3 ), clr, thickness );
}

void c_render::triangle_filled( const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, ImU32 clr ) const {
    m_draw_list->AddTriangleFilled( ImVec2( x1, y1 ), ImVec2( x2, y2 ), ImVec2( x3, y3 ), clr );
}

void c_render::circle( const float x1, const float y1, const float radius, ImU32 col, const int segments ) const {
    m_draw_list->AddCircle( ImVec2( x1, y1 ), radius, col, segments );
}

void c_render::circle_filled( const float x1, const float y1, const float radius, ImU32 col, const int segments ) const {
    m_draw_list->AddCircleFilled( ImVec2( x1, y1 ), radius, col, segments );
}

void c_render::text( const vec2_t& txt_pos,
    ImU32 color,
    const int flags,
    const std::string_view& message,
    ImFont* font ) const
{
    ImFont* use_font = font ? font : m_fonts.m_esp;
    if ( !m_draw_list || !use_font || message.empty() )
        return;

    const int base_alpha = ( color >> IM_COL32_A_SHIFT ) & 0xFF;
    if ( base_alpha <= 0 )
        return;

    const char* text_begin = message.data();
    const char* text_end = text_begin + message.size();

    auto calc_text_size_cached = [](ImFont* font_ptr, const char* begin, const char* end) -> ImVec2
    {
        struct text_size_cache_entry_t
        {
            uint64_t key = 0;
            uint32_t stamp = 0;
            ImVec2 size{};
        };

        constexpr size_t cache_size = 1024;
        static std::array<text_size_cache_entry_t, cache_size> cache{};
        static uint32_t current_stamp = 1;
        static int last_frame = -1;

        const int frame = ImGui::GetFrameCount();
        if (frame != last_frame)
        {
            last_frame = frame;
            ++current_stamp;
            if (current_stamp == 0)
            {
                current_stamp = 1;
                for (auto& entry : cache)
                    entry.stamp = 0;
            }
        }

        uint64_t hash = 1469598103934665603ull;
        for (const char* it = begin; it != end; ++it)
        {
            hash ^= static_cast<uint8_t>(*it);
            hash *= 1099511628211ull;
        }

        const uint64_t key =
            (static_cast<uint64_t>(reinterpret_cast<uintptr_t>(font_ptr)) * 1315423911ull) ^
            (hash + (static_cast<uint64_t>(end - begin) << 1));

        auto& slot = cache[key % cache_size];
        if (slot.stamp == current_stamp && slot.key == key)
            return slot.size;

        slot.key = key;
        slot.stamp = current_stamp;
        slot.size = font_ptr->CalcTextSizeA(font_ptr->LegacySize, FLT_MAX, 0.0f, begin, end);
        return slot.size;
    };

    const ImVec2 size = calc_text_size_cached(use_font, text_begin, text_end);
    const ImVec2 pos{ std::roundf( txt_pos.x - size.x * 0.5f ), std::roundf( txt_pos.y ) };

    const bool draw_shadow = ( flags & drop_shadow ) != 0;
    const bool draw_outline = ( flags & outline ) != 0;
    if ( draw_shadow || draw_outline )
    {
        const int outline_alpha = static_cast<int>( base_alpha * 0.6f );
        if ( outline_alpha > 0 )
        {
            const ImU32 outline_clr = IM_COL32( 0, 0, 0, outline_alpha );

            if ( draw_shadow )
                m_draw_list->AddText( use_font, use_font->LegacySize, ImVec2( pos.x + 1.0f, pos.y + 1.0f ), outline_clr, text_begin, text_end );

            if ( draw_outline )
            {
                constexpr std::array<ImVec2, 8> offsets = {
                    ImVec2{ 1, -1 }, ImVec2{ -1,  1 }, ImVec2{ -1, -1 }, ImVec2{ 1,  1 },
                    ImVec2{ 0,  1 }, ImVec2{  0, -1 }, ImVec2{  1,  0 }, ImVec2{ -1,  0 }
                };

                for ( const auto& o : offsets )
                    m_draw_list->AddText( use_font, use_font->LegacySize, ImVec2( pos.x + o.x, pos.y + o.y ), outline_clr, text_begin, text_end );
            }
        }
    }

    m_draw_list->AddText( use_font, use_font->LegacySize, pos, color, text_begin, text_end );
}


bool c_render::world_to_screen( const vec3_t& in, vec2_t& out, const ViewMatrix_t& matrix ) {
    const float w = matrix[ 0 ][ 3 ] * in.x + matrix[ 1 ][ 3 ] * in.y + matrix[ 2 ][ 3 ] * in.z + matrix[ 3 ][ 3 ];
    if ( w < 0.001f )
        return false;

    static int cached_frame = -1;
    static ImVec2 viewport_pos{};
    static ImVec2 viewport_half_size{};

    const int frame = ImGui::GetFrameCount();
    if ( frame != cached_frame )
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        if ( !viewport )
            return false;

        viewport_pos = viewport->Pos;
        viewport_half_size = { viewport->Size.x * 0.5f, viewport->Size.y * 0.5f };
        cached_frame = frame;
    }

    if ( viewport_half_size.x <= 0.0f || viewport_half_size.y <= 0.0f )
        return false;

    const float inv_w = 1.0f / w;
    const float x = ( in.x * matrix[ 0 ][ 0 ] + in.y * matrix[ 1 ][ 0 ] + in.z * matrix[ 2 ][ 0 ] + matrix[ 3 ][ 0 ] ) * inv_w;
    const float y = ( in.x * matrix[ 0 ][ 1 ] + in.y * matrix[ 1 ][ 1 ] + in.z * matrix[ 2 ][ 1 ] + matrix[ 3 ][ 1 ] ) * inv_w;

    out.x = viewport_pos.x + ( viewport_half_size.x * x ) + viewport_half_size.x;
    out.y = viewport_pos.y - ( viewport_half_size.y * y ) + viewport_half_size.y;

    return true;
}

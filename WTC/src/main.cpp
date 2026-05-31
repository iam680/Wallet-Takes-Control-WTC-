#include "includes.hpp"
#include "core/update.hpp"
#include "core/core.hpp"
#include "core/config.hpp"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
static HMONITOR                 g_CurrentMonitor = nullptr;
static bool                     g_F1DownLastFrame = false;
static bool                     g_F2DownLastFrame = false;

// Forward declarations of helper functions
bool CreateDeviceD3D( HWND hWnd );
void CleanupDeviceD3D( );
void CreateRenderTarget( );
void CleanupRenderTarget( );
void UpdateCurrentMonitor( HWND hWnd );
void ApplyFullscreenToMonitor( HWND hWnd, int index );
int clamp_monitor_index( int value, int count );
LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

static void ApplyDarkRedStyle( )
{
    ImGui::StyleColorsDark( );
    ImGuiStyle& style = ImGui::GetStyle( );
    ImVec4* colors = style.Colors;

    colors[ ImGuiCol_WindowBg ] = ImVec4( 0.08f, 0.04f, 0.05f, 0.95f );
    colors[ ImGuiCol_ChildBg ] = ImVec4( 0.10f, 0.05f, 0.06f, 0.90f );
    colors[ ImGuiCol_PopupBg ] = ImVec4( 0.12f, 0.05f, 0.06f, 0.98f );
    colors[ ImGuiCol_Border ] = ImVec4( 0.45f, 0.10f, 0.14f, 0.75f );
    colors[ ImGuiCol_FrameBg ] = ImVec4( 0.18f, 0.05f, 0.07f, 0.85f );
    colors[ ImGuiCol_FrameBgHovered ] = ImVec4( 0.35f, 0.08f, 0.11f, 0.95f );
    colors[ ImGuiCol_FrameBgActive ] = ImVec4( 0.52f, 0.10f, 0.14f, 1.00f );
    colors[ ImGuiCol_TitleBg ] = ImVec4( 0.22f, 0.05f, 0.08f, 0.95f );
    colors[ ImGuiCol_TitleBgActive ] = ImVec4( 0.35f, 0.06f, 0.10f, 1.00f );
    colors[ ImGuiCol_Button ] = ImVec4( 0.30f, 0.07f, 0.10f, 0.90f );
    colors[ ImGuiCol_ButtonHovered ] = ImVec4( 0.48f, 0.10f, 0.14f, 1.00f );
    colors[ ImGuiCol_ButtonActive ] = ImVec4( 0.62f, 0.11f, 0.16f, 1.00f );
    colors[ ImGuiCol_Header ] = ImVec4( 0.30f, 0.07f, 0.10f, 0.85f );
    colors[ ImGuiCol_HeaderHovered ] = ImVec4( 0.46f, 0.10f, 0.14f, 0.95f );
    colors[ ImGuiCol_HeaderActive ] = ImVec4( 0.58f, 0.11f, 0.16f, 1.00f );
    colors[ ImGuiCol_CheckMark ] = ImVec4( 0.95f, 0.35f, 0.40f, 1.00f );
    colors[ ImGuiCol_SliderGrab ] = ImVec4( 0.86f, 0.24f, 0.30f, 0.90f );
    colors[ ImGuiCol_SliderGrabActive ] = ImVec4( 0.98f, 0.35f, 0.42f, 1.00f );

    style.WindowRounding = 9.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.TabRounding = 6.0f;
    style.WindowPadding = ImVec2( 14.0f, 12.0f );
    style.FramePadding = ImVec2( 10.0f, 6.0f );
    style.ItemSpacing = ImVec2( 10.0f, 8.0f );
}

namespace GUI
{
    struct menu_preset_t
    {
        bool  esp_enabled = true;
        bool  aimbot_enabled = false;
        float esp_render_distance = 2000.0f;
        float esp_box_distance = 2000.0f;
        bool  esp_show_boxes = true;
        bool  esp_use_3d_boxes = true;
        bool  esp_show_vehicle_names = true;
        bool  esp_show_distance = true;
        bool  esp_show_reload = false;
        bool  esp_show_reload_seconds = false;
        bool  esp_show_fps = true;

        bool show_helicopter = true;
        bool show_bomber = true;
        bool show_fighter = true;
        bool show_attacker = true;
        bool show_light_tank = true;
        bool show_heavy_tank = true;
        bool show_medium_tank = true;
        bool show_spaa = true;
        bool show_gun_boat = true;
        bool show_destroyer = true;
        bool show_cruiser = true;
        bool show_torpedo_boat = true;

        ImVec4 color_helicopter = ImVec4( 247.f / 255.f, 163.f / 255.f, 159.f / 255.f, 1.f );
        ImVec4 color_bomber = ImVec4( 155.f / 255.f, 165.f / 255.f, 237.f / 255.f, 1.f );
        ImVec4 color_fighter = ImVec4( 241.f / 255.f, 158.f / 255.f, 112.f / 255.f, 1.f );
        ImVec4 color_attacker = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
        ImVec4 color_light_tank = ImVec4( 246.f / 255.f, 234.f / 255.f, 237.f / 255.f, 1.f );
        ImVec4 color_heavy_tank = ImVec4( 246.f / 255.f, 95.f / 255.f, 99.f / 255.f, 1.f );
        ImVec4 color_medium_tank = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
        ImVec4 color_spaa = ImVec4( 184.f / 255.f, 146.f / 255.f, 238.f / 255.f, 1.f );
        ImVec4 color_gun_boat = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
        ImVec4 color_destroyer = ImVec4( 246.f / 255.f, 95.f / 255.f, 99.f / 255.f, 1.f );
        ImVec4 color_cruiser = ImVec4( 196.f / 255.f, 249.f / 255.f, 180.f / 255.f, 1.f );
        ImVec4 color_torpedo_boat = ImVec4( 184.f / 255.f, 146.f / 255.f, 238.f / 255.f, 1.f );

        int monitor_index = 0;
        bool auto_fullscreen_on_start = false;
    };

    bool menu_open = true;
    int active_preset = 0;
    std::array<menu_preset_t, 2> presets = {};
    struct bool_binding_t { bool menu_preset_t::* preset; bool* runtime; };
    struct float_binding_t { float menu_preset_t::* preset; float* runtime; };
    struct color_binding_t { ImVec4 menu_preset_t::* preset; ImVec4* runtime; };

    inline const std::array<bool_binding_t, 22> bool_bindings = {{
        { &menu_preset_t::esp_enabled, &sdk::esp_enabled },
        { &menu_preset_t::aimbot_enabled, &sdk::aimbot_enabled },
        { &menu_preset_t::esp_show_boxes, &sdk::esp_show_boxes },
        { &menu_preset_t::esp_use_3d_boxes, &sdk::esp_use_3d_boxes },
        { &menu_preset_t::esp_show_vehicle_names, &sdk::esp_show_vehicle_names },
        { &menu_preset_t::esp_show_distance, &sdk::esp_show_distance },
        { &menu_preset_t::esp_show_reload, &sdk::esp_show_reload },
        { &menu_preset_t::esp_show_reload_seconds, &sdk::esp_show_reload_seconds },
        { &menu_preset_t::esp_show_fps, &sdk::esp_show_fps },
        { &menu_preset_t::show_helicopter, &sdk::show_helicopter },
        { &menu_preset_t::show_bomber, &sdk::show_bomber },
        { &menu_preset_t::show_fighter, &sdk::show_fighter },
        { &menu_preset_t::show_attacker, &sdk::show_attacker },
        { &menu_preset_t::show_light_tank, &sdk::show_light_tank },
        { &menu_preset_t::show_heavy_tank, &sdk::show_heavy_tank },
        { &menu_preset_t::show_medium_tank, &sdk::show_medium_tank },
        { &menu_preset_t::show_spaa, &sdk::show_spaa },
        { &menu_preset_t::show_gun_boat, &sdk::show_gun_boat },
        { &menu_preset_t::show_destroyer, &sdk::show_destroyer },
        { &menu_preset_t::show_cruiser, &sdk::show_cruiser },
        { &menu_preset_t::show_torpedo_boat, &sdk::show_torpedo_boat },
        { &menu_preset_t::auto_fullscreen_on_start, &sdk::auto_fullscreen_on_start }
    }};

    inline const std::array<float_binding_t, 2> float_bindings = {{
        { &menu_preset_t::esp_render_distance, &sdk::esp_render_distance },
        { &menu_preset_t::esp_box_distance, &sdk::esp_box_distance }
    }};

    inline const std::array<color_binding_t, 12> color_bindings = {{
        { &menu_preset_t::color_helicopter, &sdk::color_helicopter },
        { &menu_preset_t::color_bomber, &sdk::color_bomber },
        { &menu_preset_t::color_fighter, &sdk::color_fighter },
        { &menu_preset_t::color_attacker, &sdk::color_attacker },
        { &menu_preset_t::color_light_tank, &sdk::color_light_tank },
        { &menu_preset_t::color_heavy_tank, &sdk::color_heavy_tank },
        { &menu_preset_t::color_medium_tank, &sdk::color_medium_tank },
        { &menu_preset_t::color_spaa, &sdk::color_spaa },
        { &menu_preset_t::color_gun_boat, &sdk::color_gun_boat },
        { &menu_preset_t::color_destroyer, &sdk::color_destroyer },
        { &menu_preset_t::color_cruiser, &sdk::color_cruiser },
        { &menu_preset_t::color_torpedo_boat, &sdk::color_torpedo_boat }
    }};

    menu_preset_t capture_current_settings( )
    {
        menu_preset_t preset = {};
        for ( const auto& binding : bool_bindings )
            preset.*( binding.preset ) = *binding.runtime;
        for ( const auto& binding : float_bindings )
            preset.*( binding.preset ) = *binding.runtime;
        for ( const auto& binding : color_bindings )
            preset.*( binding.preset ) = *binding.runtime;

        preset.monitor_index = sdk::monitor_index;
        return preset;
    }

    void apply_preset( const menu_preset_t& preset )
    {
        for ( const auto& binding : bool_bindings )
            *binding.runtime = preset.*( binding.preset );
        for ( const auto& binding : float_bindings )
            *binding.runtime = preset.*( binding.preset );
        for ( const auto& binding : color_bindings )
            *binding.runtime = preset.*( binding.preset );

        sdk::monitor_index = clamp_monitor_index( preset.monitor_index, sdk::monitor_count );
    }

    void initialize_presets( )
    {
        presets[ 0 ] = capture_current_settings( );
        presets[ 1 ] = presets[ 0 ];

        presets[ 1 ].esp_enabled = false;
        presets[ 1 ].aimbot_enabled = true;
        presets[ 1 ].esp_render_distance = 5500.0f;
        presets[ 1 ].esp_box_distance = 3000.0f;
        presets[ 1 ].auto_fullscreen_on_start = true;

        active_preset = 0;
    }

    void sync_shared_settings_to_all_presets( )
    {
        const int safe_monitor_index = clamp_monitor_index( sdk::monitor_index, sdk::monitor_count );
        for ( auto& preset : presets )
        {
            preset.monitor_index = safe_monitor_index;
            preset.auto_fullscreen_on_start = sdk::auto_fullscreen_on_start;
        }
    }

    void cycle_preset( )
    {
        presets[ active_preset ] = capture_current_settings( );
        sync_shared_settings_to_all_presets( );
        active_preset = ( active_preset + 1 ) % static_cast<int>( presets.size( ) );
        apply_preset( presets[ active_preset ] );
    }

    void load_presets_from_map( const std::unordered_map<std::string, std::string>& values )
    {
        for ( int i = 0; i < static_cast<int>( presets.size( ) ); ++i )
        {
            menu_preset_t loaded = presets[ i ];
            const std::string prefix = "preset" + std::to_string( i + 1 ) + "_";

            auto read_bool = [&]( const char* key, bool& target )
            {
                const auto it = values.find( prefix + key );
                if ( it != values.end( ) )
                    target = config::parse_bool( it->second, target );
            };
            auto read_float = [&]( const char* key, float& target )
            {
                const auto it = values.find( prefix + key );
                if ( it != values.end( ) )
                    target = std::clamp( config::parse_float( it->second, target ), 0.0f, 100000.0f );
            };
            auto read_color = [&]( const char* key, ImVec4& target )
            {
                const auto it = values.find( prefix + key );
                if ( it != values.end( ) )
                    config::apply_color( target, it->second );
            };

            read_bool( "esp_enabled", loaded.esp_enabled );
            read_bool( "aimbot_enabled", loaded.aimbot_enabled );
            read_float( "esp_render_distance", loaded.esp_render_distance );
            read_float( "esp_box_distance", loaded.esp_box_distance );
            read_bool( "esp_show_boxes", loaded.esp_show_boxes );
            read_bool( "esp_use_3d_boxes", loaded.esp_use_3d_boxes );
            read_bool( "esp_show_vehicle_names", loaded.esp_show_vehicle_names );
            read_bool( "esp_show_distance", loaded.esp_show_distance );
            read_bool( "esp_show_reload", loaded.esp_show_reload );
            read_bool( "esp_show_reload_seconds", loaded.esp_show_reload_seconds );
            read_bool( "esp_show_fps", loaded.esp_show_fps );

            read_bool( "show_helicopter", loaded.show_helicopter );
            read_bool( "show_bomber", loaded.show_bomber );
            read_bool( "show_fighter", loaded.show_fighter );
            read_bool( "show_attacker", loaded.show_attacker );
            read_bool( "show_light_tank", loaded.show_light_tank );
            read_bool( "show_heavy_tank", loaded.show_heavy_tank );
            read_bool( "show_medium_tank", loaded.show_medium_tank );
            read_bool( "show_spaa", loaded.show_spaa );
            read_bool( "show_gun_boat", loaded.show_gun_boat );
            read_bool( "show_destroyer", loaded.show_destroyer );
            read_bool( "show_cruiser", loaded.show_cruiser );
            read_bool( "show_torpedo_boat", loaded.show_torpedo_boat );
            read_bool( "auto_fullscreen_on_start", loaded.auto_fullscreen_on_start );

            read_color( "color_helicopter", loaded.color_helicopter );
            read_color( "color_bomber", loaded.color_bomber );
            read_color( "color_fighter", loaded.color_fighter );
            read_color( "color_attacker", loaded.color_attacker );
            read_color( "color_light_tank", loaded.color_light_tank );
            read_color( "color_heavy_tank", loaded.color_heavy_tank );
            read_color( "color_medium_tank", loaded.color_medium_tank );
            read_color( "color_spaa", loaded.color_spaa );
            read_color( "color_gun_boat", loaded.color_gun_boat );
            read_color( "color_destroyer", loaded.color_destroyer );
            read_color( "color_cruiser", loaded.color_cruiser );
            read_color( "color_torpedo_boat", loaded.color_torpedo_boat );

            if ( const auto it = values.find( prefix + "monitor_index" ); it != values.end( ) )
                loaded.monitor_index = config::parse_int( it->second, loaded.monitor_index );

            loaded.esp_render_distance = std::clamp( loaded.esp_render_distance, 1000.0f, 55000.0f );
            loaded.esp_box_distance = std::clamp( loaded.esp_box_distance, 500.0f, 3500.0f );
            presets[ i ] = loaded;
        }

        if ( const auto it = values.find( "active_preset" ); it != values.end( ) )
            active_preset = std::clamp( config::parse_int( it->second, active_preset ), 0, static_cast<int>( presets.size( ) ) - 1 );

        apply_preset( presets[ active_preset ] );
    }

    void append_presets_to_stream( std::ostream& output )
    {
        output << "active_preset=" << std::clamp( active_preset, 0, static_cast<int>( presets.size( ) ) - 1 ) << '\n';

        for ( int i = 0; i < static_cast<int>( presets.size( ) ); ++i )
        {
            const menu_preset_t& preset = presets[ i ];
            const std::string prefix = "preset" + std::to_string( i + 1 ) + "_";

            output << prefix << "esp_enabled=" << ( preset.esp_enabled ? 1 : 0 ) << '\n';
            output << prefix << "aimbot_enabled=" << ( preset.aimbot_enabled ? 1 : 0 ) << '\n';
            output << prefix << "esp_render_distance=" << preset.esp_render_distance << '\n';
            output << prefix << "esp_box_distance=" << preset.esp_box_distance << '\n';
            output << prefix << "esp_show_boxes=" << ( preset.esp_show_boxes ? 1 : 0 ) << '\n';
            output << prefix << "esp_use_3d_boxes=" << ( preset.esp_use_3d_boxes ? 1 : 0 ) << '\n';
            output << prefix << "esp_show_vehicle_names=" << ( preset.esp_show_vehicle_names ? 1 : 0 ) << '\n';
            output << prefix << "esp_show_distance=" << ( preset.esp_show_distance ? 1 : 0 ) << '\n';
            output << prefix << "esp_show_reload=" << ( preset.esp_show_reload ? 1 : 0 ) << '\n';
            output << prefix << "esp_show_reload_seconds=" << ( preset.esp_show_reload_seconds ? 1 : 0 ) << '\n';
            output << prefix << "esp_show_fps=" << ( preset.esp_show_fps ? 1 : 0 ) << '\n';

            output << prefix << "show_helicopter=" << ( preset.show_helicopter ? 1 : 0 ) << '\n';
            output << prefix << "show_bomber=" << ( preset.show_bomber ? 1 : 0 ) << '\n';
            output << prefix << "show_fighter=" << ( preset.show_fighter ? 1 : 0 ) << '\n';
            output << prefix << "show_attacker=" << ( preset.show_attacker ? 1 : 0 ) << '\n';
            output << prefix << "show_light_tank=" << ( preset.show_light_tank ? 1 : 0 ) << '\n';
            output << prefix << "show_heavy_tank=" << ( preset.show_heavy_tank ? 1 : 0 ) << '\n';
            output << prefix << "show_medium_tank=" << ( preset.show_medium_tank ? 1 : 0 ) << '\n';
            output << prefix << "show_spaa=" << ( preset.show_spaa ? 1 : 0 ) << '\n';
            output << prefix << "show_gun_boat=" << ( preset.show_gun_boat ? 1 : 0 ) << '\n';
            output << prefix << "show_destroyer=" << ( preset.show_destroyer ? 1 : 0 ) << '\n';
            output << prefix << "show_cruiser=" << ( preset.show_cruiser ? 1 : 0 ) << '\n';
            output << prefix << "show_torpedo_boat=" << ( preset.show_torpedo_boat ? 1 : 0 ) << '\n';
            output << prefix << "auto_fullscreen_on_start=" << ( preset.auto_fullscreen_on_start ? 1 : 0 ) << '\n';

            output << prefix << "color_helicopter=" << config::color_to_string( preset.color_helicopter ) << '\n';
            output << prefix << "color_bomber=" << config::color_to_string( preset.color_bomber ) << '\n';
            output << prefix << "color_fighter=" << config::color_to_string( preset.color_fighter ) << '\n';
            output << prefix << "color_attacker=" << config::color_to_string( preset.color_attacker ) << '\n';
            output << prefix << "color_light_tank=" << config::color_to_string( preset.color_light_tank ) << '\n';
            output << prefix << "color_heavy_tank=" << config::color_to_string( preset.color_heavy_tank ) << '\n';
            output << prefix << "color_medium_tank=" << config::color_to_string( preset.color_medium_tank ) << '\n';
            output << prefix << "color_spaa=" << config::color_to_string( preset.color_spaa ) << '\n';
            output << prefix << "color_gun_boat=" << config::color_to_string( preset.color_gun_boat ) << '\n';
            output << prefix << "color_destroyer=" << config::color_to_string( preset.color_destroyer ) << '\n';
            output << prefix << "color_cruiser=" << config::color_to_string( preset.color_cruiser ) << '\n';
            output << prefix << "color_torpedo_boat=" << config::color_to_string( preset.color_torpedo_boat ) << '\n';
            output << prefix << "monitor_index=" << preset.monitor_index << '\n';
        }
    }

    void OnRender()
    {
        if ( !menu_open )
            return;

        ImGui::SetNextWindowSize( ImVec2( 800, 600 ), ImGuiCond_FirstUseEver );
        ImGui::SetNextWindowPos( ImVec2( 100, 100 ), ImGuiCond_FirstUseEver );

        if ( ImGui::Begin( "[F1 Hide/Show] [F2 Change Preset] [Menu]", &menu_open, ImGuiWindowFlags_NoCollapse ) )
        {
            ImGui::Text( "Active preset: %d", active_preset + 1 );
            ImGui::Separator( );
            ImGui::Checkbox( "ESP Enabled", &sdk::esp_enabled );
            ImGui::SameLine( );
            ImGui::Checkbox( "Show FPS", &sdk::esp_show_fps );
            ImGui::SameLine( );
            if ( ImGui::Checkbox( "Enable V-Sync", &sdk::esp_vsync_enabled ) )
                config::save( );
            ImGui::SameLine( );
            static int vsync_fps_input = 120;
            static bool vsync_input_initialized = false;
            if ( !vsync_input_initialized )
            {
                vsync_fps_input = sdk::esp_vsync_fps;
                vsync_input_initialized = true;
            }
            ImGui::SetNextItemWidth( 80.0f );
            ImGui::InputInt( "##vsync_fps", &vsync_fps_input, 0, 0 );
            ImGui::SameLine( );
            if ( ImGui::Button( "Apply V-Sync" ) )
            {
                sdk::esp_vsync_fps = std::clamp( vsync_fps_input, 30, 500 );
                vsync_fps_input = sdk::esp_vsync_fps;
                config::save( );
            }
            ImGui::Checkbox( "Ballistic Calculator Enabled", &sdk::aimbot_enabled );
            ImGui::SliderFloat( "ESP Render Distance", &sdk::esp_render_distance, 1000.f, 55000.f, "%.0f m" );
            ImGui::SliderFloat( "ESP Box Distance", &sdk::esp_box_distance, 500.f, 3500.f, "%.0f m" );
            ImGui::Checkbox( "ESP Show 2D Box", &sdk::esp_show_boxes );
            ImGui::Checkbox( "ESP Use 3D Boxes", &sdk::esp_use_3d_boxes );
            ImGui::Checkbox( "ESP Show Vehicle Names", &sdk::esp_show_vehicle_names );
            ImGui::Checkbox( "ESP Show Distance", &sdk::esp_show_distance );
            ImGui::Checkbox( "ESP Show Reload", &sdk::esp_show_reload );
            ImGui::SameLine( );
            ImGui::Checkbox( "Show Seconds", &sdk::esp_show_reload_seconds );
            ImGui::SeparatorText( "Vehicle filters" );

            const auto draw_vehicle_row = []( const char* label, bool* enabled, ImVec4* color )
            {
                ImGui::PushID( label );
                ImGui::Checkbox( label, enabled );
                ImGui::SameLine( 320.0f );
                ImGui::SetNextItemWidth( 170.0f );
                ImGui::ColorEdit4( "##color", ( float* )color, ImGuiColorEditFlags_NoInputs );
                ImGui::PopID( );
            };

            ImGui::Checkbox( "ESP Show Bomb Impact", &sdk::esp_show_bomb_impact );
            draw_vehicle_row( "Show Helicopter", &sdk::show_helicopter, &sdk::color_helicopter );
            draw_vehicle_row( "Show Bomber", &sdk::show_bomber, &sdk::color_bomber );
            draw_vehicle_row( "Show Fighter", &sdk::show_fighter, &sdk::color_fighter );
            draw_vehicle_row( "Show Attacker", &sdk::show_attacker, &sdk::color_attacker );
            draw_vehicle_row( "Show Light Tank", &sdk::show_light_tank, &sdk::color_light_tank );
            draw_vehicle_row( "Show Heavy Tank", &sdk::show_heavy_tank, &sdk::color_heavy_tank );
            draw_vehicle_row( "Show Medium Tank", &sdk::show_medium_tank, &sdk::color_medium_tank );
            draw_vehicle_row( "Show SPAA", &sdk::show_spaa, &sdk::color_spaa );
            draw_vehicle_row( "Show Gun Boat", &sdk::show_gun_boat, &sdk::color_gun_boat );
            draw_vehicle_row( "Show Destroyer", &sdk::show_destroyer, &sdk::color_destroyer );
            draw_vehicle_row( "Show Cruiser", &sdk::show_cruiser, &sdk::color_cruiser );
            draw_vehicle_row( "Show Torpedo Boat", &sdk::show_torpedo_boat, &sdk::color_torpedo_boat );
            ImGui::Separator( );

            int safe_monitor_index = sdk::monitor_index;
            if ( sdk::monitor_count <= 0 )
                safe_monitor_index = 0;
            else
            {
                if ( safe_monitor_index < 0 )
                    safe_monitor_index = 0;
                if ( safe_monitor_index > sdk::monitor_count - 1 )
                    safe_monitor_index = sdk::monitor_count - 1;
            }
            sdk::monitor_index = safe_monitor_index;

            const char* preview = "Unknown monitor";
            if ( !sdk::monitor_labels.empty( ) && safe_monitor_index < ( int )sdk::monitor_labels.size( ) )
                preview = sdk::monitor_labels[ safe_monitor_index ].c_str( );

            if ( ImGui::BeginCombo( "Monitor", preview ) )
            {
                for ( int i = 0; i < ( int )sdk::monitor_labels.size( ); ++i )
                {
                    const bool is_selected = ( sdk::monitor_index == i );
                    if ( ImGui::Selectable( sdk::monitor_labels[ i ].c_str( ), is_selected ) )
                    {
                        sdk::monitor_index = i;
                        sync_shared_settings_to_all_presets( );
                        config::save( );
                    }

                    if ( is_selected )
                        ImGui::SetItemDefaultFocus( );
                }
                ImGui::EndCombo( );
            }

            if ( ImGui::Button( "Apply Fullscreen To Monitor" ) )
                sdk::request_monitor_apply = true;
            if ( ImGui::Checkbox( "Auto Fullscreen On Start", &sdk::auto_fullscreen_on_start ) )
            {
                sync_shared_settings_to_all_presets( );
                config::save( );
            }

            ImGui::Separator( );
            if ( ImGui::Button( "Restart" ) )
                sdk::request_manual_reload = true;

            if ( !sdk::manual_reload_status.empty( ) )
                ImGui::TextWrapped( "Reload status: %s", sdk::manual_reload_status.c_str( ) );

            ImGui::Separator( );
            static std::string offsets_file_status;
            if ( ImGui::Button( "Create offsets.txt" ) )
            {
                const bool saved = offsets::create_or_update_external_offsets_file( );
                offsets_file_status = saved ? "offsets.txt updated" : "failed to write offsets.txt";
            }
            ImGui::SameLine( );
            bool read_offsets_txt = offsets::is_external_offsets_enabled( );
            if ( ImGui::Checkbox( "Read offsets.txt", &read_offsets_txt ) )
            {
                offsets::set_external_offsets_enabled( read_offsets_txt );
                offsets_file_status = read_offsets_txt
                    ? "read source: offsets.txt"
                    : "read source: offsets.hpp";
                sdk::request_manual_reload = true;
                sdk::manual_reload_status = "Restart requested (offset source changed)";
            }

            if ( !offsets_file_status.empty( ) )
                ImGui::TextWrapped( "Offsets status: %s", offsets_file_status.c_str( ) );

            ImGui::Separator( );
            if ( ImGui::Button( "Close Menu" ) )
                menu_open = false;
        }
        ImGui::End( );
    }
}

int main( int, char** )
{
    // Build date lock removed.

    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness( );
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor( ::MonitorFromPoint( POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY ) );

    // Create application window
    WNDCLASSEXW wc = { sizeof( wc ), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle( nullptr ), nullptr, nullptr, nullptr, nullptr, L"SDK - ", nullptr };
    ::RegisterClassExW( &wc );
    HWND hwnd = ::CreateWindowExW( WS_EX_APPWINDOW, wc.lpszClassName, L"SDK - ", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, nullptr, nullptr, wc.hInstance, nullptr );
    GUI::initialize_presets( );
    UpdateCurrentMonitor( hwnd );
    config::load( );
    GUI::menu_open = true;

    // Initialize Direct3D
    if ( !CreateDeviceD3D( hwnd ) )
    {
        CleanupDeviceD3D( );
        ::UnregisterClassW( wc.lpszClassName, wc.hInstance );
        return TRUE;
    }

    // Show the window
    ::ShowWindow( hwnd, SW_SHOWDEFAULT );
    ::UpdateWindow( hwnd );

    if ( sdk::auto_fullscreen_on_start )
        ApplyFullscreenToMonitor( hwnd, sdk::monitor_index );

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO& io = ImGui::GetIO( ); ( void )io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ApplyDarkRedStyle( );

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle( );
    style.ScaleAllSizes( main_scale );      // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
    {
        style.WindowRounding = 0.0f;
        style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init( hwnd );
    ImGui_ImplDX11_Init( g_pd3dDevice, g_pd3dDeviceContext );

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // render init
    g_render->init( );

    // hack init
    if ( !core::Thread( ) )
        return FALSE;

    // Main loop
    bool done = false;
    while ( !done )
    {
        const auto frame_start = std::chrono::steady_clock::now();
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while ( ::PeekMessage( &msg, nullptr, 0U, 0U, PM_REMOVE ) )
        {
            ::TranslateMessage( &msg );
            ::DispatchMessage( &msg );
            if ( msg.message == WM_QUIT )
                done = true;
        }
        if ( done )
            break;

        const bool is_foreground_process = [ ]( )
        {
            const HWND foreground = ::GetForegroundWindow( );
            if ( !foreground )
                return false;

            DWORD foreground_process_id = 0;
            ::GetWindowThreadProcessId( foreground, &foreground_process_id );
            return foreground_process_id == ::GetCurrentProcessId( );
        }( );

        const bool f1_down_now = ( ::GetAsyncKeyState( VK_F1 ) & 0x8000 ) != 0;
        if ( is_foreground_process && f1_down_now && !g_F1DownLastFrame )
        {
            GUI::menu_open = !GUI::menu_open;
            config::save( );
        }
        g_F1DownLastFrame = f1_down_now;

        const bool f2_down_now = ( ::GetAsyncKeyState( VK_F2 ) & 0x8000 ) != 0;
        if ( is_foreground_process && f2_down_now && !g_F2DownLastFrame )
        {
            GUI::cycle_preset( );
            config::save( );
        }
        g_F2DownLastFrame = f2_down_now;

        // Handle window being minimized or screen locked
        if ( g_SwapChainOccluded && g_pSwapChain->Present( 0, DXGI_PRESENT_TEST ) == DXGI_STATUS_OCCLUDED )
        {
            ::Sleep( 10 );
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if ( g_ResizeWidth != 0 && g_ResizeHeight != 0 )
        {
            CleanupRenderTarget( );
            const HRESULT resize_hr = g_pSwapChain->ResizeBuffers( 0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0 );
            if ( FAILED( resize_hr ) )
                LOG( "[D3D] ResizeBuffers failed: 0x%08X\n", static_cast<unsigned int>( resize_hr ) );
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget( );
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );

        const ImVec2 display_size = io.DisplaySize;
        if ( display_size.x > 0.0f && display_size.y > 0.0f )
        {
            sdk::screen_width = static_cast<int>( display_size.x );
            sdk::screen_height = static_cast<int>( display_size.y );
        }

        core::MaintainConnection( false );

        const int clamped_monitor_index = clamp_monitor_index( sdk::monitor_index, sdk::monitor_count );
        if ( clamped_monitor_index != sdk::monitor_index )
            sdk::monitor_index = clamped_monitor_index;

        if ( sdk::request_monitor_apply )
        {
            ApplyFullscreenToMonitor( hwnd, sdk::monitor_index );
            sdk::request_monitor_apply = false;
        }

        if ( sdk::request_manual_reload )
        {
            if ( core::RestartRuntime( ) )
                sdk::manual_reload_status = "OK";
            else if ( sdk::manual_reload_status.empty( ) )
                sdk::manual_reload_status = "Failed";
            sdk::request_manual_reload = false;
        }

        g_render->draw_list( ) = ImGui::GetBackgroundDrawList( );

        GUI::OnRender( );
        if ( g_render->draw_list( ) )
        {
            // Refresh frame snapshot on render tick to minimize camera/entity age mismatch.
            misc::UpdateFrameSnapshot( );
            esp::run( );
        }

        // Rendering
        ImGui::Render( );
        const float clear_color_with_alpha[ 4 ] = { 0, 0, 0, 0 };
        g_pd3dDeviceContext->OMSetRenderTargets( 1, &g_mainRenderTargetView, nullptr );
        g_pd3dDeviceContext->ClearRenderTargetView( g_mainRenderTargetView, clear_color_with_alpha );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

        // Update and Render additional Platform Windows
        if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
        {
            ImGui::UpdatePlatformWindows( );
            ImGui::RenderPlatformWindowsDefault( );
        }

        static auto next_vsync_tick = std::chrono::steady_clock::now();
        HRESULT hr = S_OK;
        if ( sdk::esp_vsync_enabled )
        {
            const int target_fps = std::clamp( sdk::esp_vsync_fps, 30, 500 );
            if ( target_fps > 0 )
            {
                const auto frame_period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<double>( 1.0 / static_cast<double>( target_fps ) )
                );

                next_vsync_tick += frame_period;
                const auto now_tick = std::chrono::steady_clock::now();
                if ( next_vsync_tick > now_tick )
                    std::this_thread::sleep_until( next_vsync_tick );
                else
                    next_vsync_tick = now_tick;
            }
            hr = g_pSwapChain->Present( 0, DXGI_PRESENT_DO_NOT_WAIT );
            if ( hr == DXGI_ERROR_WAS_STILL_DRAWING )
                hr = S_OK;
            else if ( FAILED( hr ) && hr != DXGI_STATUS_OCCLUDED )
                hr = S_OK;
        }
        else
        {
            next_vsync_tick = std::chrono::steady_clock::now();
            hr = g_pSwapChain->Present( 0, DXGI_PRESENT_DO_NOT_WAIT );
            if ( hr == DXGI_ERROR_WAS_STILL_DRAWING )
                hr = S_OK;
            else if ( FAILED( hr ) && hr != DXGI_STATUS_OCCLUDED )
                hr = g_pSwapChain->Present( 0, 0 );
        }
        g_SwapChainOccluded = ( hr == DXGI_STATUS_OCCLUDED );

        const auto frame_end = std::chrono::steady_clock::now();
        misc::prof_main_frame_us.store(
            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start).count()),
            std::memory_order_relaxed
        );
        misc::prof_main_frame_count.fetch_add(1, std::memory_order_relaxed);
    }

    config::save( );
    core::Shutdown( );

    // Cleanup
    ImGui_ImplDX11_Shutdown( );
    ImGui_ImplWin32_Shutdown( );
    ImGui::DestroyContext( );

    CleanupDeviceD3D( );
    ::DestroyWindow( hwnd );
    ::UnregisterClassW( wc.lpszClassName, wc.hInstance );

    return FALSE;
}

// Helper functions
bool CreateDeviceD3D( HWND hWnd )
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[ 2 ] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext );
    if ( res == DXGI_ERROR_UNSUPPORTED ) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext );
    if ( res != S_OK )
        return false;

    CreateRenderTarget( );
    return true;
}

void CleanupDeviceD3D( )
{
    CleanupRenderTarget( );
    if ( g_pSwapChain ) { g_pSwapChain->Release( ); g_pSwapChain = nullptr; }
    if ( g_pd3dDeviceContext ) { g_pd3dDeviceContext->Release( ); g_pd3dDeviceContext = nullptr; }
    if ( g_pd3dDevice ) { g_pd3dDevice->Release( ); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget( )
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    const HRESULT get_buffer_hr = g_pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
    if ( FAILED( get_buffer_hr ) || !pBackBuffer )
    {
        LOG( "[D3D] GetBuffer failed: 0x%08X\n", static_cast<unsigned int>( get_buffer_hr ) );
        g_mainRenderTargetView = nullptr;
        return;
    }

    const HRESULT create_rtv_hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_mainRenderTargetView );
    if ( FAILED( create_rtv_hr ) )
    {
        LOG( "[D3D] CreateRenderTargetView failed: 0x%08X\n", static_cast<unsigned int>( create_rtv_hr ) );
        g_mainRenderTargetView = nullptr;
    }

    pBackBuffer->Release( );
}

void CleanupRenderTarget( )
{
    if ( g_mainRenderTargetView ) { g_mainRenderTargetView->Release( ); g_mainRenderTargetView = nullptr; }
}

int clamp_monitor_index( int value, int count )
{
    if ( count <= 0 )
        return 0;
    if ( value < 0 )
        return 0;
    if ( value > count - 1 )
        return count - 1;
    return value;
}

struct monitor_apply_t
{
    int current_index = -1;
    int target_index = 0;
    bool found = false;
    RECT rc = {};
};

BOOL CALLBACK collect_monitor_for_index( HMONITOR monitor, HDC, LPRECT, LPARAM data )
{
    monitor_apply_t* result = reinterpret_cast<monitor_apply_t*>( data );
    if ( !result )
        return FALSE;

    result->current_index++;
    if ( result->current_index != result->target_index )
        return TRUE;

    MONITORINFO monitor_info = {};
    monitor_info.cbSize = sizeof( monitor_info );
    if ( !::GetMonitorInfoW( monitor, &monitor_info ) )
        return FALSE;

    result->rc = monitor_info.rcMonitor;
    result->found = true;
    return FALSE;
}

struct monitor_list_state_t
{
    int count = 0;
    int current_index = 0;
    HMONITOR current_monitor = nullptr;
    std::vector<std::string> labels = {};
};

BOOL CALLBACK collect_monitor_list_callback( HMONITOR monitor, HDC, LPRECT, LPARAM data )
{
    monitor_list_state_t* state = reinterpret_cast<monitor_list_state_t*>( data );
    if ( !state )
        return FALSE;

    MONITORINFOEXW monitor_info = {};
    monitor_info.cbSize = sizeof( monitor_info );
    if ( !::GetMonitorInfoW( monitor, &monitor_info ) )
        return TRUE;

    const RECT& rc = monitor_info.rcMonitor;
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;

    char device_name[ CCHDEVICENAME ] = {};
    ::WideCharToMultiByte(
        CP_UTF8,
        0,
        monitor_info.szDevice,
        -1,
        device_name,
        static_cast<int>( sizeof( device_name ) ),
        nullptr,
        nullptr
    );

    std::ostringstream label;
    label << device_name << " (" << width << "x" << height << ")";
    state->labels.emplace_back( label.str() );

    if ( monitor == state->current_monitor )
        state->current_index = state->count;

    state->count++;
    return TRUE;
}

void UpdateCurrentMonitor( HWND hWnd )
{
    monitor_list_state_t state = {};
    state.current_monitor = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
    ::EnumDisplayMonitors( nullptr, nullptr, collect_monitor_list_callback, reinterpret_cast<LPARAM>( &state ) );

    sdk::monitor_count = std::max( state.count, 1 );
    if ( !state.labels.empty() )
        sdk::monitor_labels = std::move( state.labels );
    else
        sdk::monitor_labels = { "Unknown monitor" };

    sdk::monitor_index = clamp_monitor_index( sdk::monitor_index, sdk::monitor_count );
    g_CurrentMonitor = state.current_monitor;
}

void ApplyFullscreenToMonitor( HWND hWnd, int index )
{
    sdk::monitor_index = index;

    monitor_apply_t data = {};
    data.target_index = sdk::monitor_index;
    ::EnumDisplayMonitors( nullptr, nullptr, collect_monitor_for_index, reinterpret_cast<LPARAM>( &data ) );
    if ( !data.found )
        return;

    const RECT& rc = data.rc;
    ::SetWindowLongPtrW( hWnd, GWL_STYLE, WS_POPUP );
    ::SetWindowLongPtrW( hWnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_APPWINDOW );
    ::SetWindowPos(
        hWnd,
        HWND_TOPMOST,
        rc.left,
        rc.top,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );

    g_ResizeWidth = static_cast<UINT>( rc.right - rc.left );
    g_ResizeHeight = static_cast<UINT>( rc.bottom - rc.top );
    UpdateCurrentMonitor( hWnd );
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam ) )
        return true;

    switch ( msg )
    {
    case WM_SIZE:
        if ( wParam == SIZE_MINIMIZED )
            return 0;
        g_ResizeWidth = ( UINT )LOWORD( lParam ); // Queue resize
        g_ResizeHeight = ( UINT )HIWORD( lParam );
        return 0;
    case WM_MOVE:
    case WM_DISPLAYCHANGE:
        UpdateCurrentMonitor( hWnd );
        return 0;
    case WM_SYSCOMMAND:
        if ( ( wParam & 0xfff0 ) == SC_KEYMENU ) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage( 0 );
        return 0;
    }
    return ::DefWindowProcW( hWnd, msg, wParam, lParam );
}

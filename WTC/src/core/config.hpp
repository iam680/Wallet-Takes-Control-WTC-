#pragma once

namespace GUI
{
    extern bool menu_open;
    void load_presets_from_map( const std::unordered_map<std::string, std::string>& values );
    void append_presets_to_stream( std::ostream& output );
}

namespace config
{
    inline constexpr const char* default_config_content = R"(menu_open=1
esp_enabled=1
aimbot_enabled=1
esp_show_boxes=1
esp_use_3d_boxes=0
esp_show_vehicle_names=1
esp_show_distance=1
esp_show_reload=1
esp_show_reload_seconds=0
esp_show_fps=1
esp_vsync_enabled=0
esp_vsync_fps=120
auto_fullscreen_on_start=0
show_helicopter=1
show_bomber=1
show_fighter=1
show_attacker=1
show_light_tank=1
show_heavy_tank=1
show_medium_tank=1
show_spaa=1
show_gun_boat=1
show_destroyer=1
show_cruiser=1
show_torpedo_boat=1
esp_show_bomb_impact=1
esp_render_distance=14500
esp_box_distance=3500
entity_update_interval_ms=100
frame_update_interval_ms=8
monitor_index=0
color_helicopter=0.968627,0.639216,0.623529,1
color_bomber=0.607843,0.647059,0.929412,1
color_fighter=0.945098,0.619608,0.439216,1
color_attacker=0.768627,0.976471,0.705882,1
color_light_tank=0.964706,0.917647,0.929412,1
color_heavy_tank=0.964706,0.372549,0.388235,1
color_medium_tank=0.768627,0.976471,0.705882,1
color_spaa=0.721569,0.572549,0.933333,1
color_gun_boat=0.768627,0.976471,0.705882,1
color_destroyer=0.964706,0.372549,0.388235,1
color_cruiser=0.768627,0.976471,0.705882,1
color_torpedo_boat=0.721569,0.572549,0.933333,1
active_preset=0
preset1_esp_enabled=1
preset1_aimbot_enabled=1
preset1_esp_render_distance=14500
preset1_esp_box_distance=3500
preset1_esp_show_boxes=1
preset1_esp_use_3d_boxes=0
preset1_esp_show_vehicle_names=1
preset1_esp_show_distance=1
preset1_esp_show_reload=1
preset1_esp_show_reload_seconds=0
preset1_esp_show_fps=1
preset1_show_helicopter=1
preset1_show_bomber=1
preset1_show_fighter=1
preset1_show_attacker=1
preset1_show_light_tank=1
preset1_show_heavy_tank=1
preset1_show_medium_tank=1
preset1_show_spaa=1
preset1_show_gun_boat=1
preset1_show_destroyer=1
preset1_show_cruiser=1
preset1_show_torpedo_boat=1
preset1_auto_fullscreen_on_start=0
preset1_color_helicopter=0.968627,0.639216,0.623529,1
preset1_color_bomber=0.607843,0.647059,0.929412,1
preset1_color_fighter=0.945098,0.619608,0.439216,1
preset1_color_attacker=0.768627,0.976471,0.705882,1
preset1_color_light_tank=0.964706,0.917647,0.929412,1
preset1_color_heavy_tank=0.964706,0.372549,0.388235,1
preset1_color_medium_tank=0.768627,0.976471,0.705882,1
preset1_color_spaa=0.721569,0.572549,0.933333,1
preset1_color_gun_boat=0.768627,0.976471,0.705882,1
preset1_color_destroyer=0.964706,0.372549,0.388235,1
preset1_color_cruiser=0.768627,0.976471,0.705882,1
preset1_color_torpedo_boat=0.721569,0.572549,0.933333,1
preset1_monitor_index=0
preset2_esp_enabled=1
preset2_aimbot_enabled=0
preset2_esp_render_distance=30000
preset2_esp_box_distance=500
preset2_esp_show_boxes=0
preset2_esp_use_3d_boxes=0
preset2_esp_show_vehicle_names=1
preset2_esp_show_distance=1
preset2_esp_show_reload=0
preset2_esp_show_reload_seconds=1
preset2_esp_show_fps=1
preset2_show_helicopter=1
preset2_show_bomber=1
preset2_show_fighter=1
preset2_show_attacker=1
preset2_show_light_tank=1
preset2_show_heavy_tank=1
preset2_show_medium_tank=1
preset2_show_spaa=1
preset2_show_gun_boat=1
preset2_show_destroyer=1
preset2_show_cruiser=1
preset2_show_torpedo_boat=1
preset2_auto_fullscreen_on_start=0
preset2_color_helicopter=0.968627,0.639216,0.623529,1
preset2_color_bomber=0.607843,0.647059,0.929412,1
preset2_color_fighter=0.945098,0.619608,0.439216,1
preset2_color_attacker=0.768627,0.976471,0.705882,1
preset2_color_light_tank=0.964706,0.917647,0.929412,1
preset2_color_heavy_tank=0.964706,0.372549,0.388235,1
preset2_color_medium_tank=0.768627,0.976471,0.705882,1
preset2_color_spaa=0.721569,0.572549,0.933333,1
preset2_color_gun_boat=0.768627,0.976471,0.705882,1
preset2_color_destroyer=0.964706,0.372549,0.388235,1
preset2_color_cruiser=0.768627,0.976471,0.705882,1
preset2_color_torpedo_boat=0.721569,0.572549,0.933333,1
preset2_monitor_index=0
)";

    inline std::filesystem::path get_config_path( )
    {
        char module_path[ MAX_PATH ] = {};
        const DWORD copied = ::GetModuleFileNameA( nullptr, module_path, MAX_PATH );
        if ( copied == 0 || copied == MAX_PATH )
            return std::filesystem::path( "wtc_menu.cfg" );

        std::filesystem::path exe_path( module_path );
        return exe_path.parent_path( ) / "wtc_menu.cfg";
    }

    inline std::string trim( std::string value )
    {
        const auto not_space = []( unsigned char c ) { return !std::isspace( c ); };
        value.erase( value.begin( ), std::find_if( value.begin( ), value.end( ), not_space ) );
        value.erase( std::find_if( value.rbegin( ), value.rend( ), not_space ).base( ), value.end( ) );
        return value;
    }

    inline bool parse_bool( const std::string& value, bool fallback )
    {
        if ( value == "1" || value == "true" || value == "TRUE" )
            return true;
        if ( value == "0" || value == "false" || value == "FALSE" )
            return false;
        return fallback;
    }

    inline float parse_float( const std::string& value, float fallback )
    {
        try
        {
            return std::stof( value );
        }
        catch ( ... )
        {
            return fallback;
        }
    }

    inline int parse_int( const std::string& value, int fallback )
    {
        try
        {
            return std::stoi( value );
        }
        catch ( ... )
        {
            return fallback;
        }
    }

    inline void apply_color( ImVec4& color, const std::string& value )
    {
        std::stringstream stream( value );
        float x = color.x;
        float y = color.y;
        float z = color.z;
        float w = color.w;
        char separator1 = ',';
        char separator2 = ',';
        char separator3 = ',';
        if ( stream >> x >> separator1 >> y >> separator2 >> z >> separator3 >> w )
            color = ImVec4( x, y, z, w );
    }

    inline std::string color_to_string( const ImVec4& color )
    {
        std::ostringstream stream;
        stream << color.x << ',' << color.y << ',' << color.z << ',' << color.w;
        return stream.str( );
    }

    struct bool_binding_t { const char* key; bool* value; };
    struct float_binding_t { const char* key; float* value; };
    struct int_binding_t { const char* key; int* value; int min_value; int max_value; };
    struct color_binding_t { const char* key; ImVec4* value; };

    inline const std::array<bool_binding_t, 25> bool_bindings = {{
        { "menu_open", &GUI::menu_open },
        { "esp_enabled", &sdk::esp_enabled },
        { "aimbot_enabled", &sdk::aimbot_enabled },
        { "esp_show_boxes", &sdk::esp_show_boxes },
        { "esp_use_3d_boxes", &sdk::esp_use_3d_boxes },
        { "esp_show_vehicle_names", &sdk::esp_show_vehicle_names },
        { "esp_show_distance", &sdk::esp_show_distance },
        { "esp_show_reload", &sdk::esp_show_reload },
        { "esp_show_reload_seconds", &sdk::esp_show_reload_seconds },
        { "esp_show_fps", &sdk::esp_show_fps },
        { "esp_vsync_enabled", &sdk::esp_vsync_enabled },
        { "auto_fullscreen_on_start", &sdk::auto_fullscreen_on_start },
        { "show_helicopter", &sdk::show_helicopter },
        { "show_bomber", &sdk::show_bomber },
        { "show_fighter", &sdk::show_fighter },
        { "show_attacker", &sdk::show_attacker },
        { "show_light_tank", &sdk::show_light_tank },
        { "show_heavy_tank", &sdk::show_heavy_tank },
        { "show_medium_tank", &sdk::show_medium_tank },
        { "show_spaa", &sdk::show_spaa },
        { "show_gun_boat", &sdk::show_gun_boat },
        { "show_destroyer", &sdk::show_destroyer },
        { "show_cruiser", &sdk::show_cruiser },
        { "show_torpedo_boat", &sdk::show_torpedo_boat },
        { "esp_show_bomb_impact", &sdk::esp_show_bomb_impact }
    }};

    inline const std::array<float_binding_t, 2> float_bindings = {{
        { "esp_render_distance", &sdk::esp_render_distance },
        { "esp_box_distance", &sdk::esp_box_distance }
    }};

    inline const std::array<int_binding_t, 2> int_bindings = {{
        { "monitor_index", &sdk::monitor_index, -100, 100 },
        { "esp_vsync_fps", &sdk::esp_vsync_fps, 30, 500 }
    }};

    inline const std::array<color_binding_t, 12> color_bindings = {{
        { "color_helicopter", &sdk::color_helicopter },
        { "color_bomber", &sdk::color_bomber },
        { "color_fighter", &sdk::color_fighter },
        { "color_attacker", &sdk::color_attacker },
        { "color_light_tank", &sdk::color_light_tank },
        { "color_heavy_tank", &sdk::color_heavy_tank },
        { "color_medium_tank", &sdk::color_medium_tank },
        { "color_spaa", &sdk::color_spaa },
        { "color_gun_boat", &sdk::color_gun_boat },
        { "color_destroyer", &sdk::color_destroyer },
        { "color_cruiser", &sdk::color_cruiser },
        { "color_torpedo_boat", &sdk::color_torpedo_boat }
    }};

    inline void load( )
    {
        const std::filesystem::path path = get_config_path( );
        std::ifstream input( path );
        if ( !input.is_open( ) )
        {
            std::ofstream output( path, std::ios::trunc );
            if ( output.is_open( ) )
            {
                output << default_config_content;
                output.close( );
                input.open( path );
            }
        }
        if ( !input.is_open( ) )
            return;

        std::unordered_map<std::string, std::string> values;
        std::string line;
        while ( std::getline( input, line ) )
        {
            if ( line.empty( ) || line[ 0 ] == '#' || line[ 0 ] == ';' )
                continue;

            const auto separator_pos = line.find( '=' );
            if ( separator_pos == std::string::npos )
                continue;

            const std::string key = trim( line.substr( 0, separator_pos ) );
            const std::string value = trim( line.substr( separator_pos + 1 ) );
            if ( !key.empty( ) )
                values[ key ] = value;
        }

        for ( const auto& binding : bool_bindings )
        {
            const auto it = values.find( binding.key );
            if ( it != values.end( ) )
                *binding.value = parse_bool( it->second, *binding.value );
        }

        for ( const auto& binding : float_bindings )
        {
            const auto it = values.find( binding.key );
            if ( it != values.end( ) )
                *binding.value = parse_float( it->second, *binding.value );
        }

        for ( const auto& binding : int_bindings )
        {
            const auto it = values.find( binding.key );
            if ( it == values.end( ) )
                continue;

            const int parsed = parse_int( it->second, *binding.value );
            const int clamped = std::clamp( parsed, binding.min_value, binding.max_value );
            *binding.value = clamped;
        }

        if ( const auto it = values.find( "entity_update_interval_ms" ); it != values.end( ) )
            core::entity_update_interval_ms.store( std::clamp( parse_int( it->second, core::entity_update_interval_ms.load( ) ), 1, 200 ) );

        if ( const auto it = values.find( "frame_update_interval_ms" ); it != values.end( ) )
            core::frame_update_interval_ms.store( std::clamp( parse_int( it->second, core::frame_update_interval_ms.load( ) ), 1, 200 ) );

        for ( const auto& binding : color_bindings )
        {
            const auto it = values.find( binding.key );
            if ( it != values.end( ) )
                apply_color( *binding.value, it->second );
        }

        GUI::load_presets_from_map( values );
    }

    inline bool save( )
    {
        const std::filesystem::path path = get_config_path( );
        std::ofstream output( path, std::ios::trunc );
        if ( !output.is_open( ) )
            return false;

        for ( const auto& binding : bool_bindings )
            output << binding.key << '=' << ( *binding.value ? 1 : 0 ) << '\n';

        for ( const auto& binding : float_bindings )
            output << binding.key << '=' << *binding.value << '\n';

        output << "entity_update_interval_ms=" << std::clamp( core::entity_update_interval_ms.load( ), 1, 200 ) << '\n';
        output << "frame_update_interval_ms=" << std::clamp( core::frame_update_interval_ms.load( ), 1, 200 ) << '\n';
        output << "monitor_index=" << sdk::monitor_index << '\n';

        for ( const auto& binding : color_bindings )
            output << binding.key << '=' << color_to_string( *binding.value ) << '\n';

        GUI::append_presets_to_stream( output );

        return output.good( );
    }

}

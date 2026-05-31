#pragma once

#include <ostream>
#include <string>
#include <unordered_map>

namespace GUI
{
    extern bool menu_open;
    void OnRender();

    void load_presets_from_map(const std::unordered_map<std::string, std::string>& values);
    void append_presets_to_stream(std::ostream& output);
}

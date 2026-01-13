//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vstgui/lib/cstring.h>

namespace MinMax
{
    const struct Files
    {
        inline static const char* STR_USERPROFILE = "USERPROFILE";
        inline static const char* PRESET_ROOT = "Documents/VST3 Presets/MinMax/QBDrummer/Maps";
        inline static const VSTGUI::UTF8String FILE_EXT = "csv";

        inline static std::filesystem::path getPresetPath()
        {
            return std::filesystem::path(getenv(STR_USERPROFILE)).append(PRESET_ROOT).make_preferred();
        }

        inline static void createPresetDirectory()
        {
            std::string p = getPresetPath().string();

            if (!std::filesystem::exists(getPresetPath().string()))
                std::filesystem::create_directories(p);
        }
    };
}
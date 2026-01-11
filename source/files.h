//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <cstdlib>
#include <filesystem>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <string>
#include <system_error>
#include <vector>
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

        inline static tresult getPresetFiles(std::vector<std::string>& file_names)
        {
            createPresetDirectory();
            std::filesystem::directory_iterator iter(getPresetPath()), end;
            std::error_code err;

            for (; iter != end && !err; iter.increment(err))
            {
                const std::filesystem::directory_entry entry = *iter;
                if (std::filesystem::path(entry.path().string()).extension() != ".csv") continue;
                file_names.push_back(entry.path().string());
            }

            return kResultTrue;
        }
    };
}
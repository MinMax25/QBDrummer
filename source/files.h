//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <filesystem>
#include <codecvt>
#include <sstream>
#include <fstream>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <vstgui/vstgui_uidescription.h>
#include <vstgui/plugin-bindings/vst3editor.h>
#include <vstgui/uidescription/detail/uiviewcreatorattributes.h>

#include "plugdefine.h"

namespace MinMax
{
    using namespace VSTGUI;

    namespace fs = std::filesystem;

    const struct Files
    {
        inline static const char* STR_USERPROFILE = "USERPROFILE";
        inline static const char* PRESET_ROOT = "Documents/VST3 Presets/MinMax/QBDrummer/Maps";
        inline static const UTF8String FILE_EXT = "csv";

        inline static fs::path getPresetPath()
        {
            return fs::path(getenv(STR_USERPROFILE)).append(PRESET_ROOT).make_preferred();
        }

        inline static void createPresetDirectory()
        {
            std::string p = getPresetPath().string();

            if (!fs::exists(getPresetPath().string()))
                fs::create_directories(p);
        }

        inline static tresult getPresetFiles(std::vector<std::string>& file_names)
        {
            createPresetDirectory();
            fs::directory_iterator iter(getPresetPath()), end;
            std::error_code err;

            for (; iter != end && !err; iter.increment(err))
            {
                const fs::directory_entry entry = *iter;
                if (fs::path(entry.path().string()).extension() != ".csv") continue;
                file_names.push_back(entry.path().string());
            }

            return kResultTrue;
        }
    };
}
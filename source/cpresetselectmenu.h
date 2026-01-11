//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vstgui/lib/cbitmap.h>
#include <vstgui/lib/controls/ccontrol.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/controls/icontrollistener.h>
#include <vstgui/lib/crect.h>

#include "files.h"
#include "plugdefine.h"

namespace MinMax
{
    class CPresetSelectMenu
        : public VSTGUI::COptionMenu
    {
    public:
        CPresetSelectMenu(
            std::function<void(VSTGUI::CControl*)> _onSelectChanged,
            PRESETNAME name,
            const VSTGUI::CRect& size, VSTGUI::IControlListener* listener, int32_t tag, VSTGUI::CBitmap* background = nullptr, VSTGUI::CBitmap* bgWhenClick = nullptr, const int32_t style = 0)
            : COptionMenu(size, listener, tag, background, bgWhenClick, style)
        {
            onSelectChanged = _onSelectChanged;
            setStyle(VSTGUI::COptionMenu::kCheckStyle);
            getPresetList();
            setCurrentEntry(name);
        }

        ~CPresetSelectMenu()
        {
            onSelectChanged = nullptr;
        }

        void valueChanged() override
        {
            if (onSelectChanged) onSelectChanged(this);
            VSTGUI::COptionMenu::valueChanged();
        }

        void getPresetList()
        {
            removeAllEntry();

            addEntry(u8"Default");
            setCurrent(0);

            auto root = Files::getPresetPath();
            addDirectoryToMenu(this, root, root);
        }

        void setCurrentEntry(PRESETNAME name)
        {
            auto presetName = std::string(name);
            for (int i = 0; i < (int)getItems()->size(); i++)
            {
                if (getEntry(i)->getTitle() == presetName)
                {
                    setCurrent(i);
                    break;
                }
            }
        }

    protected:
        std::function<void(VSTGUI::CControl* pControl)> onSelectChanged;

        void addDirectoryToMenu(VSTGUI::COptionMenu* menu, const std::filesystem::path& dir, const std::filesystem::path& root)
        {
            for (auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    auto subMenu = new VSTGUI::COptionMenu();
                    subMenu->setStyle(VSTGUI::COptionMenu::kCheckStyle);
                    addDirectoryToMenu(subMenu, entry.path(), root);
                    menu->addEntry(subMenu, entry.path().filename().u8string());
                    subMenu->forget();
                }
                else if (entry.path().extension() == ".csv")
                {
                    auto rel = std::filesystem::relative(entry.path(), root);
                    auto item = menu->addEntry(entry.path().stem().u8string());
                    item->setTag(0);
                    item->setKey(std::filesystem::path(entry.path()).u8string());
                    item->setTitle(std::filesystem::path(entry.path()).stem().u8string());
                }
            }
        }
    };
}
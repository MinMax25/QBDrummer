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
    using namespace VSTGUI;

    namespace fs = std::filesystem;

    class CPresetSelectMenu
        : public COptionMenu
    {
    public:
        CPresetSelectMenu(
            std::function<void(CControl*)> _onSelectChanged,
            PRESETNAME name,
            const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background = nullptr, CBitmap* bgWhenClick = nullptr, const int32_t style = 0)
            : COptionMenu(size, listener, tag, background, bgWhenClick, style)
        {
            onSelectChanged = _onSelectChanged;
            setStyle(COptionMenu::kCheckStyle);
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
            COptionMenu::valueChanged();
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
        std::function<void(CControl* pControl)> onSelectChanged;

        void addDirectoryToMenu(COptionMenu* menu, const fs::path& dir, const fs::path& root)
        {
            for (auto& entry : fs::directory_iterator(dir))
            {
                if (entry.is_directory())
                {
                    auto subMenu = new COptionMenu();
                    subMenu->setStyle(COptionMenu::kCheckStyle);
                    addDirectoryToMenu(subMenu, entry.path(), root);
                    menu->addEntry(subMenu, entry.path().filename().u8string());
                    subMenu->forget();
                }
                else if (entry.path().extension() == ".csv")
                {
                    auto rel = fs::relative(entry.path(), root);
                    auto item = menu->addEntry(entry.path().stem().u8string());
                    item->setTag(0);
                    item->setKey(fs::path(entry.path()).u8string());
                    item->setTitle(fs::path(entry.path()).stem().u8string());
                }
            }
        }
    };
}
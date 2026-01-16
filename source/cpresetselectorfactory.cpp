//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/smartpointer.h>
#include <sstream>
#include <string>
#include <string.h>
#include <thread>
#include <vector>
#include <vstgui/lib/ccolor.h>
#include <vstgui/lib/controls/ccontrol.h>
#include <vstgui/lib/controls/coptionmenu.h>
#include <vstgui/lib/cpoint.h>
#include <vstgui/lib/crect.h>
#include <vstgui/lib/cstring.h>
#include <vstgui/lib/cview.h>
#include <vstgui/lib/cviewcontainer.h>
#include <vstgui/lib/vstguibase.h>
#include <vstgui/plugin-bindings/vst3editor.h>
#include <vstgui/uidescription/detail/uiviewcreatorattributes.h>
#include <vstgui/uidescription/iuidescription.h>
#include <vstgui/uidescription/iviewcreator.h>
#include <vstgui/uidescription/uiattributes.h>
#include <vstgui/uidescription/uiviewfactory.h>

#include "cmenubutton.h"
#include "cselectmenu.h"
#include "files.h"
#include "plugdefine.h"

namespace MinMax
{
    class CPresetSelector
        : public VSTGUI::CViewContainer
    {
    public:
        CPresetSelector(const VSTGUI::UIAttributes& attributes, const VSTGUI::IUIDescription* description, const VSTGUI::CRect& size)
            : CViewContainer(size)
        {
            Files::createPresetDirectory();
            
            editor = static_cast<VSTGUI::VST3Editor*>(description->getController());

            map = std::stoi(attributes.getAttributeValue("custom-view-name")->c_str());

            setBackgroundColor(VSTGUI::kTransparentCColor);
            setViewSize(VSTGUI::CRect(0, 0, 420, 20));

            PRESETNAME pname{};
            std::thread th(getPresetName, map, pname);
            th.join();

            menubutton = 
                new CMenuButton(
                    VSTGUI::CRect(VSTGUI::CPoint(0, 0), VSTGUI::CPoint(360, 20)),
                    pname,
                    [this](VSTGUI::CControl* pControl) { popupMenu(pControl); }
                );
            addView(menubutton);
        }

        ~CPresetSelector()
        {
        }

        CLASS_METHODS(CPresetSelector, CViewContainer)

    protected:
        VSTGUI::VST3Editor* editor{};

        int16 map = 0;

        CMenuButton* menubutton = nullptr;

        static void getPresetName(int16 map, PRESETNAME pname)
        {
            sem.wait();

            if (map == 1)
                strcpy(pname, PSET1.Name);
            else
                strcpy(pname, PSET2.Name);

            sem.notify();
        }

        void popupMenu(VSTGUI::CControl* pControl)
        {
            auto* menu = createOpenPresetMenu();

            VSTGUI::CPoint p(pControl->getViewSize().left, pControl->getViewSize().bottom);
            pControl->localToFrame(p);

            menu->popup(getFrame(), p);
            menu->forget();
        }

        CSelectMenu* createOpenPresetMenu()
        {
            Files::createPresetDirectory();

            auto* menu =
                new CSelectMenu(
                    [this](VSTGUI::CControl* pControl, VSTGUI::UTF8String path) { onPresetSelectChanged(path); }
                );

            VSTGUI::COptionMenu* dirMenu = nullptr;
            VSTGUI::CMenuItem* dirItem = nullptr;
            int itemCount = 0;

            for (const std::filesystem::directory_entry& folder : std::filesystem::directory_iterator(Files::getPresetPath()))
            {
                if (!folder.is_directory()) continue;
                if (std::filesystem::is_empty(folder.path())) continue;

                if (itemCount > 0)
                {
                    dirItem->setSubmenu(dirMenu);
                    menu->addEntry(dirItem);
                    dirMenu->forget();
                }

                dirItem = new VSTGUI::CMenuItem(folder.path().stem().u8string().c_str());
                dirMenu = new VSTGUI::COptionMenu();

                for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(folder.path()))
                {
                    if (file.path().extension() != ".csv") continue;

                    auto* fileItem = new VSTGUI::CMenuItem(file.path().stem().u8string().c_str(), -1);
                    fileItem->setKey(file.path().u8string());
                    dirMenu->addEntry(fileItem);
                    itemCount++;

                }
            }

            if (itemCount > 0)
            {
                dirItem->setSubmenu(dirMenu);
                menu->addEntry(dirItem);
                dirMenu->forget();
            }

            return menu;
        }

        void onPresetSelectChanged(VSTGUI::UTF8String presetPath)
        {
            std::filesystem::path p(presetPath.getString());
            auto& name = p.stem().string();

            menubutton->setTitle(name.c_str());

            Preset preset{};
            preset.Map = map;
            name.copy(preset.Name, sizeof(preset.Name));

            int count = 0;

            std::ifstream file(presetPath);
            std::string line;

            while (std::getline(file, line))
            {
                std::vector<std::string> row;
                split(line, ',', row);

                if (row.size() == 4 && isNumber(row[0]) && isNumber(row[1]))
                {
                    int id = std::atoi(row[0].c_str());
                    int sb = std::atoi(row[1].c_str());

                    if (sb == 0)
                    {
                        preset.data[count++] = (id & 0x0fff) | 0x8000;
                    }
                    else
                    {
                        preset.data[count++] = (id & 0x0fff) | 0xc000;
                    }

                    if (count >= PRESET_SIZE) goto L1000;

                    std::vector<std::string> pitchs;
                    split(row[3], '|', pitchs);

                    for each (auto p in pitchs)
                    {
                        if (isNumber(p)) preset.data[count++] = std::atoi(p.c_str());
                        if (count >= PRESET_SIZE) goto L1000;
                    }
                }
            }

        L1000:
            if (auto message = Steinberg::owned(editor->getController()->allocateMessage()))
            {
                message->setMessageID(MsgPreset);
                if (auto attr = message->getAttributes())
                {
                    attr->setBinary(MsgPreset, &preset, sizeof(Preset));
                }
                if (editor->getController() == nullptr) return;
                editor->getController()->getPeer()->notify(message);
            }
        }

        void split(std::string const& str, const char delim, std::vector<std::string>& out)
        {
            std::stringstream ss(str);
            std::string s;
            while (std::getline(ss, s, delim)) out.push_back(s);
        }

        bool isNumber(const std::string& str)
        {
            for (char const& c : str) if (std::isdigit(static_cast<unsigned char>(c)) == 0) return false;
            return true;
        }
    };

    class CPresetSelectorFactory
        : public VSTGUI::ViewCreatorAdapter
    {
    public:

        CPresetSelectorFactory() { VSTGUI::UIViewFactory::registerViewCreator(*this); }

        VSTGUI::IdStringPtr getViewName() const override { return "UI:Preset"; }

        VSTGUI::IdStringPtr getBaseViewName() const override { return VSTGUI::UIViewCreator::kCViewContainer; }

        VSTGUI::CView* create(const VSTGUI::UIAttributes& attributes, const VSTGUI::IUIDescription* description) const override
        {
            return new CPresetSelector(attributes, description, VSTGUI::CRect(0, 0, 100, 100));
        }
    };

    CPresetSelectorFactory __gCPresetSelectorFactory;
}
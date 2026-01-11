//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma warning(disable : 4996)

#include <filesystem>
#include <codecvt>
#include <sstream>
#include <fstream>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <vstgui/vstgui_uidescription.h>
#include <vstgui/plugin-bindings/vst3editor.h>
#include <vstgui/uidescription/detail/uiviewcreatorattributes.h>

#include "plugdefine.h"
#include "files.h"
#include "cpresetselectmenu.h"
#include "cmenubutton.h"

namespace MinMax
{
    class CPresetSelector
        : public VSTGUI::CViewContainer
    {
    public:
        CPresetSelector(const VSTGUI::UIAttributes& attributes, const VSTGUI::IUIDescription* description, const VSTGUI::CRect& size)
            : CViewContainer(size)
        {
            editor = static_cast<VSTGUI::VST3Editor*>(description->getController());

            map = std::stoi(attributes.getAttributeValue("custom-view-name")->c_str());

            setBackgroundColor(VSTGUI::kTransparentCColor);
            setViewSize(VSTGUI::CRect(0, 0, 420, 20));

            PRESETNAME pname{};
            std::thread th(getPresetName, map, pname);
            th.join();

            menubutton = new CMenuButton(VSTGUI::CRect(VSTGUI::CPoint(0, 0), VSTGUI::CPoint(360, 20)), pname, nullptr);
            addView(menubutton);

            //optTarget =
            //    new CPresetSelectMenu(
            //        [this](VSTGUI::CControl* pControl) { onPresetSelectChanged(pControl); },
            //        pname,
            //        VSTGUI::CRect(VSTGUI::CPoint(0, 0), VSTGUI::CPoint(360, 20)),
            //        description->getController(),
            //        -1
            //    );
            //optTarget->setBackColor(VSTGUI::kWhiteCColor);
            //optTarget->setFontColor(VSTGUI::kBlackCColor);
            //addView(optTarget);
        }

        ~CPresetSelector()
        {
        }

        CLASS_METHODS(CPresetSelector, CViewContainer)

    protected:
        VSTGUI::VST3Editor* editor{};

        int16 map = 0;

        CMenuButton* menubutton = nullptr;

        CPresetSelectMenu* optTarget = nullptr;

        VSTGUI::UTF8StringPtr filename = nullptr;

        static inline std::wstring convertUtf8ToUtf16(char const* str)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            return converter.from_bytes(str);
        }

        static void getPresetName(int16 map, PRESETNAME pname)
        {
            sem.wait();

            if (map == 1)
                strcpy(pname, PSET1.Name);
            else
                strcpy(pname, PSET2.Name);

            sem.notify();
        }

        void onPresetSelectChanged(VSTGUI::CControl* pControl)
        {
            auto* top = static_cast<VSTGUI::COptionMenu*>(pControl);

            VSTGUI::UTF8String fullPath;

            int32_t idx = -1;
            if (auto* menu = top->getLastItemMenu(idx))
            {
                if (auto* item = menu->getEntry(idx))
                {
                    fullPath = item->getKeycode();
                }
            }
            
            if (fullPath.empty()) return;

            std::filesystem::path fp = std::filesystem::path(fullPath.getString());

            auto& name = VSTGUI::UTF8String(fp.stem().u8string());

            Preset preset{};
            preset.Map = map;
            name.copy(preset.Name, sizeof(preset.Name));

            int count = 0;

            if (optTarget->getCurrentIndex() > 0)
            {
                std::filesystem::path path = Files::getPresetPath().append(convertUtf8ToUtf16(name)).replace_extension(Files::FILE_EXT.getString());
                std::ifstream file(path);
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
                            preset.data[count++] = (id & 0x0fff) | 0x8000;
                        else
                            preset.data[count++] = (id & 0x0fff) | 0xc000;

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
            }
        L1000:
            if (auto message = Steinberg::owned(editor->getController()->allocateMessage()))
            {
                message->setMessageID(MsgPreset);
                if (auto attr = message->getAttributes())
                    attr->setBinary(MsgPreset, &preset, sizeof(Preset));
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
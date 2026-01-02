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

namespace MinMax
{
    using namespace VSTGUI;

    namespace fs = std::filesystem;

    const struct Files
    {
        inline static const char* STR_USERPROFILE = "USERPROFILE";
        inline static const char* PRESET_ROOT = "Documents/VST3 Presets/MinMax/QB Drummer/Maps";
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

            std::vector<std::string> filenames;
            Files::getPresetFiles(filenames);
            for (auto& item : filenames) addEntry(fs::path(item).stem().u8string());
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
    };

    class CPresetSelector
        : public CViewContainer
    {
    public:
        CPresetSelector(const UIAttributes& attributes, const IUIDescription* description, const CRect& size)
            : CViewContainer(size)
        {
            editor = static_cast<VST3Editor*>(description->getController());

            map = std::stoi(attributes.getAttributeValue("custom-view-name")->c_str());

            setBackgroundColor(kTransparentCColor);
            setViewSize(CRect(0, 0, 420, 20));

            PRESETNAME pname{};
            std::thread th(getPresetName, map, pname);
            th.join();

            optTarget =
                new CPresetSelectMenu(
                    [this](CControl* pControl) { onPresetSelectChanged(pControl); },
                    pname,
                    CRect(CPoint(0, 0), CPoint(360, 20)),
                    description->getController(),
                    -1
                );
            optTarget->setBackColor(kWhiteCColor);
            optTarget->setFontColor(kBlackCColor);
            addView(optTarget);
        }

        ~CPresetSelector()
        {
        }

        CLASS_METHODS(CPresetSelector, CViewContainer)

    protected:
        VST3Editor* editor{};

        int16 map = 0;

        CPresetSelectMenu* optTarget = nullptr;

        UTF8StringPtr filename = nullptr;

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

        void onPresetSelectChanged(CControl* pControl)
        {
            auto& name = optTarget->getEntry(optTarget->getCurrentIndex())->getTitle();

            Preset preset{};
            preset.Map = map;
            name.copy(preset.Name, sizeof(preset.Name));

            int count = 0;

            if (optTarget->getCurrentIndex() > 0)
            {
                fs::path path = Files::getPresetPath().append(convertUtf8ToUtf16(name)).replace_extension(Files::FILE_EXT.getString());
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
        : public ViewCreatorAdapter
    {
    public:

        CPresetSelectorFactory() { UIViewFactory::registerViewCreator(*this); }

        IdStringPtr getViewName() const override { return "UI:Preset"; }

        IdStringPtr getBaseViewName() const override { return UIViewCreator::kCViewContainer; }

        CView* create(const UIAttributes& attributes, const IUIDescription* description) const override
        {
            return new CPresetSelector(attributes, description, CRect(0, 0, 100, 100));
        }
    };

    CPresetSelectorFactory __gCPresetSelectorFactory;
}
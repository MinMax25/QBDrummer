//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <functional>
#include <vstgui/lib/cstring.h>
#include <vstgui/lib/controls/coptionmenu.h>

namespace MinMax
{
    class CSelectMenu
        : public VSTGUI::COptionMenu
    {
    public:
        using onClickCallBack = std::function<void(CSelectMenu*, VSTGUI::UTF8String)>;

        CSelectMenu(onClickCallBack cb)
            : VSTGUI::COptionMenu()
            , onClick(cb)
        {
        }

        void valueChanged() override
        {
            if (!lastMenu) return;
            if (auto value = lastMenu->getCurrent())
            {
                if (onClick) onClick(this, value->getKeycode());
            }
        }

    protected:
        onClickCallBack onClick;
    };
}

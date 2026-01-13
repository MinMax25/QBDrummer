//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <functional>
#include <vstgui/lib/cbuttonstate.h>
#include <vstgui/lib/ccolor.h>
#include <vstgui/lib/cdrawcontext.h>
#include <vstgui/lib/cdrawdefs.h>
#include <vstgui/lib/controls/cbuttons.h>
#include <vstgui/lib/cpoint.h>
#include <vstgui/lib/crect.h>
#include <vstgui/lib/cstring.h>
#include <vstgui/lib/vstguifwd.h>

namespace MinMax
{
    // メニュー表示ボタン
    class CMenuButton
        : public VSTGUI::CTextButton
    {
    public:
        using onClickCallBack = std::function<void(CMenuButton*)>;

        const VSTGUI::CColor NORMAL_TEXT_COLOR = VSTGUI::kGreyCColor;
        const VSTGUI::CColor EDIT_TEXT_COLOR = VSTGUI::kCyanCColor;

        CMenuButton(const VSTGUI::CRect& size, const VSTGUI::UTF8String& title, onClickCallBack cb)
            : VSTGUI::CTextButton(size, nullptr, -1, title)
            , onClick(cb)
        {
            setRoundRadius(0);
        }

        VSTGUI::CMouseEventResult onMouseEntered(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override
        {
            mouseInside = true;
            invalid();
            return VSTGUI::kMouseEventHandled;
        }

        VSTGUI::CMouseEventResult onMouseExited(VSTGUI::CPoint& where, const VSTGUI::CButtonState& buttons) override
        {
            mouseInside = false;
            invalid();
            return VSTGUI::kMouseEventHandled;
        }

        void draw(VSTGUI::CDrawContext* ctx) override
        {
            if (mouseInside)
            {
                ctx->setFillColor(VSTGUI::kWhiteCColor);
                ctx->drawRect(getViewSize(), VSTGUI::kDrawFilled);
            }

            VSTGUI::CTextButton::draw(ctx);
        }

        void valueChanged() override
        {
            if (getValue()) return;
            if (onClick) onClick(this);
        }

    private:
        bool mouseInside{ false };

        onClickCallBack onClick;
    };
}

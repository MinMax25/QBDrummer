//------------------------------------------------------------------------
// Copyright(c) 2025 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <cstdarg>
#include <cstdio>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace MinMax
{

    //======================================================================
    // デバッグ出力ラッパー
    //======================================================================
    inline void DLogWrite(const char* fmt, ...)
    {
#if defined(_WIN32)
        va_list args;
        va_start(args, fmt);

        std::vector<char> buf(512);
        int n = vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
        if (n < 0 || n >= static_cast<int>(buf.size()))
        {
            buf.resize(n + 1);
            vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
        }

        va_end(args);
        OutputDebugStringA(buf.data());
#else
        (void)fmt;
#endif
    }

    inline void DLogWriteLine(const char* fmt, ...)
    {
#if defined(_WIN32)
        va_list args;
        va_start(args, fmt);

        std::vector<char> buf(512);
        int n = vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
        if (n < 0 || n >= static_cast<int>(buf.size()))
        {
            buf.resize(n + 1);
            vsnprintf_s(buf.data(), buf.size(), _TRUNCATE, fmt, args);
        }

        va_end(args);
        OutputDebugStringA(buf.data());
        OutputDebugStringA("\n");
#else
        (void)fmt;
#endif
    }
}

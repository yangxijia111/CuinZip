#pragma once

#include "StatusBar.g.h"
#include "ControlMacros.h"

namespace winrt::NanaZip::Modern::implementation
{
    // CuinZip P1-2: 状态栏信息层级重整。
    // TextPrimary — 左侧主信息:"128 items · 245 MB"(无选择)或
    //               "3 selected · 16.4 MB"(有选择),由 Win32 面板侧拼装。
    // TextArchive — 右侧压缩包信息(当前打开的压缩包名),可为空。
    struct StatusBar : StatusBarT<StatusBar>
    {
        StatusBar() = default;

        DEPENDENCY_PROPERTY_HEADER(TextPrimary, winrt::hstring);
        DEPENDENCY_PROPERTY_HEADER(TextArchive, winrt::hstring);
    };
}

namespace winrt::NanaZip::Modern::factory_implementation
{
    struct StatusBar : StatusBarT<StatusBar, implementation::StatusBar>
    { };
}

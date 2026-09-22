#pragma once

// CuinZip P1-2: 现代界面字符串辅助。
// 从 NanaZip.Modern 的 resw(PRI)加载本地化字符串;
// 找不到资源或 PRI 不可用时回退到调用方提供的英文兜底文本,
// 其余语言允许暂时回退 English(见 Docs/CUINZIP_UI_PLAN.md P1-2)。

#include <winrt/Windows.ApplicationModel.Resources.Core.h>

#include <map>
#include <mutex>
#include <string>

namespace winrt::NanaZip::Modern
{
    // 从 "NanaZip.Modern/Common" 资源子树取字符串,失败时返回 fallback。
    winrt::hstring GetUiString(std::wstring_view name, std::wstring_view fallback);
}

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

    // 清空 GetUiString 的进程内缓存(语言切换后必须调用,否则旧语言
    // 字符串一直命中缓存)。
    void ClearUiStringCache();

    // 设置资源解析语言覆盖(空 = 默认 en 候选;如 "zh-Hans")。
    // P1-8:不使用 ResourceContext(其 view-independent 形态曾引发
    // FM 主窗口线程崩溃),语言经候选手选实现。
    void SetUiResourceLanguage(std::wstring_view language);
    std::wstring GetUiResourceLanguage();

    // Save only an explicit Settings choice through the file-manager host.
    // Test hosts and startup language replay never write user configuration.
    void PersistUiLanguagePreference(std::wstring_view language);

    // 在资源子树里按语言手选候选(PickCandidate 实现,供 Legacy
    // 字符串解析共用;匹配不到回退 en / 无限定符默认候选)。
    winrt::hstring PickResourceCandidate(
        winrt::Windows::ApplicationModel::Resources::Core::ResourceMap const& subtree,
        std::wstring_view name,
        std::wstring_view language);
}

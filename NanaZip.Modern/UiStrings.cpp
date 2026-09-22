// CuinZip P1-2: 现代界面字符串辅助的实现。

#include "pch.h"
#include "UiStrings.h"

namespace
{
    // 与 K7ModernGetLegacyStringResource 相同的缓存模式,
    // 避免 XAML 控件在每次模板应用时反复跨 ABI 查询资源。
    std::mutex g_CachedUiStringsMutex;
    std::map<std::wstring, winrt::hstring> g_CachedUiStrings;
}

namespace winrt::NanaZip::Modern
{
    winrt::hstring GetUiString(
        std::wstring_view name,
        std::wstring_view fallback)
    {
        {
            std::lock_guard Lock(g_CachedUiStringsMutex);
            auto Iterator = g_CachedUiStrings.find(std::wstring(name));
            if (g_CachedUiStrings.end() != Iterator)
            {
                return Iterator->second;
            }
        }

        winrt::hstring Result;

        try
        {
            // 键支持两种形态(与 x:Uid 的 "/NanaZip.Modern/<file>/<key>" 寻址一致):
            //   "Key"                  → "NanaZip.Modern/Common" 子树
            //   "File/Key"(如 SettingsPage/xxx)→ "NanaZip.Modern/File" 子树
            std::wstring SubtreeName = L"NanaZip.Modern/Common";
            std::wstring_view KeyName = name;

            const size_t SlashPos = name.find(L'/');
            if (SlashPos != std::wstring_view::npos && SlashPos > 0)
            {
                SubtreeName = L"NanaZip.Modern/";
                SubtreeName.append(name.substr(0, SlashPos));
                KeyName = name.substr(SlashPos + 1);
            }

            winrt::Windows::ApplicationModel::Resources::Core::ResourceMap
                Subtree{ nullptr };

            try
            {
                Subtree = winrt::Windows::ApplicationModel::Resources::Core
                    ::ResourceManager::Current()
                    .MainResourceMap().GetSubtree(SubtreeName);
            }
            catch (...)
            {
                // PRI 不可用(unpackaged 环境等)时直接走英文兜底。
                Subtree = nullptr;
            }

            if (Subtree)
            {
                winrt::hstring ResourceName(KeyName);
                if (Subtree.HasKey(ResourceName))
                {
                    Result = Subtree.Lookup(ResourceName)
                        .Candidates().GetAt(0).ValueAsString();
                }
            }
        }
        catch (...)
        {
            Result = winrt::hstring{};
        }

        if (Result.empty())
        {
            Result = winrt::hstring(fallback);
        }

        {
            std::lock_guard Lock(g_CachedUiStringsMutex);
            g_CachedUiStrings.emplace(std::wstring(name), Result);
        }

        return Result;
    }
}

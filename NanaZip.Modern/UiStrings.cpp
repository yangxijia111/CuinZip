// CuinZip P1-2: 现代界面字符串辅助的实现。

#include "pch.h"
#include "UiStrings.h"

#include <winrt/Windows.ApplicationModel.Resources.Core.h>
#include <winrt/Windows.Globalization.h>

#include <algorithm>

namespace
{
    // 与 K7ModernGetLegacyStringResource 相同的缓存模式,
    // 避免 XAML 控件在每次模板应用时反复跨 ABI 查询资源。
    std::mutex g_CachedUiStringsMutex;
    std::map<std::wstring, winrt::hstring> g_CachedUiStrings;

    // P1-8:资源语言覆盖(空 = 跟随系统)。资源解析不使用
    // ResourceContext(P1-6.2 引入的 GetValue + view-independent context
    // 在 FM 主窗口线程触发过悬空 IMap 调用导致 0xC000041D 崩溃,
    // 见 Docs/CUINZIP_UI_PLAN.md P1-8 节),改为候选手选:
    // 按本覆盖值匹配候选项的 language 限定符。
    std::wstring g_ResourceLanguage;
    std::mutex g_ResourceLanguageMutex;

    // 语言标签近似匹配:zh-Hans 与 zh-CN/Hans 视为兼容(比较首个
    // 子标签 + 脚本;资源限定符常见值 en-US / zh-Hans / zh-Hant)。
    bool LanguageMatches(
        std::wstring const& wanted,
        std::wstring const& candidate)
    {
        if (wanted.empty() || candidate.empty())
        {
            return false;
        }
        auto lower = [](std::wstring const& text)
        {
            std::wstring result;
            result.reserve(text.size());
            for (wchar_t ch : text)
            {
                result.push_back(static_cast<wchar_t>(::towlower(ch)));
            }
            return result;
        };
        std::wstring w = lower(wanted);
        std::wstring c = lower(candidate);
        if (w == c)
        {
            return true;
        }
        auto script = [](std::wstring const& tag)
        {
            if (tag == L"zh-cn" || tag == L"zh-sg" || tag.find(L"zh-hans") == 0)
                return std::wstring(L"zh-hans");
            if (tag == L"zh-tw" || tag == L"zh-hk" || tag == L"zh-mo" ||
                tag.find(L"zh-hant") == 0)
                return std::wstring(L"zh-hant");
            return tag.substr(0, tag.find(L'-'));
        };
        return script(w) == script(c);
    }

    // 在候选列表里按覆盖语言手选;匹配不到时回退默认(en)候选。
    winrt::hstring PickCandidate(
        winrt::Windows::ApplicationModel::Resources::Core::
            ResourceMap const& subtree,
        std::wstring const& name,
        std::wstring const& language)
    {
        auto candidates = subtree.Lookup(
            winrt::hstring(name)).Candidates();
        if (candidates.Size() == 0)
        {
            return winrt::hstring{};
        }

        winrt::hstring fallback;
        for (uint32_t i = 0; i < candidates.Size(); ++i)
        {
            auto candidate = candidates.GetAt(i);
            std::wstring languageQualifier;
            for (auto const& qualifier : candidate.Qualifiers())
            {
                if (0 == ::_wcsicmp(qualifier.QualifierName().c_str(), L"language"))
                {
                    languageQualifier = std::wstring(
                        qualifier.QualifierValue());
                    break;
                }
            }
            if (!languageQualifier.empty())
            {
                if (LanguageMatches(language, languageQualifier))
                {
                    return candidate.ValueAsString();
                }
                if (fallback.empty()
                    && LanguageMatches(L"en", languageQualifier))
                {
                    fallback = candidate.ValueAsString();
                }
            }
            else if (fallback.empty())
            {
                // 无语言限定符的候选是语言中性的默认值
                fallback = candidate.ValueAsString();
            }
        }
        if (!fallback.empty())
        {
            return fallback;
        }
        return candidates.GetAt(0).ValueAsString();
    }
}

namespace winrt::NanaZip::Modern
{
    winrt::hstring PickResourceCandidate(
        winrt::Windows::ApplicationModel::Resources::Core::ResourceMap const& subtree,
        std::wstring_view name,
        std::wstring_view language)
    {
        // 同编译单元内转发到匿名命名空间的手选实现(供 Legacy 共用)
        return PickCandidate(
            subtree, std::wstring(name), std::wstring(language));
    }

    void SetUiResourceLanguage(std::wstring_view language)
    {
        std::lock_guard Lock(g_ResourceLanguageMutex);
        g_ResourceLanguage = std::wstring(language);
    }

    std::wstring GetUiResourceLanguage()
    {
        std::lock_guard Lock(g_ResourceLanguageMutex);
        return g_ResourceLanguage;
    }

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
                // resw 的 "Name.Text" 键在 PRI 中是 子树 Name + 资源
                // Text(两级,斜杠分隔);用点号查询永远 miss(P1-8 实证,
                // 此前该路径全部走了 fallback,Modern resw 从未真正生效)
                std::wstring ResourcePath;
                ResourcePath.reserve(KeyName.size() + 1);
                for (wchar_t ch : KeyName)
                {
                    ResourcePath.push_back(
                        (ch == L'.') ? L'/' : ch);
                }

                winrt::hstring ResourceName(ResourcePath);
                if (Subtree.HasKey(ResourceName))
                {
                    // P1-8:候选手选(不用 ResourceContext,见文件头说明)
                    std::wstring language;
                    {
                        std::lock_guard Lock(g_ResourceLanguageMutex);
                        language = g_ResourceLanguage;
                    }
                    if (language.empty())
                    {
                        language = L"en";
                    }
                    Result = PickCandidate(Subtree, ResourcePath, language);
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

    void ClearUiStringCache()
    {
        std::lock_guard Lock(g_CachedUiStringsMutex);
        g_CachedUiStrings.clear();
    }
}

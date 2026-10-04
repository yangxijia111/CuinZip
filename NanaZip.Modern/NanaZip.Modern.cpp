/*
 * PROJECT:    NanaZip.Modern
 * FILE:       NanaZip.Modern.cpp
 * PURPOSE:    Implementation for NanaZip Modern Experience
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include "pch.h"

#include "NanaZip.Modern.h"

#include <Mile.Helpers.h>
#include <Mile.Xaml.h>

#include "App.h"
#include "SponsorPage.h"
#include "AboutPage.h"
#include "InformationPage.h"
#include "ProgressPage.h"
#include "CopyLocationPage.h"
#include "SettingsPage.h"
// **************** CuinZip P1-6 Modification Start ****************
#include "StartPage.h"
// **************** CuinZip P1-6 Modification End ****************
// **************** CuinZip P1-3 Modification Start ****************
#include "CompressDialogPage.h"
#include "ExtractDialogPage.h"
// **************** CuinZip P1-3 Modification End ****************
#include "UiStrings.h"

#pragma comment(lib, "comctl32.lib")

#include <winrt/Windows.ApplicationModel.Resources.Core.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>

#include <mutex>
#include <map>
#include <thread>
#include <vector>

namespace winrt
{
    using Windows::ApplicationModel::Resources::Core::ResourceManager;
    using Windows::ApplicationModel::Resources::Core::ResourceMap;
}

namespace
{
    static winrt::ResourceMap GetMainResourceMap()
    {
        static winrt::ResourceMap CachedResult = ([]() -> winrt::ResourceMap
        {
            try
            {
                return winrt::ResourceManager::Current().MainResourceMap();
            }
            catch (...)
            {
                // Do nothing.
            }
            return nullptr;
        }());

        return CachedResult;
    }

    static std::mutex g_CachedLanguageStringResourcesMutex;
    static std::map<UINT32, winrt::hstring> g_CachedLanguageStringResources;

    // CuinZip P1-8:当前语言覆盖值(空 = 跟随系统/英文默认)。
    // K7ModernSetAppLanguage 维护,Legacy/Modern 字符串解析共用
    // (声明前置供 K7ModernGetLegacyStringResource 使用)。
    static std::wstring g_AppLanguage;
}

EXTERN_C LPCWSTR WINAPI K7ModernGetLegacyStringResource(
    _In_ UINT32 ResourceId)
{
    {
        std::lock_guard Lock(g_CachedLanguageStringResourcesMutex);
        auto Iterator = g_CachedLanguageStringResources.find(ResourceId);
        if (g_CachedLanguageStringResources.end() != Iterator)
        {
            return Iterator->second.c_str();
        }
    }

    static winrt::ResourceMap LegacyResourceMap = ([]() -> winrt::ResourceMap
    {
        winrt::ResourceMap MainResourceMap = ::GetMainResourceMap();
        if (MainResourceMap)
        {
            return MainResourceMap.GetSubtree(L"Legacy");
        }
        return nullptr;
    }());
    if (!LegacyResourceMap)
    {
        return nullptr;
    }

    winrt::hstring ResourceName = L"Resource" + winrt::to_hstring(ResourceId);
    if (!LegacyResourceMap.HasKey(ResourceName))
    {
        return nullptr;
    }

    // P1-8:候选手选解析(P1-6.2 的 GetValue + view-independent context
    // 在 FM 主窗口线程引发过 0xC000041D 崩溃,弃用;语言取自本文件的
    // g_AppLanguage 覆盖值,由 K7ModernSetAppLanguage 维护)
    winrt::hstring Content = winrt::NanaZip::Modern::PickResourceCandidate(
        LegacyResourceMap,
        ResourceName,
        winrt::NanaZip::Modern::GetUiResourceLanguage());
    std::lock_guard Lock(g_CachedLanguageStringResourcesMutex);
    auto Iterator = g_CachedLanguageStringResources.emplace(
        ResourceId,
        std::move(Content));
    return Iterator.first->second.c_str();
}

namespace
{
    static winrt::NanaZip::Modern::App g_AppInstance = nullptr;
}

EXTERN_C BOOL WINAPI K7ModernAvailable()
{
    return nullptr != g_AppInstance;
}

EXTERN_C HRESULT WINAPI K7ModernInitialize()
{
    if (g_AppInstance)
    {
        return S_OK;
    }
    if (!::GetMainResourceMap())
    {
        // NanaZip.Modern requires resources.pri to get XAML resources.
        return E_NOINTERFACE;
    }
    try
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        using Implementation = winrt::NanaZip::Modern::implementation::App;
        g_AppInstance = winrt::make<Implementation>();
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
    return S_OK;
}

EXTERN_C HRESULT WINAPI K7ModernUninitialize()
{
    if (!g_AppInstance)
    {
        return S_OK;
    }
    try
    {
        g_AppInstance.Close();
        g_AppInstance = nullptr;
        winrt::uninit_apartment();
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
    return S_OK;
}

namespace winrt
{
    using Windows::UI::Xaml::Hosting::DesktopWindowXamlSource;
}

namespace
{
    HWND K7ModernCreateXamlWindow(
        _In_opt_ HWND ParentWindowHandle,
        _In_ DWORD ExtendedWindowStyle,
        _In_ DWORD WindowStyle)
    {
        HWND WindowHandle = ::CreateWindowExW(
            ExtendedWindowStyle,
            L"Mile.Xaml.ContentWindow",
            nullptr,
            WindowStyle,
            CW_USEDEFAULT,
            0,
            CW_USEDEFAULT,
            0,
            ParentWindowHandle,
            nullptr,
            nullptr,
            nullptr);
        if (!WindowHandle)
        {
            return nullptr;
        }
        if (!::SetWindowSubclass(
            WindowHandle,
            [](
                _In_ HWND hWnd,
                _In_ UINT uMsg,
                _In_ WPARAM wParam,
                _In_ LPARAM lParam,
                _In_ UINT_PTR uIdSubclass,
                _In_ DWORD_PTR dwRefData) -> LRESULT
        {
            UNREFERENCED_PARAMETER(uIdSubclass);
            UNREFERENCED_PARAMETER(dwRefData);

            switch (uMsg)
            {
            case WM_CLOSE:
            {
                HWND ParentWindow = ::GetWindow(hWnd, GW_OWNER);
                if (ParentWindow)
                {
                    ::EnableWindow(ParentWindow, TRUE);
                }
                break;
            }
            default:
                break;
            }

            return ::DefSubclassProc(
                hWnd,
                uMsg,
                wParam,
                lParam);
        },
            0,
            0))
        {
            ::DestroyWindow(WindowHandle);
            return nullptr;
        }
        return WindowHandle;
    }

    HWND K7ModernCreateXamlDialog(
        _In_opt_ HWND ParentWindowHandle)
    {
        HWND WindowHandle = ::K7ModernCreateXamlWindow(
            ParentWindowHandle,
            WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME,
            WS_CAPTION | WS_SYSMENU);

        MILE_WINDOW_SYSTEM_BACKDROP_TYPE SystemBackdropType =
            MILE_WINDOW_SYSTEM_BACKDROP_TYPE_AUTO;
        if (S_OK == ::MileGetWindowSystemBackdropTypeAttribute(
            WindowHandle,
            &SystemBackdropType))
        {
            if (MILE_WINDOW_SYSTEM_BACKDROP_TYPE_AUTO != SystemBackdropType &&
                MILE_WINDOW_SYSTEM_BACKDROP_TYPE_NONE != SystemBackdropType)
            {
                const COLORREF IgnoreAccentColor = static_cast<COLORREF>(-2);
                ::MileSetWindowCaptionColorAttribute(
                    WindowHandle,
                    IgnoreAccentColor);
            }
        }

        return WindowHandle;
    }

    int K7ModernShowXamlWindow(
        _In_opt_ HWND WindowHandle,
        _In_ int Width,
        _In_ int Height,
        _In_ HWND ParentWindowHandle)
    {
        if (!WindowHandle)
        {
            return -1;
        }

        UINT DpiValue = ::GetDpiForWindow(WindowHandle);

        int ScaledWidth = ::MulDiv(Width, DpiValue, USER_DEFAULT_SCREEN_DPI);
        int ScaledHeight = ::MulDiv(Height, DpiValue, USER_DEFAULT_SCREEN_DPI);

        RECT ParentRect = {};
        if (ParentWindowHandle)
        {
            ::GetWindowRect(ParentWindowHandle, &ParentRect);
        }
        else
        {
            HMONITOR MonitorHandle = ::MonitorFromWindow(
                WindowHandle,
                MONITOR_DEFAULTTONEAREST);
            if (MonitorHandle)
            {
                MONITORINFO MonitorInfo;
                MonitorInfo.cbSize = sizeof(MONITORINFO);
                if (::GetMonitorInfoW(MonitorHandle, &MonitorInfo))
                {
                    ParentRect = MonitorInfo.rcWork;
                }
            }
        }

        int ParentWidth = ParentRect.right - ParentRect.left;
        int ParentHeight = ParentRect.bottom - ParentRect.top;

        ::SetWindowPos(
            WindowHandle,
            nullptr,
            ParentRect.left + ((ParentWidth - ScaledWidth) / 2),
            ParentRect.top + ((ParentHeight - ScaledHeight) / 2),
            ScaledWidth,
            ScaledHeight,
            SWP_NOZORDER | SWP_NOACTIVATE);

        ::ShowWindow(WindowHandle, SW_SHOW);
        ::UpdateWindow(WindowHandle);

        return ::MileXamlContentWindowDefaultMessageLoop();
    }

    int K7ModernShowXamlDialog(
        _In_opt_ HWND WindowHandle,
        _In_ int Width,
        _In_ int Height,
        _In_ LPVOID Content,
        _In_ HWND ParentWindowHandle)
    {
        if (!WindowHandle)
        {
            return -1;
        }

        ::MileAllowNonClientDefaultDrawingForWindow(WindowHandle, FALSE);

        HMENU MenuHandle = ::GetSystemMenu(WindowHandle, FALSE);
        if (MenuHandle)
        {
            ::RemoveMenu(MenuHandle, 0, MF_SEPARATOR);
            ::RemoveMenu(MenuHandle, SC_RESTORE, MF_BYCOMMAND);
            ::RemoveMenu(MenuHandle, SC_SIZE, MF_BYCOMMAND);
            ::RemoveMenu(MenuHandle, SC_MINIMIZE, MF_BYCOMMAND);
            ::RemoveMenu(MenuHandle, SC_MAXIMIZE, MF_BYCOMMAND);
        }

        // Restore the owner's original state even when content setup fails or
        // throws before WM_CLOSE can run. Otherwise fallback dialogs inherit a
        // disabled main window and the application appears stuck.
        const BOOL ParentWasEnabled = ParentWindowHandle &&
            ::IsWindowEnabled(ParentWindowHandle);
        if (ParentWindowHandle)
            ::EnableWindow(ParentWindowHandle, FALSE);

        int Result = -1;
        try
        {
            if (SUCCEEDED(::MileXamlSetXamlContentForContentWindow(
                WindowHandle, Content)))
            {
                Result = ::K7ModernShowXamlWindow(
                    WindowHandle, Width, Height, ParentWindowHandle);
            }
        }
        catch (...)
        {
            Result = -1;
        }
        if (Result == -1 && ::IsWindow(WindowHandle))
            ::DestroyWindow(WindowHandle);
        if (ParentWindowHandle && ::IsWindow(ParentWindowHandle))
            ::EnableWindow(ParentWindowHandle, ParentWasEnabled);
        return Result;
    }

    winrt::DesktopWindowXamlSource K7ModernGetDesktopWindowXamlSource(
        _In_ HWND WindowHandle)
    {
        winrt::DesktopWindowXamlSource XamlSource = nullptr;
        winrt::copy_from_abi(
            XamlSource,
            ::GetPropW(WindowHandle, L"XamlWindowSource"));
        return XamlSource;
    }
}

EXTERN_C INT WINAPI K7ModernShowSponsorDialog(
    _In_opt_ HWND ParentWindowHandle)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::SponsorPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::SponsorPage;

    Interface Window = winrt::make<Implementation>(WindowHandle);

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        460,
        320,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
}

EXTERN_C INT WINAPI K7ModernShowAboutDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR ExtendedMessage)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::AboutPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::AboutPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        ExtendedMessage);

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        480,
        320,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
}

EXTERN_C INT WINAPI K7ModernShowInformationDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_opt_ LPCWSTR Content)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::InformationPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::InformationPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Title,
        Content);

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        560,
        560,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
}

EXTERN_C VOID WINAPI K7ModernUpdateProgressWindowStatus(
    _In_ HWND WindowHandle,
    _In_ PK7_PROGRESS_WINDOW_STATUS Status)
{
    if (!WindowHandle || !Status)
    {
        return;
    }

    winrt::DesktopWindowXamlSource XamlSource =
        ::K7ModernGetDesktopWindowXamlSource(WindowHandle);
    if (!XamlSource)
    {
        return;
    }

    using Interface =
        winrt::NanaZip::Modern::ProgressPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::ProgressPage;
    Interface InstanceObject = XamlSource.Content().as<Interface>();
    if (!InstanceObject)
    {
        return;
    }
    winrt::get_self<Implementation>(InstanceObject)->UpdateStatus(Status);
}

EXTERN_C VOID WINAPI K7ModernSetProgressWindowPausedMode(
    _In_ HWND WindowHandle,
    _In_ BOOL Paused)
{
    if (!WindowHandle)
    {
        return;
    }

    winrt::DesktopWindowXamlSource XamlSource =
        ::K7ModernGetDesktopWindowXamlSource(WindowHandle);
    if (!XamlSource)
    {
        return;
    }

    using Interface =
        winrt::NanaZip::Modern::ProgressPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::ProgressPage;
    Interface InstanceObject = XamlSource.Content().as<Interface>();
    if (!InstanceObject)
    {
        return;
    }
    winrt::get_self<Implementation>(InstanceObject)->SetPausedMode(Paused);
}

EXTERN_C INT WINAPI K7ModernShowProgressWindow(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_ BOOL ShowCompressionInformation,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext)
{
    HWND WindowHandle = ::K7ModernCreateXamlWindow(
        ParentWindowHandle,
        WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME,
        WS_OVERLAPPEDWINDOW);
    if (!WindowHandle)
    {
        return -1;
    }

    ::MileAllowNonClientDefaultDrawingForWindow(WindowHandle, FALSE);

    using Interface =
        winrt::NanaZip::Modern::ProgressPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::ProgressPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Title,
        ShowCompressionInformation);

    if (FAILED(::MileXamlSetXamlContentForContentWindow(
        WindowHandle,
        winrt::get_abi(Window))))
    {
        ::DestroyWindow(WindowHandle);
        return -1;
    }

    if (WindowSubclassHandler)
    {
        if (!::SetWindowSubclass(
            WindowHandle,
            WindowSubclassHandler,
            1,
            reinterpret_cast<DWORD_PTR>(WindowSubclassContext)))
        {
            ::DestroyWindow(WindowHandle);
            return -1;
        }
    }

    int Result = ::K7ModernShowXamlWindow(
        WindowHandle,
        600,
        360,
        ParentWindowHandle);

    return Result;
}

EXTERN_C INT WINAPI K7ModernShowCopyLocationDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_opt_ LPCWSTR Subtitle,
    _In_opt_ LPCWSTR AdditionalInformation,
    _In_opt_ LPCWSTR InitialPath,
    _In_ BOOL ShowExtractAll,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::CopyLocationPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::CopyLocationPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Title,
        Subtitle,
        AdditionalInformation,
        InitialPath,
        ShowExtractAll);

    if (WindowSubclassHandler)
    {
        if (!::SetWindowSubclass(
            WindowHandle,
            WindowSubclassHandler,
            1,
            reinterpret_cast<DWORD_PTR>(WindowSubclassContext)))
        {
            ::DestroyWindow(WindowHandle);
            return -1;
        }
    }

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        600,
        400,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
}

EXTERN_C LPCWSTR WINAPI K7ModernGetCopyLocationDialogPath(
    _In_ HWND WindowHandle)
{
    if (!WindowHandle)
    {
        return nullptr;
    }

    winrt::DesktopWindowXamlSource XamlSource =
        ::K7ModernGetDesktopWindowXamlSource(WindowHandle);
    if (!XamlSource)
    {
        return nullptr;
    }

    using Interface =
        winrt::NanaZip::Modern::CopyLocationPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::CopyLocationPage;
    Interface InstanceObject = XamlSource.Content().as<Interface>();
    if (!InstanceObject)
    {
        return nullptr;
    }
    return winrt::get_self<Implementation>(InstanceObject)->GetPath();
}

// **************** CuinZip P1-2 Modification Start ****************

EXTERN_C INT WINAPI K7ModernShowSettingsDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ const K7_MODERN_SETTINGS_CALLBACKS* Callbacks)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::SettingsPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::SettingsPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Callbacks);

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        760,
        600,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
}

// CuinZip P1-8:K7ModernGetUiString 的所有权缓存(文件级供语言切换清空)
namespace
{
    std::map<std::wstring, winrt::hstring> g_CachedAbiUiStrings;
    std::mutex g_CachedAbiUiStringsMutex;
}

EXTERN_C LPCWSTR WINAPI K7ModernGetUiString(
    _In_ LPCWSTR Name,
    _In_opt_ LPCWSTR Fallback)
{
    if (!Name)
    {
        return nullptr;
    }

    // GetUiString 内部有缓存;返回的字符串由本模块持有,
    // 调用方直接使用,不需要释放(与 K7ModernGetLegacyStringResource
    // 的所有权约定一致)。
    std::lock_guard Lock(g_CachedAbiUiStringsMutex);
    auto Iterator = g_CachedAbiUiStrings.emplace(
        std::wstring(Name),
        winrt::NanaZip::Modern::GetUiString(
            Name, Fallback ? Fallback : L""));
    return Iterator.first->second.c_str();
}

// **************** CuinZip P1-8 Modification Start ****************

namespace
{
    // Only the file-manager host persists explicit Settings choices.
    K7_MODERN_LANGUAGE_PERSIST_CALLBACK g_LanguagePersist = nullptr;

    // 语言偏好的持久化位置(HKCU);跟随系统 = 删除值。
    // 实测 XAML island 线程内直接写注册表不落盘(独立进程正常),
    // 因此所有持久化都转到分离的普通后台线程执行。
    wchar_t const* const kLanguageRegistryKey = L"Software\\CuinZip\\FM";
    wchar_t const* const kLanguageRegistryValue = L"Language";

    void PersistAppLanguageAsync(std::wstring const& language)
    {
        std::thread([language]()
        {
            if (g_LanguagePersist)
            {
                g_LanguagePersist(language.c_str());
                return;
            }
            HKEY key = nullptr;
            if (ERROR_SUCCESS != ::RegCreateKeyExW(
                HKEY_CURRENT_USER, kLanguageRegistryKey, 0, nullptr,
                REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr,
                &key, nullptr))
            {
                return;
            }
            if (language.empty())
            {
                ::RegDeleteValueW(key, kLanguageRegistryValue);
            }
            else
            {
                ::RegSetValueExW(
                    key, kLanguageRegistryValue, 0, REG_SZ,
                    reinterpret_cast<BYTE const*>(language.c_str()),
                    static_cast<DWORD>(
                        (language.size() + 1) * sizeof(wchar_t)));
            }
            ::RegCloseKey(key);
        }).detach();
    }
}

void winrt::NanaZip::Modern::PersistUiLanguagePreference(std::wstring_view language)
{
    PersistAppLanguageAsync(std::wstring(language));
}

EXTERN_C BOOL WINAPI K7ModernSetAppLanguage(_In_opt_ LPCWSTR Language)
{
    std::wstring language(Language ? Language : L"");
    g_AppLanguage = language;

    std::wstring qualifier = language;

    // 1) "跟随系统":取当前用户语言列表首项作为解析语言(回放用户
    //    真实偏好);失败时保持空 = 英文默认候选
    if (qualifier.empty())
    {
        ULONG count = 0;
        ULONG length = 0;
        if (::GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &length)
            && length > 1)
        {
            std::vector<wchar_t> languages(length, L'\0');
            if (::GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count,
                languages.data(), &length))
            {
                qualifier = languages.data();
            }
        }
    }

    // 3) 主力机制:设置资源解析语言(GetUiString 与 Legacy 解析均按
    //    候选手选实现,不接触 ResourceContext——其 view-independent
    //    形态在 FM 主窗口线程引发过 0xC000041D 崩溃,P1-8 弃用)
    winrt::NanaZip::Modern::SetUiResourceLanguage(qualifier);

    // 4) 清空全部字符串缓存,让后续取值重新按新语言解析
    winrt::NanaZip::Modern::ClearUiStringCache();
    {
        std::lock_guard Lock(g_CachedLanguageStringResourcesMutex);
        g_CachedLanguageStringResources.clear();
    }
    {
        std::lock_guard Lock(g_CachedAbiUiStringsMutex);
        g_CachedAbiUiStrings.clear();
    }

    // 5) 持久化(后台线程;首页按钮与设置页切换共用本入口)
    PersistAppLanguageAsync(language);

    return TRUE;
}

EXTERN_C LPCWSTR WINAPI K7ModernGetAppLanguage()
{
    return g_AppLanguage.c_str();
}

EXTERN_C VOID WINAPI K7ModernSetLanguagePersistCallback(
    _In_opt_ K7_MODERN_LANGUAGE_PERSIST_CALLBACK Callback)
{
    g_LanguagePersist = Callback;
}

// **************** CuinZip P1-8 Modification End ****************

// **************** CuinZip P1-2 Modification End ****************

// **************** CuinZip P1-3 Modification Start ****************

EXTERN_C INT WINAPI K7ModernShowCompressDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_ const K7_DIALOG_MIRROR_ENGINE* Engine,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    try
    {
    using Interface =
        winrt::NanaZip::Modern::CompressDialogPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::CompressDialogPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Engine);

    if (WindowSubclassHandler)
    {
        if (!::SetWindowSubclass(
            WindowHandle,
            WindowSubclassHandler,
            1,
            reinterpret_cast<DWORD_PTR>(WindowSubclassContext)))
        {
            ::DestroyWindow(WindowHandle);
            return -1;
        }
    }

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        560,
        560,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
    }
    catch (...)
    {
        if (::IsWindow(WindowHandle))
            ::DestroyWindow(WindowHandle);
        return -1;
    }
}

EXTERN_C INT WINAPI K7ModernShowExtractDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_ const K7_DIALOG_MIRROR_ENGINE* Engine,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    try
    {
    using Interface =
        winrt::NanaZip::Modern::ExtractDialogPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::ExtractDialogPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        Engine);

    if (WindowSubclassHandler)
    {
        if (!::SetWindowSubclass(
            WindowHandle,
            WindowSubclassHandler,
            1,
            reinterpret_cast<DWORD_PTR>(WindowSubclassContext)))
        {
            ::DestroyWindow(WindowHandle);
            return -1;
        }
    }

    int Result = ::K7ModernShowXamlDialog(
        WindowHandle,
        500,
        440,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return Result;
    }
    catch (...)
    {
        if (::IsWindow(WindowHandle))
            ::DestroyWindow(WindowHandle);
        return -1;
    }
}

// **************** CuinZip P1-3 Modification End ****************

// **************** CuinZip P1-6 Modification Start ****************

EXTERN_C INT WINAPI K7ModernShowStartWindow(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ K7_START_GET_RECENT_CALLBACK GetRecent,
    _In_opt_ K7_MODERN_START_RESULT* Result)
{
    HWND WindowHandle = ::K7ModernCreateXamlDialog(ParentWindowHandle);
    if (!WindowHandle)
    {
        return -1;
    }

    using Interface =
        winrt::NanaZip::Modern::StartPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::StartPage;

    Interface Window = winrt::make<Implementation>(
        WindowHandle,
        GetRecent,
        Result);

    int ResultCode = ::K7ModernShowXamlDialog(
        WindowHandle,
        640,
        560,
        winrt::get_abi(Window),
        ParentWindowHandle);

    return ResultCode;
}

// **************** CuinZip P1-6 Modification End ****************

/*
 * PROJECT:    NanaZip.Modern
 * FILE:       FileAssociation.cpp
 * PURPOSE:    CuinZip P1-4 文件关联状态查询与官方默认应用设置入口。
 *             只读取 Windows 官方的 UserChoice / Progid 状态,不写入任何
 *             默认应用注册,Windows 11 的用户确认机制不被绕过。
 *
 * LICENSE:    The MIT License
 */

#include "pch.h"

#include "NanaZip.Modern.h"

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <appmodel.h>

#include <cstring>
#include <string>

namespace
{
    // 文件管理器可执行名(MSIX 包内与未打包部署一致),用于判定默认打开
    // 方式是否指向 CuinZip。
    constexpr wchar_t const* kFileManagerExecutable =
        L"NanaZip.Modern.FileManager.exe";

    std::wstring ReadStringValue(
        _In_ HKEY RootKey,
        _In_ std::wstring const& SubKey,
        _In_ std::wstring const& ValueName)
    {
        std::wstring Result;
        DWORD Type = 0;
        DWORD Size = 0;
        LONG Status = ::RegGetValueW(
            RootKey,
            SubKey.c_str(),
            ValueName.c_str(),
            RRF_RT_REG_SZ,
            &Type,
            nullptr,
            &Size);
        if (Status != ERROR_SUCCESS || Size < sizeof(wchar_t))
        {
            return Result;
        }

        Result.resize(Size / sizeof(wchar_t));
        Status = ::RegGetValueW(
            RootKey,
            SubKey.c_str(),
            ValueName.c_str(),
            RRF_RT_REG_SZ,
            &Type,
            &Result[0],
            &Size);
        if (Status != ERROR_SUCCESS)
        {
            Result.clear();
            return Result;
        }

        while (!Result.empty() && L'\0' == Result.back())
        {
            Result.pop_back();
        }
        return Result;
    }

    std::wstring GetUserChoiceProgId(std::wstring const& Extension)
    {
        std::wstring SubKey = L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\Explorer\\FileExts\\";
        SubKey += Extension;
        SubKey += L"\\UserChoice";
        return ReadStringValue(HKEY_CURRENT_USER, SubKey, L"Progid");
    }

    std::wstring GetProgIdOpenCommand(std::wstring const& ProgId)
    {
        if (ProgId.empty())
        {
            return std::wstring();
        }
        std::wstring SubKey = ProgId;
        SubKey += L"\\shell\\open\\command";
        // HKCR 是合并视图(HKLM\Software\Classes + HKCU\Software\Classes +
        // 包注册),普通进程读取即可看到 MSIX 包的 Progid 注册。
        return ReadStringValue(HKEY_CLASSES_ROOT, SubKey, std::wstring());
    }

    std::wstring GetExtensionOpenCommand(std::wstring const& Extension)
    {
        std::wstring SubKey = Extension;
        SubKey += L"\\shell\\open\\command";
        return ReadStringValue(HKEY_CLASSES_ROOT, SubKey, std::wstring());
    }

    bool ContainsInsensitive(
        std::wstring const& Text,
        std::wstring const& Pattern)
    {
        return ::StrStrIW(Text.c_str(), Pattern.c_str()) != nullptr;
    }

    std::wstring ExtractExecutableName(std::wstring const& Command)
    {
        std::wstring Trimmed = Command;
        size_t Start = Trimmed.find_first_not_of(L" \t");
        if (Start == std::wstring::npos)
        {
            return std::wstring();
        }
        Trimmed.erase(0, Start);

        std::wstring FirstToken;
        if (!Trimmed.empty() && L'"' == Trimmed[0])
        {
            size_t End = Trimmed.find(L'"', 1);
            FirstToken = (End == std::wstring::npos)
                ? Trimmed.substr(1)
                : Trimmed.substr(1, End - 1);
        }
        else
        {
            size_t End = Trimmed.find_first_of(L" ");
            FirstToken = (End == std::wstring::npos) ? Trimmed : Trimmed.substr(0, End);
        }

        if (FirstToken.empty())
        {
            return std::wstring();
        }
        return std::wstring(::PathFindFileNameW(FirstToken.c_str()));
    }

    std::wstring GetCurrentApplicationUserModelIdSimple()
    {
        using ProcType = decltype(::GetCurrentApplicationUserModelId)*;

        static std::wstring CachedResult = ([]() -> std::wstring
        {
            std::wstring ApplicationUserModelId;
            HMODULE ModuleHandle = ::GetModuleHandleW(L"kernel32.dll");
            if (!ModuleHandle)
            {
                return ApplicationUserModelId;
            }

            ProcType ProcAddress = reinterpret_cast<ProcType>(
                ::GetProcAddress(ModuleHandle, "GetCurrentApplicationUserModelId"));
            if (!ProcAddress)
            {
                return ApplicationUserModelId;
            }

            UINT32 Length = 0;
            if (ERROR_INSUFFICIENT_BUFFER != ProcAddress(&Length, nullptr))
            {
                return ApplicationUserModelId;
            }

            ApplicationUserModelId.resize(Length);
            if (ERROR_SUCCESS != ProcAddress(
                &Length,
                reinterpret_cast<PWSTR>(ApplicationUserModelId.data())))
            {
                ApplicationUserModelId.clear();
                return ApplicationUserModelId;
            }

            while (!ApplicationUserModelId.empty() &&
                L'\0' == ApplicationUserModelId.back())
            {
                ApplicationUserModelId.pop_back();
            }
            return ApplicationUserModelId;
        }());

        return CachedResult;
    }
}

EXTERN_C BOOL WINAPI K7ModernQueryFileAssociation(
    _In_ LPCWSTR Extension,
    _Out_ BOOL* IsDefault,
    _Out_writes_opt_(CurrentAppNameLength) LPWSTR CurrentAppName,
    _In_ UINT32 CurrentAppNameLength)
{
    if (!Extension || !IsDefault)
    {
        return FALSE;
    }

    *IsDefault = FALSE;
    if (CurrentAppName && CurrentAppNameLength > 0)
    {
        CurrentAppName[0] = L'\0';
    }

    std::wstring NormalizedExtension(Extension);
    if (!NormalizedExtension.empty() && L'.' != NormalizedExtension[0])
    {
        NormalizedExtension.insert(0, L".");
    }

    std::wstring Command;

    // 1. Windows 11 的默认应用状态记录在 FileExts\<ext>\UserChoice\Progid。
    std::wstring ProgId = GetUserChoiceProgId(NormalizedExtension);
    if (!ProgId.empty())
    {
        Command = GetProgIdOpenCommand(ProgId);
    }

    // 2. 没有 UserChoice 时回落到 Progid 注册(经典每用户/系统级关联)。
    if (Command.empty())
    {
        std::wstring ExtensionProgId = ReadStringValue(
            HKEY_CLASSES_ROOT,
            NormalizedExtension,
            std::wstring());
        Command = GetProgIdOpenCommand(ExtensionProgId);
    }

    // 3. 极少数情况下 open 命令直接挂在扩展名键下。
    if (Command.empty())
    {
        Command = GetExtensionOpenCommand(NormalizedExtension);
    }

    if (Command.empty())
    {
        // 查询成功,但该扩展名当前没有默认打开方式。
        return TRUE;
    }

    if (ContainsInsensitive(Command, kFileManagerExecutable))
    {
        *IsDefault = TRUE;
    }

    if (CurrentAppName && CurrentAppNameLength > 0)
    {
        std::wstring Name = ExtractExecutableName(Command);
        if (!Name.empty())
        {
            ::wcsncpy_s(
                CurrentAppName,
                CurrentAppNameLength,
                Name.c_str(),
                _TRUNCATE);
        }
    }

    return TRUE;
}

EXTERN_C BOOL WINAPI K7ModernLaunchDefaultAppsSettings()
{
    std::wstring NavigateUri = L"ms-settings:defaultapps";
    std::wstring ApplicationUserModelId = GetCurrentApplicationUserModelIdSimple();
    if (!ApplicationUserModelId.empty())
    {
        NavigateUri.append(L"?registeredAUMID=");
        NavigateUri.append(ApplicationUserModelId);
    }

    SHELLEXECUTEINFOW Information = {};
    Information.cbSize = sizeof(SHELLEXECUTEINFOW);
    Information.lpVerb = L"open";
    Information.lpFile = NavigateUri.c_str();
    Information.nShow = SW_SHOWNORMAL;
    return ::ShellExecuteExW(&Information) ? TRUE : FALSE;
}

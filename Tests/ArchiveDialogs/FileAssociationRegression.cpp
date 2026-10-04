// Production association code, with all configuration reads and shell launches
// replaced by in-memory Win32 mocks. This test never changes Windows defaults.
#include <Windows.h>
#include <CommCtrl.h>
#include <Shellapi.h>
#include <shlwapi.h>
#include <appmodel.h>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <tuple>

namespace
{
    using Key = std::tuple<ULONG_PTR, std::wstring, std::wstring>;
    std::map<Key, std::wstring> values;
    std::wstring launched;
    int failures = 0;
    void Set(HKEY root, const wchar_t* path, const wchar_t* name, const wchar_t* value)
    {
        values[{reinterpret_cast<ULONG_PTR>(root), path, name}] = value;
    }
    void Check(bool ok, const char* name)
    {
        std::printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) ++failures;
    }
}

LSTATUS WINAPI TestRegGetValueW(HKEY root, LPCWSTR path, LPCWSTR name,
    DWORD, LPDWORD type, PVOID data, LPDWORD size)
{
    auto found = values.find({reinterpret_cast<ULONG_PTR>(root),
        path ? path : L"", name ? name : L""});
    if (found == values.end()) return ERROR_FILE_NOT_FOUND;
    DWORD needed = static_cast<DWORD>((found->second.size() + 1) * sizeof(wchar_t));
    if (type) *type = REG_SZ;
    if (data && *size < needed) { *size = needed; return ERROR_MORE_DATA; }
    if (data) std::memcpy(data, found->second.c_str(), needed);
    *size = needed;
    return ERROR_SUCCESS;
}
BOOL WINAPI TestGetModuleHandleExW(DWORD, LPCWSTR, HMODULE* module)
{
    *module = reinterpret_cast<HMODULE>(1);
    return TRUE;
}
DWORD WINAPI TestGetModuleFileNameW(HMODULE, LPWSTR path, DWORD capacity)
{
    const std::wstring value = L"D:\\CuinZip\\NanaZip.Modern.dll";
    wcsncpy_s(path, capacity, value.c_str(), _TRUNCATE);
    return static_cast<DWORD>(value.size());
}
HMODULE WINAPI TestGetModuleHandleW(LPCWSTR) { return reinterpret_cast<HMODULE>(1); }
LONG WINAPI TestGetCurrentApplicationUserModelId(UINT32*, PWSTR)
{
    return APPMODEL_ERROR_NO_APPLICATION;
}
FARPROC WINAPI TestGetProcAddress(HMODULE, LPCSTR name)
{
    return std::strcmp(name, "GetCurrentApplicationUserModelId") == 0
        ? reinterpret_cast<FARPROC>(&TestGetCurrentApplicationUserModelId) : nullptr;
}
BOOL WINAPI TestShellExecuteExW(SHELLEXECUTEINFOW* info)
{
    launched = info->lpFile;
    return TRUE;
}

#define RegGetValueW TestRegGetValueW
#define GetModuleHandleExW TestGetModuleHandleExW
#define GetModuleFileNameW TestGetModuleFileNameW
#define GetModuleHandleW TestGetModuleHandleW
#define GetProcAddress TestGetProcAddress
#define ShellExecuteExW TestShellExecuteExW
#include "../../NanaZip.Modern/FileAssociation.cpp"
#undef RegGetValueW
#undef GetModuleHandleExW
#undef GetModuleFileNameW
#undef GetModuleHandleW
#undef GetProcAddress
#undef ShellExecuteExW

int main()
{
    const wchar_t* choice = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.zip\\UserChoice";
    BOOL isDefault = TRUE;
    wchar_t name[128] = {};

    Set(HKEY_CURRENT_USER, choice, L"Progid", L"Other.Delegate");
    Set(HKEY_CLASSES_ROOT, L"Other.Delegate\\Application", L"ApplicationName", L"Other app");
    Set(HKEY_CLASSES_ROOT, L".zip", L"", L"CuinZip.Archive");
    Set(HKEY_CLASSES_ROOT, L"CuinZip.Archive\\shell\\open\\command", L"", L"\"D:\\CuinZip\\CuinZip.exe\" \"%1\"");
    K7ModernQueryFileAssociation(L"zip", &isDefault, name, ARRAYSIZE(name));
    Check(!isDefault && std::wstring(name) == L"Other app",
        "DelegateExecute UserChoice never falls back to an unselected class");

    values.clear();
    Set(HKEY_CURRENT_USER, choice, L"Progid", L"Other.Command");
    Set(HKEY_CLASSES_ROOT, L"Other.Command\\shell\\open\\command", L"", L"\"C:\\Other\\viewer.exe\" --hint=NanaZip.Modern.FileManager.exe \"%1\"");
    K7ModernQueryFileAssociation(L".zip", &isDefault, name, ARRAYSIZE(name));
    Check(!isDefault && std::wstring(name) == L"viewer.exe",
        "a filename in another program's arguments is not the default executable");

    Set(HKEY_CLASSES_ROOT, L"Other.Command\\shell\\open\\command", L"", L"\"C:\\NanaZip\\NanaZip.Modern.FileManager.exe\" \"%1\"");
    K7ModernQueryFileAssociation(L".zip", &isDefault, name, ARRAYSIZE(name));
    Check(!isDefault, "upstream NanaZip in another directory is not CuinZip");

    Set(HKEY_CLASSES_ROOT, L"Other.Command\\shell\\open\\command", L"", L"\"D:\\CuinZip\\NanaZip.Modern.FileManager.exe\" \"%1\"");
    K7ModernQueryFileAssociation(L".zip", &isDefault, name, ARRAYSIZE(name));
    Check(isDefault && std::wstring(name) == L"CuinZip",
        "this runtime's file manager is recognised as CuinZip");

    values.clear();
    Set(HKEY_CLASSES_ROOT, L".zip", L"", L"CuinZip.Archive");
    Set(HKEY_CLASSES_ROOT, L"CuinZip.Archive\\shell\\open\\command", L"", L"\"D:\\CuinZip\\CuinZip.exe\" \"%1\"");
    K7ModernQueryFileAssociation(L".zip", &isDefault, name, ARRAYSIZE(name));
    Check(isDefault && std::wstring(name) == L"CuinZip",
        "Setup launcher class is recognised as CuinZip");

    values.clear();
    Set(HKEY_CURRENT_USER, L"Software\\RegisteredApplications", L"CuinZip", L"Software\\CuinZip\\Capabilities");
    Check(K7ModernLaunchDefaultAppsSettings() &&
        launched == L"ms-settings:defaultapps?registeredAppUser=CuinZip",
        "Setup opens Windows settings for its registered user application");
    values.clear();
    Check(K7ModernLaunchDefaultAppsSettings() && launched == L"ms-settings:defaultapps",
        "portable runtime opens generic Windows defaults without claiming registration");

    Check(!K7ModernQueryFileAssociation(nullptr, &isDefault, name, ARRAYSIZE(name)) &&
        !K7ModernQueryFileAssociation(L"zip", nullptr, name, ARRAYSIZE(name)),
        "invalid association arguments fail safely");
    std::printf("%d failure(s)\n", failures);
    return failures ? 1 : 0;
}

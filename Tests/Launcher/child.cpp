#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR Arguments, int)
{
    std::vector<wchar_t> Path(32768);
    DWORD Length = GetModuleFileNameW(nullptr, Path.data(), static_cast<DWORD>(Path.size()));
    if (!Length || Length >= Path.size()) return 2;
    std::wstring Capture(Path.data(), Length);
    Capture.resize(Capture.find_last_of(L"\\") + 1);
    Capture += L"capture.txt";
    DWORD Needed = GetCurrentDirectoryW(0, nullptr);
    std::vector<wchar_t> Cwd(Needed);
    if (!GetCurrentDirectoryW(Needed, Cwd.data())) return 3;
    std::wstring Text = L"args=" + std::wstring(Arguments ? Arguments : L"")
        + L"\r\ncwd=" + Cwd.data();
    HANDLE File = CreateFileW(Capture.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (File == INVALID_HANDLE_VALUE) return 4;
    DWORD Written = 0;
    const wchar_t Bom = 0xfeff;
    BOOL Ok = WriteFile(File, &Bom, sizeof(Bom), &Written, nullptr)
        && WriteFile(File, Text.data(), static_cast<DWORD>(Text.size() * sizeof(wchar_t)),
            &Written, nullptr);
    CloseHandle(File);
    return Ok ? 0 : 5;
}

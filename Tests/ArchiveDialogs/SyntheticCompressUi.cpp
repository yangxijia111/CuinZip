// Loads the built Modern dialog with an in-memory engine. No Classic archive
// dialog or application preference code is used, and no user settings are saved.
#include <Windows.h>
#include <CommCtrl.h>
#include "../../NanaZip.Modern/NanaZip.Modern.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace
{
    std::map<UINT, std::wstring> fields = {
        {100, L"initial.7z"}, {105, L"100K"}, {111, L"initial-parameters"},
        {120, L"SyntheticSecret"}, {121, L"SyntheticSecret"}
    };
    std::map<UINT, bool> checks;
    const std::map<UINT, std::wstring> captions = {
        {0, L"CuinZip synthetic compression test"},
        {100, L"initial.7z"}, {101, L"Browse"}, {130, L"D:\\Synthetic"},
        {4001, L"Archive name"}, {4003, L"Archive format"}, {4004, L"Compression level"},
        {3801, L"Password"}, {3802, L"Confirm password"},
        {4002, L"Update mode"}, {3410, L"Path mode"}, {4012, L"Create self extracting archive"},
        {4005, L"Method"}, {4006, L"Dictionary"}, {4007, L"Order"}, {4008, L"Solid size"},
        {4009, L"Threads"}, {4017, L"Memory"}, {3803, L"Show password"},
        {4016, L"Encrypt file names"}, {4015, L"Encryption method"},
        {7302, L"Split volume size"}, {4010, L"Parameters"},
        {4013, L"Shared files"}, {4019, L"Delete source files"}, {2100, L"Options"}
    };
    void Copy(const std::wstring& source, wchar_t* target, UINT capacity)
    {
        if (target && capacity) wcsncpy_s(target, capacity, source.c_str(), _TRUNCATE);
    }
    UINT WINAPI ReadCombo(void*, UINT id, wchar_t* items, UINT maxItems,
        LPARAM* data, int* selection, wchar_t* text, UINT textMax, BOOL* enabled, BOOL* visible)
    {
        const std::vector<std::wstring> choices = id == 104
            ? std::vector<std::wstring>{L"7z", L"Zip"}
            : std::vector<std::wstring>{fields.count(id) ? fields[id] : L"Normal"};
        for (UINT i = 0; i < (std::min)(maxItems, (UINT)choices.size()); ++i)
        {
            Copy(choices[i], items ? items + (size_t)i * K7_DIALOG_MIRROR_ITEM_TEXT : nullptr,
                K7_DIALOG_MIRROR_ITEM_TEXT);
            if (data) data[i] = i;
        }
        if (selection) *selection = 0;
        Copy(fields.count(id) ? fields[id] : choices[0], text, textMax);
        if (enabled) *enabled = TRUE;
        if (visible) *visible = TRUE;
        return (UINT)choices.size();
    }
    BOOL WINAPI ReadText(void*, UINT id, wchar_t* text, UINT capacity, BOOL* enabled, BOOL* visible)
    {
        Copy(fields.count(id) ? fields[id] : captions.count(id) ? captions.at(id) : L"", text, capacity);
        if (enabled) *enabled = TRUE;
        if (visible) *visible = TRUE;
        return TRUE;
    }
    BOOL WINAPI ReadCheck(void*, UINT id, BOOL* checked, BOOL* enabled, BOOL* visible)
    {
        if (checked) *checked = checks[id];
        if (enabled) *enabled = TRUE;
        if (visible) *visible = TRUE;
        return TRUE;
    }
    void WINAPI SetComboSelection(void*, UINT id, int value)
    {
        if (id == 104) fields[id] = value == 0 ? L"7z" : L"Zip";
    }
    void WINAPI SetComboText(void*, UINT id, const wchar_t* text) { fields[id] = text; }
    void WINAPI SetText(void*, UINT id, const wchar_t* text) { fields[id] = text; }
    void WINAPI SetCheck(void*, UINT id, BOOL checked) { checks[id] = checked != FALSE; }
    void WINAPI Click(void*, UINT) {}
    BOOL WINAPI PressOK(void*)
    {
        bool ok = true;
        auto test = [&ok](bool valid, const char* label)
        {
            std::printf("%s: %s\n", valid ? "PASS" : "FAIL", label);
            ok = ok && valid;
        };
        test(fields[100] == L"typed archive.7z", "focused live archive editor submitted typed text");
        test(fields[111] == L"new-parameters", "custom parameters persisted through option changes");
        test(fields[105].empty(), "cleared split volume was submitted as empty");
        test(fields[121] == L"SyntheticSecret", "confirmation password retains seeded native value");
        std::fflush(stdout);
        return TRUE; // Test results are asserted by the controller from stdout.
    }
}

int main()
{
    ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    HMODULE library = ::LoadLibraryW(L"NanaZip.Modern.dll");
    if (!library) { std::printf("INIT_FAIL LoadLibrary %lu\n", ::GetLastError()); return 2; }
    auto initialize = reinterpret_cast<decltype(&K7ModernInitialize)>(::GetProcAddress(library, "K7ModernInitialize"));
    auto show = reinterpret_cast<decltype(&K7ModernShowCompressDialog)>(::GetProcAddress(library, "K7ModernShowCompressDialog"));
    if (!initialize || !show) { std::puts("INIT_FAIL exports"); return 2; }
    std::puts("INIT_BEGIN"); std::fflush(stdout);
    HRESULT status = initialize();
    std::printf("INIT_RETURN 0x%08lX\n", status); std::fflush(stdout);
    if (FAILED(status)) { std::printf("INIT_FAIL initialize 0x%08lX\n", status); return 2; }
    K7_DIALOG_MIRROR_ENGINE engine = {};
    engine.Context = &fields;
    engine.ReadCombo = ReadCombo;
    engine.ReadText = ReadText;
    engine.ReadCheck = ReadCheck;
    engine.SetComboSelection = SetComboSelection;
    engine.SetComboText = SetComboText;
    engine.SetText = SetText;
    engine.SetCheck = SetCheck;
    engine.NotifyButtonClick = Click;
    engine.PressOK = PressOK;
    std::puts("SHOW_BEGIN"); std::fflush(stdout);
    const int result = show(nullptr, &engine, nullptr, nullptr);
    std::printf("DIALOG_EXIT %d\n", result);
    return result == -1 ? 2 : 0;
}

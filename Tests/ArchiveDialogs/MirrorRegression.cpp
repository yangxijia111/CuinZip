// Exercises the production bridge with real Win32 controls and a dialog that
// never loads or saves user settings. No archive GUI process is launched.
#include <Windows.h>
#include <CommCtrl.h>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <vector>

#ifdef CUINZIP_TEST_BASELINE
#include "../../Output/Tests/ArchiveDialogs/ModernDialogMirror.baseline.cpp"
#include "../../Output/Tests/ArchiveDialogs/DialogMirrorUi.baseline.h"
#else
#include "../../NanaZip.Universal/SevenZip/CPP/7zip/UI/GUI/ModernDialogMirror.cpp"
#include "../../NanaZip.Modern/DialogMirrorUi.h"
#endif

HINSTANCE g_hInstance;
// Only the three framework dispatch defaults are stubbed; bridge code and the
// Modern snapshot helpers above are compiled directly from production sources.
bool NWindows::NControl::CDialog::OnMessage(UINT, WPARAM, LPARAM) { return false; }
bool NWindows::NControl::CDialog::OnCommand(unsigned, unsigned, LPARAM) { return false; }
bool NWindows::NControl::CDialog::OnButtonClicked(unsigned, HWND) { return false; }

namespace
{
    int failures = 0;
    void Check(bool ok, const char* name)
    {
        std::printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) ++failures;
    }

    struct TestDialog : NWindows::NControl::CDialog
    {
        bool* Completed;
        HWND Created = nullptr;
        explicit TestDialog(bool* completed) : Completed(completed) {}
        bool OnMessage(UINT message, WPARAM wParam, LPARAM) override
        {
            if (message == WM_INITDIALOG)
                Created = *this;
            if (message == WM_COMMAND && LOWORD(wParam) == IDOK)
                *Completed = true;
            return false;
        }
    };

    INT WINAPI FailShow(HWND parent, const K7_DIALOG_MIRROR_ENGINE*, SUBCLASSPROC, LPVOID)
    {
        ::EnableWindow(parent, FALSE);
        return -1;
    }
    INT WINAPI ThrowShow(HWND parent, const K7_DIALOG_MIRROR_ENGINE*, SUBCLASSPROC, LPVOID)
    {
        ::EnableWindow(parent, FALSE);
        throw std::runtime_error("Test: XAML failed");
    }
    INT WINAPI CancelShow(HWND, const K7_DIALOG_MIRROR_ENGINE*, SUBCLASSPROC, LPVOID)
    {
        return 0; // Normal XAML loop termination without an accepted result.
    }
    INT WINAPI AcceptShow(HWND, const K7_DIALOG_MIRROR_ENGINE* engine,
        SUBCLASSPROC handler, LPVOID context)
    {
        if (!engine->PressOK(engine->Context))
            return -1;
        handler(nullptr, WM_COMMAND,
            MAKEWPARAM(K7_DIALOG_MIRROR_RESULT_OK, BN_CLICKED), 0, 1,
            reinterpret_cast<DWORD_PTR>(context));
        return 0;
    }
}

int main()
{
    g_hInstance = ::GetModuleHandleW(nullptr);
    HWND parent = ::CreateWindowExW(0, L"STATIC", L"test", WS_OVERLAPPED,
        0, 0, 100, 100, nullptr, nullptr, g_hInstance, nullptr);
    HWND combo = ::CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
        0, 0, 100, 100, parent, (HMENU)100, g_hInstance, nullptr);
    Check(parent && combo, "create isolated native controls");
    if (!parent || !combo) return 1;

    CMirrorContext context = {};
    context.Window = parent;
    const std::wstring longPath = L"D:\\" + std::wstring(900, L'a');
    ::SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)longPath.c_str());
    std::vector<wchar_t> slots(1100, L'Z');
    const UINT count = MirrorReadCombo(&context, 100, slots.data(), 1,
        nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    Check(count == 1 && slots[511] == L'\0' && slots[512] == L'Z',
        "long history item stays inside its 512-character ABI slot");

    ::SendMessageW(combo, CB_SETCURSEL, 0, 0);
    MirrorSetComboSelection(&context, 100, -1);
    Check(::SendMessageW(combo, CB_GETCURSEL, 0, 0) == 0,
        "custom text's negative selection cannot clear native state");
    MirrorSetComboSelection(&context, 100, 500);
    Check(::SendMessageW(combo, CB_GETCURSEL, 0, 0) == 0,
        "out-of-range selection cannot corrupt native state");
    MirrorSetComboText(&context, 100, L"100K");
    MirrorSetComboText(&context, 100, L"");
    wchar_t value[32] = {};
    ::GetWindowTextW(combo, value, 32);
    Check(value[0] == L'\0', "clearing optional split text is preserved");

    // Test the actual Modern snapshot reader with more than 64 thread entries
    // and a long editable destination, using the real native bridge callback.
    ::SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (int i = 0; i < 130; ++i)
    {
        const std::wstring item = std::to_wstring(i);
        ::SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    ::SendMessageW(combo, CB_SETCURSEL, 100, 0);
    K7_DIALOG_MIRROR_ENGINE engine = {};
    engine.Context = &context;
    engine.ReadCombo = MirrorReadCombo;
    engine.ReadText = MirrorReadText;
    namespace Ui = winrt::NanaZip::Modern::implementation::MirrorUi;
    auto snapshot = Ui::ReadCombo(&engine, 100);
    Check(snapshot.Ok && snapshot.Items.size() == 130 && snapshot.Selection == 100,
        "thread lists retain all entries and selections above 64");
    MirrorSetComboText(&context, 100, longPath.c_str());
    snapshot = Ui::ReadCombo(&engine, 100);
    Check(snapshot.Ok && snapshot.Text == longPath,
        "editable destination path longer than 511 characters is intact");
    auto textSnapshot = Ui::ReadText(&engine, 100);
    Check(textSnapshot.Ok && textSnapshot.Text == longPath,
        "text reader retains long paths");

    bool completed = false;
    TestDialog dialog(&completed);
    INT_PTR result = NModernDialogMirror::Show(parent, dialog, 2000, completed, FailShow);
    Check(result == 0 && ::IsWindowEnabled(parent),
        "XAML creation failure allows classic fallback and restores owner");
    Check(!::IsWindow(dialog.Created), "failed XAML disposes the hidden dialog");
    try
    {
        result = NModernDialogMirror::Show(parent, dialog, 2000, completed, ThrowShow);
        Check(result == 0 && ::IsWindowEnabled(parent),
            "XAML exception allows fallback and restores owner");
    }
    catch (...)
    {
        Check(false, "XAML exception allows fallback and restores owner");
        ::EnableWindow(parent, TRUE);
        ::DestroyWindow(dialog.Created);
    }
    result = NModernDialogMirror::Show(parent, dialog, 2000, completed, CancelShow);
    Check(result == IDCANCEL, "user cancellation stays cancellation");
    result = NModernDialogMirror::Show(parent, dialog, 2000, completed, AcceptShow);
    Check(result == IDOK && completed, "validated acceptance returns success");
    ::DestroyWindow(parent);
    std::printf("%d failure(s)\n", failures);
    return failures ? 1 : 0;
}

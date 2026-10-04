// Isolated Home regression host. No file-manager registry callbacks, settings,
// file associations, archive subprocesses, or native file pickers are invoked.
// Put this EXE beside the tested DLLs/resources.pri, or pass their directory.
#include <Windows.h>
#include <UIAutomation.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Resources.Core.h>

#include "../NanaZip.Modern/NanaZip.Modern.h"
#include "../NanaZip.Modern/StartPagePaths.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

namespace
{
    using namespace winrt::NanaZip::Modern::implementation;
    std::vector<std::wstring> g_RecentPaths;
    int g_Failures = 0;
    int g_LanguageSaveAttempts = 0;

    VOID CALLBACK FakeLanguagePersist(LPCWSTR)
    {
        ++g_LanguageSaveAttempts;
    }

    void Check(bool pass, char const* description)
    {
        std::printf("%s: %s\n", pass ? "PASS" : "FAIL", description);
        std::fflush(stdout);
        if (!pass)
            ++g_Failures;
    }

    void CheckPathTransfer()
    {
        std::vector<std::wstring> paths =
            { L"D:\\测试 文件.txt", L"D:\\folder\\second.bin" };
        std::size_t needed = 1;
        for (auto const& path : paths)
            needed += path.size() + 1;
        std::vector<wchar_t> buffer(needed + 1, L'X');
        INT32 count = -1;
        Check(TryWriteStartPaths(paths, buffer.data(), needed, count)
            && count == 2, "all selected Unicode paths fit exactly");
        Check(std::wstring(buffer.data()) == paths[0]
            && std::wstring(buffer.data() + paths[0].size() + 1) == paths[1]
            && buffer[needed - 1] == L'\0' && buffer[needed] == L'X',
            "NUL-separated round trip, double terminator and guard intact");

        std::fill(buffer.begin(), buffer.end(), L'X');
        Check(!TryWriteStartPaths(paths, buffer.data(), needed - 1, count)
            && count == 0 && buffer[0] == L'\0' && buffer[1] == L'X',
            "one-character overflow rejects whole selection, not its tail");
        Check(!TryWriteStartPaths({ std::wstring(65536, L'a') },
            buffer.data(), needed, count) && count == 0,
            "overly long path rejects safely");
        Check(!TryWriteStartPaths(paths, nullptr, 0, count) && count == 0,
            "missing output buffer rejects nonempty selection");
        Check(TryWriteStartPaths({}, buffer.data(), 1, count)
            && count == 0 && buffer[0] == L'\0', "empty selection is safe");
        Check(!TryWriteStartPaths({ L"" }, buffer.data(), needed, count)
            && count == 0, "empty path cannot terminate the list early");
        Check(!TryWriteStartPaths({ std::wstring(L"ab\0cd", 5) },
            buffer.data(), needed, count) && count == 0,
            "embedded NUL cannot corrupt the path list");
    }

    VOID WINAPI FakeRecent(K7_MODERN_START_RECENT_LIST* recent)
    {
        *recent = {};
        for (auto const& path : g_RecentPaths)
        {
            if (recent->Count >= K7_START_RECENT_MAX)
                break;
            wcsncpy_s(recent->Paths[recent->Count], path.c_str(), _TRUNCATE);
            ++recent->Count;
        }
    }

    BOOL CALLBACK FindHostWindow(HWND window, LPARAM param)
    {
        DWORD process = 0;
        ::GetWindowThreadProcessId(window, &process);
        if (process == ::GetCurrentProcessId() && ::IsWindowVisible(window))
        {
            *reinterpret_cast<HWND*>(param) = window;
            return FALSE;
        }
        return TRUE;
    }

    winrt::com_ptr<IUIAutomationElement> FindButton(
        IUIAutomation* automation, IUIAutomationElement* root,
        std::wstring const& name)
    {
        winrt::com_ptr<IUIAutomationCondition> nameCondition;
        winrt::com_ptr<IUIAutomationCondition> typeCondition;
        winrt::com_ptr<IUIAutomationCondition> condition;
        VARIANT value;
        ::VariantInit(&value);
        value.vt = VT_BSTR;
        value.bstrVal = ::SysAllocString(name.c_str());
        automation->CreatePropertyCondition(UIA_NamePropertyId, value,
            nameCondition.put());
        ::VariantClear(&value);
        value.vt = VT_I4;
        value.lVal = UIA_ButtonControlTypeId;
        automation->CreatePropertyCondition(UIA_ControlTypePropertyId, value,
            typeCondition.put());
        if (!nameCondition || !typeCondition)
            return nullptr;
        automation->CreateAndCondition(nameCondition.get(), typeCondition.get(),
            condition.put());
        winrt::com_ptr<IUIAutomationElement> element;
        if (condition)
            root->FindFirst(TreeScope_Descendants, condition.get(), element.put());
        return element;
    }

    bool Invoke(IUIAutomationElement* element)
    {
        if (!element)
            return false;
        winrt::com_ptr<IUIAutomationInvokePattern> pattern;
        if (FAILED(element->GetCurrentPatternAs(UIA_InvokePatternId,
            __uuidof(IUIAutomationInvokePattern), pattern.put_void())) || !pattern)
            return false;
        return SUCCEEDED(pattern->Invoke());
    }

    bool Enabled(IUIAutomationElement* element, bool expected)
    {
        BOOL enabled = FALSE;
        return element && SUCCEEDED(element->get_CurrentIsEnabled(&enabled))
            && (enabled != FALSE) == expected;
    }

    bool Visible(IUIAutomationElement* element)
    {
        BOOL offscreen = TRUE;
        RECT bounds = {};
        return element && SUCCEEDED(element->get_CurrentIsOffscreen(&offscreen))
            && !offscreen
            && SUCCEEDED(element->get_CurrentBoundingRectangle(&bounds))
            && bounds.right > bounds.left && bounds.bottom > bounds.top;
    }

    void CheckHomeUi(std::vector<std::wstring> names)
    {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
        winrt::com_ptr<IUIAutomation> automation;
        ::CoCreateInstance(CLSID_CUIAutomation, nullptr, CLSCTX_INPROC_SERVER,
            __uuidof(IUIAutomation), automation.put_void());
        HWND window = nullptr;
        winrt::com_ptr<IUIAutomationElement> root;
        winrt::com_ptr<IUIAutomationElement> create;
        for (int attempt = 0; attempt < 100 && !create; ++attempt)
        {
            ::EnumWindows(FindHostWindow, reinterpret_cast<LPARAM>(&window));
            if (window && automation)
            {
                root = nullptr;
                automation->ElementFromHandle(window, root.put());
                if (root)
                    create = FindButton(automation.get(), root.get(), names[0]);
            }
            if (!create)
                ::Sleep(100);
        }
        Check(Invoke(create.get()), "Create card opens input selection panel");
        ::Sleep(300);
        auto files = root ? FindButton(automation.get(), root.get(), names[1]) : nullptr;
        auto folders = root ? FindButton(automation.get(), root.get(), names[2]) : nullptr;
        auto remove = root ? FindButton(automation.get(), root.get(), names[3]) : nullptr;
        auto next = root ? FindButton(automation.get(), root.get(), names[4]) : nullptr;
        auto cancel = root ? FindButton(automation.get(), root.get(), names[5]) : nullptr;
        Check(Enabled(files.get(), true) && Visible(files.get())
            && Enabled(folders.get(), true) && Visible(folders.get()),
            "files and folders have visible enabled selection actions");
        Check(Enabled(remove.get(), false) && Enabled(next.get(), false)
            && Visible(remove.get()) && Visible(next.get()),
            "empty input disables Remove and Next");
        Check(Invoke(cancel.get()), "Cancel returns to Home without a file picker");
        ::Sleep(300);
        create = root ? FindButton(automation.get(), root.get(), names[0]) : nullptr;
        Check(Enabled(create.get(), true), "Home cards are usable after returning");
        if (window)
            ::PostMessageW(window, WM_CLOSE, 0, 0);
        winrt::uninit_apartment();
    }
}

int wmain(int argc, wchar_t** argv)
{
    CheckPathTransfer();
    if (argc == 2 && std::wstring(argv[1]) == L"--paths-only")
        return g_Failures ? 1 : 0;
    if (argc < 4)
    {
        std::puts("Usage: StartPageHost <runtime directory> <en|zh-Hans> <--strings|--langswitch|--ui> [recent paths...]");
        return 2;
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // P1-8: the requested language is applied after the runtime DLL is
    // loaded, via K7ModernSetAppLanguage (candidate-picking resolution;
    // no ResourceContext is involved anymore).

    std::wstring directory = argv[1];
    HMODULE module = ::LoadLibraryExW(
        (directory + L"\\NanaZip.Modern.dll").c_str(), nullptr,
        LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!module)
    {
        std::printf("FAIL: cannot load runtime, Win32 error %lu\n", GetLastError());
        return 2;
    }
    auto initialize = reinterpret_cast<decltype(&K7ModernInitialize)>(
        ::GetProcAddress(module, "K7ModernInitialize"));
    auto getString = reinterpret_cast<decltype(&K7ModernGetUiString)>(
        ::GetProcAddress(module, "K7ModernGetUiString"));
    auto getLegacy = reinterpret_cast<decltype(&K7ModernGetLegacyStringResource)>(
        ::GetProcAddress(module, "K7ModernGetLegacyStringResource"));
    auto show = reinterpret_cast<decltype(&K7ModernShowStartWindow)>(
        ::GetProcAddress(module, "K7ModernShowStartWindow"));
    auto uninitialize = reinterpret_cast<decltype(&K7ModernUninitialize)>(
        ::GetProcAddress(module, "K7ModernUninitialize"));
    auto setLanguage = reinterpret_cast<decltype(&K7ModernSetAppLanguage)>(
        ::GetProcAddress(module, "K7ModernSetAppLanguage"));
    auto setPersistCallback = reinterpret_cast<decltype(&K7ModernSetLanguagePersistCallback)>(
        ::GetProcAddress(module, "K7ModernSetLanguagePersistCallback"));
    if (!initialize || !getString || !getLegacy || !show || !uninitialize)
    {
        std::puts("FAIL: required exports are missing");
        return 2;
    }
    if (setLanguage)
    {
        if (setPersistCallback)
            setPersistCallback(FakeLanguagePersist);
        // "en" keeps English; the DLL resolves empty/system differently,
        // so pass through exactly what the caller asked for.
        setLanguage(std::wstring(argv[2]) == L"en" ? L"en-US" : argv[2]);
    }
    if (std::wstring(argv[3]) == L"--candidates")
    {
        auto map = winrt::Windows::ApplicationModel::Resources::Core::ResourceManager
            ::Current().MainResourceMap();
        auto resource = map.GetSubtree(L"NanaZip.Modern/StartPage")
            .Lookup(L"ActionCreateTitle/Text");
        for (auto candidate : resource.Candidates())
        {
            std::printf("value=%s\n", winrt::to_string(candidate.ValueAsString()).c_str());
            for (auto qualifier : candidate.Qualifiers())
                std::printf("  %s=%s\n", winrt::to_string(qualifier.QualifierName()).c_str(),
                    winrt::to_string(qualifier.QualifierValue()).c_str());
        }
        return 0;
    }
    bool chinese = std::wstring(argv[2]) == L"zh-Hans";
    auto matches = [](wchar_t const* actual, wchar_t const* expected)
    {
        if (actual && std::wstring(actual) != expected)
        {
            std::printf("  actual: [%s] expected: [%s]\n",
                winrt::to_string(actual).c_str(), winrt::to_string(expected).c_str());
            std::fflush(stdout);
        }
        return actual && std::wstring(actual) == expected;
    };
    Check(matches(getString(L"StartPage/ActionCreateTitle.Text", L"MISSING"),
        chinese ? L"创建压缩包" : L"Create Archive"),
        "Home strings honor the requested language context");
    Check(matches(getString(L"StartPage/AddFoldersText.Text", L"MISSING"),
        chinese ? L"添加文件夹" : L"Add folders"),
        "new selection action resolves from current resources.pri");
    Check(matches(getLegacy(3900), chinese ? L"已用时间：" : L"Elapsed time:"),
        "legacy dialog labels honor the requested language context");
    Check(g_LanguageSaveAttempts == 0,
        "applying runtime language never persists user preferences");

    if (std::wstring(argv[3]) == L"--langswitch" && setLanguage)
    {
        // CuinZip P1-8:语言切换链(context Languages + 缓存清理)回归。
        // 本模式自带英文起始 context(argv[2] = en),不依赖系统语言。
        Check(matches(getString(L"StartPage/ActionCreateTitle.Text",
            L"MISSING"), L"Create Archive"),
            "starts from the English context");
        Check(setLanguage(L"zh-Hans") != FALSE, "switch to zh-Hans succeeds");
        Check(matches(getString(L"StartPage/ActionCreateTitle.Text",
            L"MISSING"), L"创建压缩包"),
            "same key resolves in Chinese after switching");
        Check(matches(getLegacy(3900), L"已用时间："),
            "legacy labels follow the switch");
        Check(setLanguage(L"en-US") != FALSE, "switch back to en succeeds");
        Check(matches(getString(L"StartPage/ActionCreateTitle.Text",
            L"MISSING"), L"Create Archive"),
            "English resolvable again after switching back");
        setLanguage(L"zh-CN");
        Check(matches(getString(L"StartPage/ActionCreateTitle.Text",
            L"MISSING"), L"创建压缩包"),
            "zh-CN resolves Simplified Chinese script resources");
        Check(matches(getLegacy(3900), L"已用时间："),
            "zh-CN resolves legacy labels consistently");
        Check(g_LanguageSaveAttempts == 0,
            "repeated switches remain process-local");
        return g_Failures ? 1 : 0;
    }
    if (std::wstring(argv[3]) == L"--strings")
        return g_Failures ? 1 : 0;
    for (int i = 4; i < argc; ++i)
        g_RecentPaths.emplace_back(argv[i]);
    HRESULT initialized = initialize();
    if (FAILED(initialized))
    {
        std::printf("FAIL: XAML initialization returned %08lx\n", initialized);
        return 2;
    }
    std::vector<std::wstring> names;
    for (auto key : { L"ActionCreateTitle.Text", L"AddFilesText.Text",
        L"AddFoldersText.Text", L"RemoveFilesText.Text", L"CreateConfirmText.Text",
        L"CreateCancelText.Text" })
    {
        names.emplace_back(getString((std::wstring(L"StartPage/") + key).c_str(), L"MISSING"));
    }
    std::thread automationThread(CheckHomeUi, names);
    std::vector<wchar_t> paths(65536, L'\0');
    K7_MODERN_START_RESULT result = {};
    result.PathBuffer = paths.data();
    result.PathBufferCapacity = static_cast<UINT32>(paths.size());
    int shown = show(nullptr, FakeRecent, &result);
    automationThread.join();
    Check(shown != -1 && result.Action == K7_START_ACTION_NONE
        && result.PathCount == 0, "isolated Home closes without launching archive operations");
    uninitialize();
    winrt::uninit_apartment();
    return g_Failures ? 1 : 0;
}

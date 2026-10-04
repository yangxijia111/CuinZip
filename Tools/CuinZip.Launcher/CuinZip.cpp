// Copyright (c) CuinZip Project. Licensed under the MIT License.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <string>
#include <vector>
#include <iterator>

namespace
{
    bool UseChinese()
    {
        return PRIMARYLANGID(::GetUserDefaultUILanguage()) == LANG_CHINESE;
    }

    int ShowError(std::wstring const& Message)
    {
        ::MessageBoxW(nullptr, Message.c_str(), L"CuinZip", MB_OK | MB_ICONERROR);
        return 1;
    }

    std::wstring GetLauncherDirectory()
    {
        std::vector<wchar_t> Path(32768);
        DWORD Length = ::GetModuleFileNameW(nullptr, Path.data(),
            static_cast<DWORD>(Path.size()));
        if (Length == 0 || Length >= Path.size())
        {
            return {};
        }
        std::wstring Result(Path.data(), Length);
        size_t Separator = Result.find_last_of(L"\\/");
        return Separator == std::wstring::npos ? std::wstring()
            : Result.substr(0, Separator + 1);
    }

    bool IsFile(std::wstring const& Path)
    {
        DWORD Attributes = ::GetFileAttributesW(Path.c_str());
        return Attributes != INVALID_FILE_ATTRIBUTES
            && !(Attributes & FILE_ATTRIBUTE_DIRECTORY);
    }
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR Arguments, int)
{
    std::wstring Directory = GetLauncherDirectory();
    if (Directory.empty())
    {
        return ShowError(UseChinese()
            ? L"无法找到 CuinZip 所在文件夹。请重新解压完整的软件包后启动。"
            : L"Cannot locate the CuinZip folder. Extract the complete package again.");
    }

    std::wstring RuntimeDirectory = Directory + L"x64\\";
    constexpr wchar_t const* RequiredFiles[] = {
        L"NanaZip.Modern.FileManager.exe", L"NanaZip.Modern.dll",
        L"NanaZip.Core.dll", L"NanaZip.Codecs.dll", L"K7Base.dll", L"K7User.dll",
        L"NanaZip.Universal.Windows.exe", L"resources.pri",
        L"Mile.Xaml.Styles.SunValley.xbf"
    };
    std::wstring MissingFiles;
    for (auto File : RequiredFiles)
    {
        if (!IsFile(RuntimeDirectory + File))
        {
            MissingFiles += L"\n  x64\\";
            MissingFiles += File;
        }
    }
    if (!MissingFiles.empty())
    {
        return ShowError((UseChinese()
            ? L"CuinZip 文件不完整，缺少以下文件：\n"
            : L"The CuinZip package is incomplete. Missing files:\n")
            + MissingFiles + (UseChinese()
                ? L"\n\n请先把 ZIP 完整解压到文件夹，再双击 CuinZip.exe。"
                  L"\n请保留旁边的 x64 文件夹，不要只复制启动程序。"
                : L"\n\nExtract the entire ZIP into a folder, then run CuinZip.exe."
                  L"\nKeep the x64 folder beside the launcher; do not copy the launcher alone."));
    }

    std::wstring Executable = RuntimeDirectory + RequiredFiles[0];
    // wWinMain supplies the original command tail without argv[0]. Preserve its
    // quoting and the caller's working directory, including relative file paths.
    std::wstring CommandLine = L"\"" + Executable + L"\"";
    if (Arguments && *Arguments)
    {
        CommandLine += L" ";
        CommandLine += Arguments;
    }
    STARTUPINFOW Startup = {};
    Startup.cb = sizeof(Startup);
    PROCESS_INFORMATION Process = {};
    if (!::CreateProcessW(Executable.c_str(), CommandLine.data(), nullptr,
        nullptr, FALSE, 0, nullptr, nullptr, &Startup, &Process))
    {
        DWORD Error = ::GetLastError();
        wchar_t Detail[1024] = {};
        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, Error, 0, Detail, static_cast<DWORD>(std::size(Detail)), nullptr);
        return ShowError((UseChinese()
            ? L"CuinZip 启动失败。\n\nWindows 错误 "
            : L"CuinZip could not start.\n\nWindows error ")
            + std::to_wstring(Error) + L": " + Detail + (UseChinese()
                ? L"\n请确认软件包已完整解压，并查看安全软件是否隔离了程序文件。"
                : L"\nConfirm that the package was fully extracted and check whether"
                  L" your security software quarantined a program file."));
    }
    ::CloseHandle(Process.hThread);
    ::CloseHandle(Process.hProcess);
    return 0;
}

/*
 * PROJECT:    NanaZip
 * FILE:       NanaZip.ShellExtension.cpp
 * PURPOSE:    Implementation for NanaZip Shell Extension
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include <Windows.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <shobjidl_core.h>

#include <winrt/Windows.Foundation.h>

#include "../SevenZip/CPP/Common/Common.h"
#include "../SevenZip/CPP/Windows/DLL.h"
#include "../SevenZip/CPP/Windows/FileDir.h"
#include "../SevenZip/CPP/Windows/FileFind.h"
#include "../SevenZip/CPP/Windows/FileName.h"
#include "../SevenZip/CPP/Windows/ProcessUtils.h"
#include "../SevenZip/CPP/7zip/UI/Common/ArchiveName.h"
#include "../SevenZip/CPP/7zip/UI/Common/CompressCall.h"
#include "../SevenZip/CPP/7zip/UI/Common/ExtractingFilePath.h"
#include "../SevenZip/CPP/7zip/UI/Common/ZipRegistry.h"
#include "../SevenZip/CPP/7zip/UI/FileManager/FormatUtils.h"
#include "../SevenZip/CPP/7zip/UI/FileManager/LangUtils.h"
#include "../SevenZip/CPP/7zip/UI/Explorer/ContextMenuFlags.h"
#include "../SevenZip/CPP/7zip/UI/Explorer/resource.h"

// **************** CuinZip P1-4 Modification Start ****************
// 命令 ID 枚举前置:匿名命名空间中的选择分析 / 标题 / 显隐辅助函数需要引用。
namespace NanaZip::ShellExtension
{
    namespace CommandID
    {
        enum
        {
            None,

            Open,
            Test,

            Extract,
            ExtractHere,
            ExtractHereSmart,
            ExtractTo,

            Compress,
            CompressTo7z,
            CompressToZip,

            CompressEmail,
            CompressTo7zEmail,
            CompressToZipEmail,

            HashCRC32,
            HashCRC64,
            HashSHA1,
            HashSHA256,
            HashAll,

            Maximum
        };
    }
}
// **************** CuinZip P1-4 Modification End ****************

namespace
{
    static const char* const kExtractExcludeExtensions =
        " 3gp"
        " aac ans ape asc asm asp aspx avi awk"
        " bas bat bmp"
        " c cs cls clw cmd cpp csproj css ctl cxx"
        " def dep dlg dsp dsw"
        " eps"
        " f f77 f90 f95 fla flac frm"
        " gif"
        " h hpp hta htm html hxx"
        " ico idl inc ini inl"
        " java jpeg jpg js"
        " la lnk log"
        " mak manifest wmv mov mp3 mp4 mpe mpeg mpg m4a"
        " ofr ogg"
        " pac pas pdf php php3 php4 php5 phptml pl pm png ps py pyo"
        " ra rb rc reg rka rm rtf"
        " sed sh shn shtml sln sql srt swa"
        " tcl tex tiff tta txt"
        " vb vcproj vbs"
        " wav webp wma wv"
        " xml xsd xsl xslt"
        " ";

    static bool FindExt(const char* p, const FString& name)
    {
        int dotPos = name.ReverseFind_Dot();
        if (dotPos < 0 || dotPos == (int)name.Len() - 1)
            return false;

        AString s;

        for (unsigned pos = dotPos + 1;; pos++)
        {
            wchar_t c = name[pos];
            if (c == 0)
                break;
            if (c >= 0x80)
                return false;
            s += (char)MyCharLower_Ascii((char)c);
        }

        for (unsigned i = 0; p[i] != 0;)
        {
            unsigned j;
            for (j = i; p[j] != ' '; j++);
            if (s.Len() == j - i && memcmp(p + i, (const char*)s, s.Len()) == 0)
                return true;
            i = j + 1;
        }

        return false;
    }

    static bool DoNeedExtract(const FString& name)
    {
        return !FindExt(kExtractExcludeExtensions, name);
    }

    static const char* const kArcExts[] =
    {
        "7z"
      , "bz2"
      , "gz"
      , "lz"
      , "liz"
      , "lz4"
      , "lz5"
      , "rar"
      , "zip"
      , "zst"
    };

    static bool IsItArcExt(const UString& ext)
    {
        for (unsigned i = 0; i < ARRAY_SIZE(kArcExts); i++)
            if (ext.IsEqualTo_Ascii_NoCase(kArcExts[i]))
                return true;
        return false;
    }

    UString GetSubFolderNameForExtract(const UString& arcName)
    {
        int dotPos = arcName.ReverseFind_Dot();
        if (dotPos < 0)
            return Get_Correct_FsFile_Name(arcName) + L'~';

        const UString ext = arcName.Ptr(dotPos + 1);
        UString res = arcName.Left(dotPos);
        res.TrimRight();
        dotPos = res.ReverseFind_Dot();
        if (dotPos > 0)
        {
            const UString ext2 = res.Ptr(dotPos + 1);
            if ((ext.IsEqualTo_Ascii_NoCase("001") && IsItArcExt(ext2))
                || (ext.IsEqualTo_Ascii_NoCase("rar") &&
                    (ext2.IsEqualTo_Ascii_NoCase("part001")
                        || ext2.IsEqualTo_Ascii_NoCase("part01")
                        || ext2.IsEqualTo_Ascii_NoCase("part1"))))
                res.DeleteFrom(dotPos);
            res.TrimRight();
        }
        return Get_Correct_FsFile_Name(res);
    }

    static void ReduceString(UString& s)
    {
        const unsigned kMaxSize = 60;
        if (s.Len() <= kMaxSize)
            return;
        s.Delete(kMaxSize / 2, s.Len() - kMaxSize);
        s.Insert(kMaxSize / 2, L" ... ");
    }

    // Backport GetQuotedReducedString from 7-Zip ZS since GetQuotedString
    // now does string escaping as well.
    static UString GetQuotedReducedString(const UString& s)
    {
        UString s2 = s;
        ReduceString(s2);
        s2.Replace(L"&", L"&&");
        s2.InsertAtFront(L'"');
        s2 += L'"'; // quote without GetQuotedString (because it escapes now)
        return s2;
    }

    static void MyFormatNew_ReducedName(UString& s, const UString& name)
    {
        s = MyFormatNew(s, GetQuotedReducedString(name));
    }

    static UString GetNanaZipPath()
    {
        return fs2us(NWindows::NDLL::GetModuleDirPrefix()) + L"NanaZip.Modern.FileManager.exe";
    }

    // **************** CuinZip P1-4 Modification Start ****************
    // 选择分析:一次收集供 GetState / GetTitle / Invoke 复用的事实
    // (文件清单、是否含压缩包、解压目标子文件夹名、压缩包基名与所在目录)。
    struct SelectionInfo
    {
        std::vector<std::wstring> FilePaths;
        bool NeedExtract = false;
        std::wstring SpecFolder = L"*";   // "Name\" 或 "*\"(多选/混合时)
        std::wstring BaseFolder;
        std::wstring ArchiveName;         // 不含扩展名
        bool Valid = false;
    };

    void CollectFilePaths(
        _In_opt_ IShellItemArray* psiItemArray,
        std::vector<std::wstring>& FilePaths)
    {
        if (!psiItemArray)
        {
            return;
        }
        DWORD Count = 0;
        if (FAILED(psiItemArray->GetCount(&Count)))
        {
            return;
        }
        for (DWORD i = 0; i < Count; ++i)
        {
            winrt::com_ptr<IShellItem> Item;
            if (SUCCEEDED(psiItemArray->GetItemAt(i, Item.put())))
            {
                LPWSTR DisplayName = nullptr;
                if (SUCCEEDED(Item->GetDisplayName(
                    SIGDN_FILESYSPATH,
                    &DisplayName)))
                {
                    FilePaths.push_back(std::wstring(DisplayName));
                    ::CoTaskMemFree(DisplayName);
                }
            }
        }
    }

    SelectionInfo AnalyzeSelection(_In_opt_ IShellItemArray* psiItemArray)
    {
        SelectionInfo Info;
        CollectFilePaths(psiItemArray, Info.FilePaths);
        if (Info.FilePaths.empty())
        {
            return Info;
        }

        for (std::wstring const& FilePath : Info.FilePaths)
        {
            DWORD FileAttributes = ::GetFileAttributesW(FilePath.c_str());
            if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }
            if (DoNeedExtract(::PathFindFileNameW(FilePath.c_str())))
            {
                Info.NeedExtract = true;
                break;
            }
        }

        if (Info.NeedExtract)
        {
            if (Info.FilePaths.size() == 1)
            {
                Info.SpecFolder = GetSubFolderNameForExtract(
                    ::PathFindFileNameW(Info.FilePaths[0].c_str()));
            }
            Info.SpecFolder += L'\\';
        }

        UStringVector FileNames;
        for (std::wstring const& FilePath : Info.FilePaths)
        {
            FileNames.Add(FilePath.c_str());
        }

        NWindows::NFile::NFind::CFileInfo FileInfo0;
        const UString& FileName = FileNames.Front();

        if (NWindows::NFile::NName::IsDevicePath(us2fs(FileName)))
        {
            // CFileInfo::Find can be slow for device files. So we don't call
            // it. We need only name here.
            FileInfo0.Name = us2fs(FileName.Ptr(
                NWindows::NFile::NName::kDevicePathPrefixSize));
            Info.BaseFolder = L"C:\\";
        }
        else
        {
            if (!FileInfo0.Find(us2fs(FileName)))
            {
                return Info;
            }
            FString FolderPrefix;
            NWindows::NFile::NDir::GetOnlyDirPrefix(
                us2fs(FileName),
                FolderPrefix);
            Info.BaseFolder = std::wstring(FolderPrefix.Ptr(), FolderPrefix.Len());
        }

        const UString Name = CreateArchiveName(
            FileNames,
            FileNames.Size() == 1 ? &FileInfo0 : nullptr);
        Info.ArchiveName = std::wstring(Name.Ptr(), Name.Len());
        Info.Valid = true;
        return Info;
    }

    // CuinZip P1-4:平铺(非二级菜单)动词的本地化标题。二级子菜单内的
    // Open 沿用 IDS_CONTEXT_OPEN("Open archive"),平铺时用
    // IDS_CONTEXT_OPEN_WITH("Open with CuinZip")语义更清晰。
    std::wstring GetLocalizedFlatTitle(
        DWORD CommandID,
        SelectionInfo const& Info)
    {
        LoadLangOneTime();

        UString TranslatedString;
        switch (CommandID)
        {
        case NanaZip::ShellExtension::CommandID::Open:
            LangString(IDS_CONTEXT_OPEN_WITH, TranslatedString);
            break;
        case NanaZip::ShellExtension::CommandID::ExtractHere:
            LangString(IDS_CONTEXT_EXTRACT_HERE, TranslatedString);
            break;
        case NanaZip::ShellExtension::CommandID::ExtractTo:
            LangString(IDS_CONTEXT_EXTRACT_TO, TranslatedString);
            MyFormatNew_ReducedName(TranslatedString, Info.SpecFolder.c_str());
            break;
        case NanaZip::ShellExtension::CommandID::Compress:
            LangString(IDS_CONTEXT_COMPRESS, TranslatedString);
            break;
        case NanaZip::ShellExtension::CommandID::CompressTo7z:
            LangString(IDS_CONTEXT_COMPRESS_TO, TranslatedString);
            MyFormatNew_ReducedName(
                TranslatedString,
                (Info.ArchiveName + L".7z").c_str());
            break;
        case NanaZip::ShellExtension::CommandID::CompressToZip:
            LangString(IDS_CONTEXT_COMPRESS_TO, TranslatedString);
            MyFormatNew_ReducedName(
                TranslatedString,
                (Info.ArchiveName + L".zip").c_str());
            break;
        default:
            break;
        }
        return std::wstring(TranslatedString.Ptr(), TranslatedString.Len());
    }

    // 平铺动词是否适用于当前选择(与二级子菜单的过滤规则一致)。
    bool IsFlatCommandApplicable(DWORD CommandID, SelectionInfo const& Info)
    {
        if (!Info.Valid)
        {
            return false;
        }
        switch (CommandID)
        {
        case NanaZip::ShellExtension::CommandID::Open:
            return Info.NeedExtract && Info.FilePaths.size() == 1;
        case NanaZip::ShellExtension::CommandID::ExtractHere:
        case NanaZip::ShellExtension::CommandID::ExtractTo:
            return Info.NeedExtract;
        case NanaZip::ShellExtension::CommandID::Compress:
        case NanaZip::ShellExtension::CommandID::CompressTo7z:
        case NanaZip::ShellExtension::CommandID::CompressToZip:
            return true;
        default:
            return false;
        }
    }

    // 平铺动词对应的设置开关(ContextMenu 设置页驱动)。
    bool IsFlatCommandEnabledBySettings(
        DWORD CommandID,
        CContextMenuInfo const& Info)
    {
        switch (CommandID)
        {
        case NanaZip::ShellExtension::CommandID::Open:
            return (Info.Flags & NContextMenuFlags::kOpen) != 0;
        case NanaZip::ShellExtension::CommandID::ExtractHere:
            return (Info.Flags & NContextMenuFlags::kExtractHere) != 0;
        case NanaZip::ShellExtension::CommandID::ExtractTo:
            return (Info.Flags & NContextMenuFlags::kExtractTo) != 0;
        case NanaZip::ShellExtension::CommandID::Compress:
            return (Info.Flags & NContextMenuFlags::kCompress) != 0;
        case NanaZip::ShellExtension::CommandID::CompressTo7z:
            return (Info.Flags & NContextMenuFlags::kCompressTo7z) != 0;
        case NanaZip::ShellExtension::CommandID::CompressToZip:
            return (Info.Flags & NContextMenuFlags::kCompressToZip) != 0;
        default:
            return false;
        }
    }

    // CuinZip P1-4:平铺动词 CLSID 表(全新 GUID,不与上游 NanaZip 复用,
    // 保证 CuinZip 与 NanaZip 可共存)。
    struct FlatVerbEntry
    {
        GUID const& Clsid;
        DWORD CommandID;
    };

    GUID const kFlatOpenClsid =
    {
        0x9EE110B9, 0x828B, 0x4B34,
        { 0xB6, 0x31, 0x20, 0xEE, 0xD2, 0x99, 0x94, 0x0E }
    };
    GUID const kFlatExtractHereClsid =
    {
        0x3FCAFA2A, 0xD2C0, 0x4D3F,
        { 0xBD, 0xC3, 0xDD, 0x63, 0x09, 0x2D, 0xD4, 0x67 }
    };
    GUID const kFlatExtractToClsid =
    {
        0xEEEA627E, 0xE069, 0x449D,
        { 0x85, 0x46, 0xB7, 0x09, 0x4E, 0x28, 0xC7, 0x90 }
    };
    GUID const kFlatCompressClsid =
    {
        0x40E45AB6, 0x96D5, 0x470B,
        { 0x8C, 0x67, 0xB0, 0x52, 0x01, 0x1E, 0xE4, 0x4C }
    };
    GUID const kFlatCompressTo7zClsid =
    {
        0xAD18A991, 0x08C6, 0x43A1,
        { 0xAA, 0xBB, 0x29, 0x53, 0xD8, 0xE4, 0x9A, 0x0C }
    };
    GUID const kFlatCompressToZipClsid =
    {
        0x95BA5FBD, 0xADD7, 0x43A0,
        { 0x8F, 0xB3, 0x63, 0xE0, 0x70, 0x4C, 0x74, 0xD2 }
    };

    FlatVerbEntry const kFlatVerbs[] =
    {
        { kFlatOpenClsid,          NanaZip::ShellExtension::CommandID::Open },
        { kFlatExtractHereClsid,   NanaZip::ShellExtension::CommandID::ExtractHere },
        { kFlatExtractToClsid,     NanaZip::ShellExtension::CommandID::ExtractTo },
        { kFlatCompressClsid,      NanaZip::ShellExtension::CommandID::Compress },
        { kFlatCompressTo7zClsid,  NanaZip::ShellExtension::CommandID::CompressTo7z },
        { kFlatCompressToZipClsid, NanaZip::ShellExtension::CommandID::CompressToZip },
    };
    // **************** CuinZip P1-4 Modification End ****************
}

namespace NanaZip::ShellExtension
{
    using SubCommandList = std::vector<winrt::com_ptr<IExplorerCommand>>;
    using SubCommandListIterator = SubCommandList::const_iterator;

    struct ExplorerCommandBase : public winrt::implements<
        ExplorerCommandBase,
        IExplorerCommand>
    {
    private:

        std::wstring m_Title;

        DWORD m_CommandID;
        bool m_IsSeparator;
        // **************** CuinZip P1-4 Modification Start ****************
        // true 表示平铺动词(直接出现在一级右键菜单),其显隐与标题由
        // 当前选择 + Context Menu 设置实时决定。
        bool m_IsFlat;
        // **************** CuinZip P1-4 Modification End ****************
        CBoolPair m_ElimDup;
        UInt32 m_WriteZone;

    public:

        ExplorerCommandBase(
            std::wstring const& Title = std::wstring(),
            DWORD CommandID = CommandID::None,
            CBoolPair const& ElimDup = CBoolPair(),
            UInt32 const& WriteZone = static_cast<UInt32>(-1),
            // **************** CuinZip P1-4 Modification Start ****************
            bool IsFlat = false) :
            // **************** CuinZip P1-4 Modification End ****************
            m_Title(Title),
            m_CommandID(CommandID),
            m_ElimDup(ElimDup),
            m_WriteZone(WriteZone),
            // **************** CuinZip P1-4 Modification Start ****************
            m_IsFlat(IsFlat)
            // **************** CuinZip P1-4 Modification End ****************
        {
            this->m_IsSeparator = (this->m_CommandID == CommandID::None);
        }

#pragma region IExplorerCommand

        HRESULT STDMETHODCALLTYPE GetTitle(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszName)
        {
            // **************** CuinZip P1-4 Modification Start ****************
            // 平铺动词的标题依赖当前选择(解压目标/压缩包名),实时计算。
            if (this->m_IsFlat)
            {
                SelectionInfo Info = AnalyzeSelection(psiItemArray);
                std::wstring Title = GetLocalizedFlatTitle(this->m_CommandID, Info);
                if (Title.empty())
                {
                    *ppszName = nullptr;
                    return E_NOTIMPL;
                }
                return ::SHStrDupW(Title.c_str(), ppszName);
            }
            // **************** CuinZip P1-4 Modification End ****************

            UNREFERENCED_PARAMETER(psiItemArray);

            if (this->m_IsSeparator)
            {
                *ppszName = nullptr;
                return S_FALSE;
            }

            return ::SHStrDupW(this->m_Title.c_str(), ppszName);
        }

        HRESULT STDMETHODCALLTYPE GetIcon(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszIcon)
        {
            UNREFERENCED_PARAMETER(psiItemArray);

            // **************** CuinZip P1-4 Modification Start ****************
            // 平铺动词显示 CuinZip 文件管理器图标(与二级菜单根项一致)。
            if (this->m_IsFlat)
            {
                UString Path = ::GetNanaZipPath();
                std::wstring Icon = std::wstring(Path.Ptr(), Path.Len());
                Icon += L",-1";
                return ::SHStrDupW(Icon.c_str(), ppszIcon);
            }
            // **************** CuinZip P1-4 Modification End ****************

            *ppszIcon = nullptr;
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetToolTip(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszInfotip)
        {
            UNREFERENCED_PARAMETER(psiItemArray);
            *ppszInfotip = nullptr;
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetCanonicalName(
            _Out_ GUID* pguidCommandName)
        {
            *pguidCommandName = GUID_NULL;
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetState(
            _In_opt_ IShellItemArray* psiItemArray,
            _In_ BOOL fOkToBeSlow,
            _Out_ EXPCMDSTATE* pCmdState)
        {
            UNREFERENCED_PARAMETER(fOkToBeSlow);

            // **************** CuinZip P1-4 Modification Start ****************
            // 平铺动词:二级菜单模式开启、对应设置关闭或不适于当前选择时隐藏。
            if (this->m_IsFlat)
            {
                CContextMenuInfo ContextMenuInfo;
                ContextMenuInfo.Load();
                if (ContextMenuInfo.Cascaded.Val)
                {
                    *pCmdState = ECS_HIDDEN;
                    return S_OK;
                }
                if (!IsFlatCommandEnabledBySettings(this->m_CommandID, ContextMenuInfo))
                {
                    *pCmdState = ECS_HIDDEN;
                    return S_OK;
                }
                SelectionInfo Info = AnalyzeSelection(psiItemArray);
                *pCmdState = IsFlatCommandApplicable(this->m_CommandID, Info)
                    ? ECS_ENABLED
                    : ECS_HIDDEN;
                return S_OK;
            }
            // **************** CuinZip P1-4 Modification End ****************

            UNREFERENCED_PARAMETER(psiItemArray);
            *pCmdState = ECS_ENABLED;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Invoke(
            _In_opt_ IShellItemArray* psiItemArray,
            _In_opt_ IBindCtx* pbc)
        {
            UNREFERENCED_PARAMETER(pbc);

            if (this->m_IsSeparator)
            {
                return E_NOTIMPL;
            }

            std::vector<std::wstring> FilePaths;
            if (psiItemArray)
            {
                DWORD Count = 0;
                if (SUCCEEDED(psiItemArray->GetCount(&Count)))
                {
                    for (DWORD i = 0; i < Count; ++i)
                    {
                        winrt::com_ptr<IShellItem> Item;
                        if (SUCCEEDED(psiItemArray->GetItemAt(
                            i,
                            Item.put())))
                        {
                            LPWSTR DisplayName = nullptr;
                            if (SUCCEEDED(Item->GetDisplayName(
                                SIGDN_FILESYSPATH,
                                &DisplayName)))
                            {
                                FilePaths.push_back(std::wstring(DisplayName));
                                ::CoTaskMemFree(DisplayName);
                            }
                        }
                    }
                }
            }

            bool NeedExtract = false;
            if (FilePaths.size() > 0)
            {
                for (std::wstring const FilePath : FilePaths)
                {
                    DWORD FileAttributes = ::GetFileAttributesW(
                        FilePath.c_str());
                    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        continue;
                    }

                    if (DoNeedExtract(::PathFindFileNameW(FilePath.c_str())))
                    {
                        NeedExtract = true;
                        break;
                    }
                }
            }

            std::wstring SpecFolder = L"*";
            if (NeedExtract)
            {
                if (FilePaths.size() == 1)
                {
                    SpecFolder = GetSubFolderNameForExtract(
                        ::PathFindFileNameW(FilePaths[0].c_str()));
                }
                SpecFolder += L'\\';
            }

            UStringVector FileNames;
            for (std::wstring const FilePath : FilePaths)
            {
                FileNames.Add(FilePath.c_str());
            }

            FString FolderPrefix;

            std::wstring ArchiveName;
            if (FilePaths.size() > 0)
            {
                NWindows::NFile::NFind::CFileInfo FileInfo0;

                const UString& FileName = FileNames.Front();

                if (NWindows::NFile::NName::IsDevicePath(us2fs(FileName)))
                {
                    // CFileInfo::Find can be slow for device files. So we
                    // don't call it.
                    // we need only name here.
                    // change it 4 - must be constant
                    FileInfo0.Name = us2fs(FileName.Ptr(
                        NWindows::NFile::NName::kDevicePathPrefixSize));
                    FolderPrefix = "C:\\";
                }
                else
                {
                    if (!FileInfo0.Find(us2fs(FileName)))
                    {
                        return ::HRESULT_FROM_WIN32(::GetLastError());
                    }
                    NWindows::NFile::NDir::GetOnlyDirPrefix(
                        us2fs(FileName),
                        FolderPrefix);
                }

                const UString Name = CreateArchiveName(
                    FileNames,
                    FileNames.Size() == 1 ? &FileInfo0 : nullptr);
                ArchiveName = std::wstring(Name.Ptr(), Name.Len());

            }

            std::wstring BaseFolder = std::wstring(
                FolderPrefix.Ptr(),
                FolderPrefix.Len());

            std::wstring ArchiveName7z = ArchiveName + L".7z";
            std::wstring ArchiveNameZip = ArchiveName + L".zip";

            switch (this->m_CommandID)
            {
            case CommandID::Open:
            {
                if (FilePaths.size() != 1)
                {
                    break;
                }

                DWORD FileAttributes = ::GetFileAttributesW(
                    FilePaths[0].c_str());
                if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    break;
                }

                if (!DoNeedExtract(FilePaths[0].c_str()))
                {
                    break;
                }

                UString params;
                params = GetQuotedString(FilePaths[0].c_str());
                NWindows::MyCreateProcess(::GetNanaZipPath(), params);

                break;
            }
            case CommandID::Test:
            {
                if (!NeedExtract)
                {
                    break;
                }

                TestArchives(FileNames);
                break;
            }
            case CommandID::Extract:
            case CommandID::ExtractHere:
            case CommandID::ExtractHereSmart:
            case CommandID::ExtractTo:
            {
                if (!NeedExtract)
                {
                    break;
                }

                std::wstring Folder = BaseFolder;
                if (this->m_CommandID != CommandID::ExtractHere &&
                    this->m_CommandID != CommandID::ExtractHereSmart)
                {
                    Folder += SpecFolder;
                }

                ExtractArchives(
                    FileNames,
                    Folder.c_str(),
                    (this->m_CommandID == CommandID::Extract),
                    ((this->m_CommandID == CommandID::ExtractTo)
                    && this->m_ElimDup.Val),
                    this->m_WriteZone,
                    (this->m_CommandID == CommandID::ExtractHereSmart));

                break;
            }
            case CommandID::Compress:
            case CommandID::CompressTo7z:
            case CommandID::CompressToZip:
            case CommandID::CompressEmail:
            case CommandID::CompressTo7zEmail:
            case CommandID::CompressToZipEmail:
            {
                bool Email =(
                    (this->m_CommandID == CommandID::CompressEmail) ||
                    (this->m_CommandID == CommandID::CompressTo7zEmail) ||
                    (this->m_CommandID == CommandID::CompressToZipEmail));
                bool ShowDialog = (
                    (this->m_CommandID == CommandID::Compress) ||
                    (this->m_CommandID == CommandID::CompressEmail));
                bool AddExtension = (
                    (this->m_CommandID == CommandID::Compress) ||
                    (this->m_CommandID == CommandID::CompressEmail));
                bool Is7z = (
                    (this->m_CommandID == CommandID::CompressTo7z) ||
                    (this->m_CommandID == CommandID::CompressTo7zEmail));

                std::wstring Name = (
                    AddExtension
                    ? ArchiveName
                    : (Is7z ? ArchiveName7z : ArchiveNameZip));

                CompressFiles(
                    BaseFolder.c_str(),
                    Name.c_str(),
                    Is7z ? L"7z" : L"zip",
                    AddExtension,
                    FileNames,
                    Email,
                    ShowDialog,
                    false);

                break;
            }
            case CommandID::HashCRC32:
            case CommandID::HashCRC64:
            case CommandID::HashSHA1:
            case CommandID::HashSHA256:
            case CommandID::HashAll:
            {
                std::wstring MethodName;
                switch (this->m_CommandID)
                {
                case CommandID::HashCRC32:
                    MethodName = L"CRC32";
                    break;
                case CommandID::HashCRC64:
                    MethodName = L"CRC64";
                    break;
                case CommandID::HashSHA1:
                    MethodName = L"SHA1";
                    break;
                case CommandID::HashSHA256:
                    MethodName = L"SHA256";
                    break;
                case CommandID::HashAll:
                    MethodName = L"*";
                    break;
                default:
                    break;
                }

                CalcChecksum(FileNames, MethodName.c_str(), L"", L"");
                break;
            }
            default:
                break;
            }

            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetFlags(
            _Out_ EXPCMDFLAGS* pFlags)
        {
            *pFlags =
                this->m_IsSeparator
                ? ECF_ISSEPARATOR
                : ECF_DEFAULT;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE EnumSubCommands(
            _Outptr_ IEnumExplorerCommand** ppEnum)
        {
            *ppEnum = nullptr;
            return E_NOTIMPL;
        }

#pragma endregion
    };


    struct ExplorerCommandRoot : public winrt::implements<
        ExplorerCommandRoot,
        IExplorerCommand,
        IEnumExplorerCommand>
    {
    private:

        DWORD m_ContextMenuFlags;
        CBoolPair m_ContextMenuElimDup;

        bool m_Initialized = false;
        SubCommandList m_SubCommands;
        SubCommandListIterator m_CurrentSubCommand;

        void Initialize(
            _In_opt_ IShellItemArray* psiItemArray)
        {
            if (m_Initialized)
            {
                return;
            }

            m_Initialized = true;

            std::vector<std::wstring> FilePaths;
            if (psiItemArray)
            {
                DWORD Count = 0;
                if (SUCCEEDED(psiItemArray->GetCount(&Count)))
                {
                    for (DWORD i = 0; i < Count; ++i)
                    {
                        winrt::com_ptr<IShellItem> Item;
                        if (SUCCEEDED(psiItemArray->GetItemAt(
                            i,
                            Item.put())))
                        {
                            LPWSTR DisplayName = nullptr;
                            if (SUCCEEDED(Item->GetDisplayName(
                                SIGDN_FILESYSPATH,
                                &DisplayName)))
                            {
                                FilePaths.push_back(std::wstring(DisplayName));
                                ::CoTaskMemFree(DisplayName);
                            }
                        }
                    }
                }
            }

            if (FilePaths.empty())
            {
                return;
            }

            UStringVector FileNames;
            for (std::wstring const FilePath : FilePaths)
            {
                FileNames.Add(FilePath.c_str());
            }

            bool NeedExtract = false;
            for (std::wstring const FilePath : FilePaths)
            {
                DWORD FileAttributes = ::GetFileAttributesW(
                    FilePath.c_str());
                if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    continue;
                }

                if (DoNeedExtract(::PathFindFileNameW(FilePath.c_str())))
                {
                    NeedExtract = true;
                    break;
                }
            }

            std::wstring SpecFolder = L"*";
            if (NeedExtract)
            {
                if (FilePaths.size() == 1)
                {
                    SpecFolder = GetSubFolderNameForExtract(
                        ::PathFindFileNameW(FilePaths[0].c_str()));
                }
                SpecFolder += L'\\';
            }

            FString FolderPrefix;

            std::wstring ArchiveName;
            {
                NWindows::NFile::NFind::CFileInfo FileInfo0;

                const UString& FileName = FileNames.Front();

                if (NWindows::NFile::NName::IsDevicePath(us2fs(FileName)))
                {
                    // CFileInfo::Find can be slow for device files. So we
                    // don't call it.
                    // we need only name here.
                    // change it 4 - must be constant
                    FileInfo0.Name = us2fs(FileName.Ptr(
                        NWindows::NFile::NName::kDevicePathPrefixSize));
                    FolderPrefix = "C:\\";
                }
                else
                {
                    if (!FileInfo0.Find(us2fs(FileName)))
                    {
                        return;
                    }
                    NWindows::NFile::NDir::GetOnlyDirPrefix(
                        us2fs(FileName),
                        FolderPrefix);
                }

                const UString Name = CreateArchiveName(
                    FileNames,
                    FileNames.Size() == 1 ? &FileInfo0 : nullptr);
                ArchiveName = std::wstring(Name.Ptr(), Name.Len());

            }

            std::wstring BaseFolder = std::wstring(
                FolderPrefix.Ptr(),
                FolderPrefix.Len());

            std::wstring ArchiveName7z = ArchiveName + L".7z";
            std::wstring ArchiveNameZip = ArchiveName + L".zip";

            using NanaZip::ShellExtension::ExplorerCommandBase;

            CContextMenuInfo ContextMenuInfo;
            ContextMenuInfo.Load();
            DWORD ContextMenuFlags = ContextMenuInfo.Flags;
            CBoolPair ContextMenuElimDup = ContextMenuInfo.ElimDup;
            UInt32 ContextMenuWriteZone = ContextMenuInfo.WriteZone;

            LoadLangOneTime();

            if (ContextMenuFlags & NContextMenuFlags::kOpen)
            {
                DWORD FileAttributes = ::GetFileAttributesW(
                    FilePaths[0].c_str());
                if ((FilePaths.size() == 1) &&
                    !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    DoNeedExtract(FilePaths[0].c_str()))
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_OPEN, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::Open));
                }
            }

            if (NeedExtract)
            {
                if (ContextMenuFlags & NContextMenuFlags::kTest)
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_TEST, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::Test));
                }

                if (ContextMenuFlags & NContextMenuFlags::kExtract)
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_EXTRACT, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::Extract,
                            ContextMenuElimDup,
                            ContextMenuWriteZone));
                }

                if (ContextMenuFlags & NContextMenuFlags::kExtractHere)
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_EXTRACT_HERE, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::ExtractHere,
                            ContextMenuElimDup,
                            ContextMenuWriteZone));
                }

                if (ContextMenuFlags & NContextMenuFlags::kExtractHereSmart)
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_EXTRACT_HERE_SMART, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::ExtractHereSmart,
                            ContextMenuElimDup,
                            ContextMenuWriteZone));
                }

                if (ContextMenuFlags & NContextMenuFlags::kExtractTo)
                {
                    UString TranslatedString;
                    LangString(IDS_CONTEXT_EXTRACT_TO, TranslatedString);
                    MyFormatNew_ReducedName(TranslatedString, SpecFolder.c_str());
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            CommandID::ExtractTo,
                            ContextMenuElimDup,
                            ContextMenuWriteZone));
                }
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompress)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS, TranslatedString);
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::Compress));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompressTo7z)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS_TO, TranslatedString);
                MyFormatNew_ReducedName(TranslatedString, ArchiveName7z.c_str());
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::CompressTo7z));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompressToZip)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS_TO, TranslatedString);
                MyFormatNew_ReducedName(TranslatedString, ArchiveNameZip.c_str());
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::CompressToZip));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompressEmail)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS_EMAIL, TranslatedString);
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::CompressEmail));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompressTo7zEmail)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS_TO_EMAIL, TranslatedString);
                MyFormatNew_ReducedName(TranslatedString, ArchiveName7z.c_str());
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::CompressTo7zEmail));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCompressToZipEmail)
            {
                UString TranslatedString;
                LangString(IDS_CONTEXT_COMPRESS_TO_EMAIL, TranslatedString);
                MyFormatNew_ReducedName(TranslatedString, ArchiveNameZip.c_str());
                this->m_SubCommands.push_back(
                    winrt::make<ExplorerCommandBase>(
                        std::wstring(
                            TranslatedString.Ptr(),
                            TranslatedString.Len()),
                        CommandID::CompressToZipEmail));
            }

            if (ContextMenuFlags & NContextMenuFlags::kCRC)
            {
                if (!this->m_SubCommands.empty())
                {
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>());
                }

                // **************** CuinZip P1-4 Modification Start ****************
                // Hash 项标题接入本地化资源,不再散落硬编码。
                struct HashItem
                {
                    UINT32 LangID;
                    DWORD CommandID;
                };
                static HashItem const kHashItems[] =
                {
                    { IDS_CONTEXT_HASH_CRC32,  CommandID::HashCRC32 },
                    { IDS_CONTEXT_HASH_CRC64,  CommandID::HashCRC64 },
                    { IDS_CONTEXT_HASH_SHA1,   CommandID::HashSHA1 },
                    { IDS_CONTEXT_HASH_SHA256, CommandID::HashSHA256 },
                    { IDS_CONTEXT_HASH_ALL,    CommandID::HashAll },
                };

                for (HashItem const& Item : kHashItems)
                {
                    UString TranslatedString;
                    LangString(Item.LangID, TranslatedString);
                    this->m_SubCommands.push_back(
                        winrt::make<ExplorerCommandBase>(
                            std::wstring(
                                TranslatedString.Ptr(),
                                TranslatedString.Len()),
                            Item.CommandID));
                }
                // **************** CuinZip P1-4 Modification End ****************
            }
        }

    public:

        ExplorerCommandRoot()
        {
            CContextMenuInfo ContextMenuInfo;
            ContextMenuInfo.Load();
            this->m_ContextMenuFlags = ContextMenuInfo.Flags;
            this->m_ContextMenuElimDup = ContextMenuInfo.ElimDup;
        }

#pragma region IExplorerCommand

        HRESULT STDMETHODCALLTYPE GetTitle(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszName)
        {
            this->Initialize(psiItemArray);

            if (this->m_SubCommands.empty())
            {
                *ppszName = nullptr;
                return E_NOTIMPL;
            }

            return ::SHStrDupW(L"CuinZip Preview", ppszName);
        }

        HRESULT STDMETHODCALLTYPE GetIcon(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszIcon)
        {
            UNREFERENCED_PARAMETER(psiItemArray);
            UString Path = ::GetNanaZipPath();
            std::wstring Icon = std::wstring(Path.Ptr(), Path.Len());
            Icon += L",-1";
            return ::SHStrDupW(Icon.c_str(), ppszIcon);
        }

        HRESULT STDMETHODCALLTYPE GetToolTip(
            _In_opt_ IShellItemArray* psiItemArray,
            _Outptr_ LPWSTR* ppszInfotip)
        {
            UNREFERENCED_PARAMETER(psiItemArray);
            *ppszInfotip = nullptr;
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetCanonicalName(
            _Out_ GUID* pguidCommandName)
        {
            *pguidCommandName = GUID_NULL;
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetState(
            _In_opt_ IShellItemArray* psiItemArray,
            _In_ BOOL fOkToBeSlow,
            _Out_ EXPCMDSTATE* pCmdState)
        {
            UNREFERENCED_PARAMETER(fOkToBeSlow);

            // **************** CuinZip P1-4 Modification Start ****************
            // 平铺模式(二级菜单模式关闭)时隐藏二级菜单根项,避免空壳菜单;
            // 二级菜单模式下若无任何可用子命令也同样隐藏。
            CContextMenuInfo ContextMenuInfo;
            ContextMenuInfo.Load();
            if (ContextMenuInfo.Cascaded.Val == false)
            {
                *pCmdState = ECS_HIDDEN;
                return S_OK;
            }

            this->Initialize(psiItemArray);
            *pCmdState = this->m_SubCommands.empty()
                ? ECS_HIDDEN
                : ECS_ENABLED;
            return S_OK;
            // **************** CuinZip P1-4 Modification End ****************
        }

        HRESULT STDMETHODCALLTYPE Invoke(
            _In_opt_ IShellItemArray* psiItemArray,
            _In_opt_ IBindCtx* pbc)
        {
            UNREFERENCED_PARAMETER(psiItemArray);
            UNREFERENCED_PARAMETER(pbc);
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE GetFlags(
            _Out_ EXPCMDFLAGS* pFlags)
        {
            *pFlags = ECF_HASSUBCOMMANDS;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE EnumSubCommands(
            _Outptr_ IEnumExplorerCommand** ppEnum)
        {
            if (this->m_SubCommands.empty())
            {
                *ppEnum = nullptr;
                return E_NOTIMPL;
            }
            else
            {
                this->m_CurrentSubCommand = this->m_SubCommands.cbegin();
                return this->QueryInterface(IID_PPV_ARGS(ppEnum));
            }
        }

#pragma endregion

#pragma region IEnumExplorerCommand

        HRESULT STDMETHODCALLTYPE Next(
            _In_ ULONG celt,
            _Out_ IExplorerCommand** pUICommand,
            _Out_opt_ ULONG* pceltFetched)
        {
            ULONG Fetched = 0;

            for (
                ULONG i = 0;
                (i < celt) &&
                (this->m_CurrentSubCommand != this->m_SubCommands.cend());
                ++i)
            {
                this->m_CurrentSubCommand->copy_to(&pUICommand[i]);
                ++Fetched;
                ++this->m_CurrentSubCommand;
            }

            if (pceltFetched)
            {
                *pceltFetched = Fetched;
            }

            return (Fetched == celt) ? S_OK : S_FALSE;
        }

        HRESULT STDMETHODCALLTYPE Skip(
            _In_ ULONG celt)
        {
            UNREFERENCED_PARAMETER(celt);
            return E_NOTIMPL;
        }

        HRESULT STDMETHODCALLTYPE Reset()
        {
            this->m_CurrentSubCommand = this->m_SubCommands.cbegin();
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE Clone(
            _Outptr_ IEnumExplorerCommand** ppenum)
        {
            *ppenum = nullptr;
            return E_NOTIMPL;
        }

#pragma endregion
    };

    struct DECLSPEC_UUID("788F8FA7-178F-40BE-BAF5-5D5D1335C0F9")
        ClassFactory : public winrt::implements<
        ClassFactory, IClassFactory>
    {
    public:

        HRESULT STDMETHODCALLTYPE CreateInstance(
            _In_opt_ IUnknown* pUnkOuter,
            _In_ REFIID riid,
            _Outptr_ void** ppvObject) noexcept override
        {
            UNREFERENCED_PARAMETER(pUnkOuter);

            try
            {
                return winrt::make<ExplorerCommandRoot>()->QueryInterface(
                    riid, ppvObject);
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
        }

        HRESULT STDMETHODCALLTYPE LockServer(
            _In_ BOOL fLock) noexcept override
        {
            if (fLock)
            {
                ++winrt::get_module_lock();
            }
            else
            {
                --winrt::get_module_lock();
            }

            return S_OK;
        }
    };

    // **************** CuinZip P1-4 Modification Start ****************
    // 平铺动词的类厂:每个 CLSID 对应一个固定命令,标题/显隐在选择变化时
    // 由 ExplorerCommandBase 实时计算。
    struct FlatVerbClassFactory : public winrt::implements<
        FlatVerbClassFactory, IClassFactory>
    {
    private:

        DWORD m_CommandID;

    public:

        FlatVerbClassFactory(DWORD CommandID) : m_CommandID(CommandID)
        {
        }

        HRESULT STDMETHODCALLTYPE CreateInstance(
            _In_opt_ IUnknown* pUnkOuter,
            _In_ REFIID riid,
            _Outptr_ void** ppvObject) noexcept override
        {
            UNREFERENCED_PARAMETER(pUnkOuter);

            try
            {
                return winrt::make<ExplorerCommandBase>(
                    std::wstring(),
                    this->m_CommandID,
                    CBoolPair(),
                    static_cast<UInt32>(-1),
                    true)->QueryInterface(riid, ppvObject);
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
        }

        HRESULT STDMETHODCALLTYPE LockServer(
            _In_ BOOL fLock) noexcept override
        {
            if (fLock)
            {
                ++winrt::get_module_lock();
            }
            else
            {
                --winrt::get_module_lock();
            }

            return S_OK;
        }
    };
    // **************** CuinZip P1-4 Modification End ****************
}

EXTERN_C HRESULT STDAPICALLTYPE DllCanUnloadNow()
{
    if (winrt::get_module_lock())
    {
        return S_FALSE;
    }

    winrt::clear_factory_cache();
    return S_OK;
}

EXTERN_C HRESULT STDAPICALLTYPE DllGetClassObject(
    _In_ REFCLSID rclsid,
    _In_ REFIID riid,
    _Outptr_ LPVOID* ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (riid != IID_IClassFactory && riid != IID_IUnknown)
    {
        return E_NOINTERFACE;
    }

    // **************** CuinZip P1-4 Modification Start ****************
    // 平铺动词 CLSID(与二级菜单根项 CLSID 一样均为 CuinZip 自有 GUID)。
    for (FlatVerbEntry const& Entry : kFlatVerbs)
    {
        if (rclsid == Entry.Clsid)
        {
            try
            {
                return winrt::make<NanaZip::ShellExtension::FlatVerbClassFactory>(
                    Entry.CommandID)->QueryInterface(riid, ppv);
            }
            catch (...)
            {
                return winrt::to_hresult();
            }
        }
    }
    // **************** CuinZip P1-4 Modification End ****************

    if (rclsid != __uuidof(NanaZip::ShellExtension::ClassFactory))
    {
        return E_INVALIDARG;
    }

    try
    {
        return winrt::make<NanaZip::ShellExtension::ClassFactory>(
            )->QueryInterface(riid, ppv);
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
}

long g_DllRefCount = 0;
HWND g_HWND = nullptr;
HINSTANCE g_hInstance = nullptr;

// Only used in Shell Extension to check whether NanaZip Modern is loaded.
EXTERN_C HMODULE K7ModernCurrentModule;

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD fdwReason,
    _In_ LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_hInstance = hinstDLL;
        ::K7ModernCurrentModule = hinstDLL;
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "UiStrings.h"

#include "NanaZip.Modern.h"

#include <shlobj.h>

#include <cstring>
#include <string>

#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Media.h>

// CuinZip P1-4: 经典设置页入口命令 ID。
// 与 FileManager(App.h)侧的 kMenuCmdID_Toolbar_Legacy_* 常量保持一致,
// 通过 WM_COMMAND 投递到主窗口,由 ExecuteCommand 打开对应属性表页。
namespace
{
    enum LegacySettingsCommandID
    {
        LegacySettings = 1075,       // Settings 页(常规/高级项)
        LegacyIntegration = 1076,    // Integration 页(关联/右键菜单/提取集成)
        LegacyFolders = 1077,        // Folders 页(提取工作文件夹)
        LegacyEditor = 1078          // Editor 页(外部编辑器)
    };

    // 分类索引(与 m_Panels 数组一致)。
    enum CategoryIndex
    {
        CategoryGeneral = 0,
        CategoryCompression,
        CategoryExtraction,
        CategoryFileAssociations,
        CategoryContextMenu,
        CategoryAppearance,
        CategoryAdvanced,
        CategoryAbout
    };

    // 压缩默认格式(值与压缩对话框的 ArcType 一致)。
    struct CompressionFormatItem
    {
        std::wstring_view Key;
        std::wstring_view Fallback;
        std::wstring_view ArcType;
    };

    CompressionFormatItem const kCompressionFormats[] =
    {
        { L"SettingsPage/Format7z.Text",    L"7z",   L"7z"   },
        { L"SettingsPage/FormatZip.Text",   L"zip",  L"Zip"  },
        { L"SettingsPage/FormatTar.Text",   L"tar",  L"Tar"  },
        { L"SettingsPage/FormatGzip.Text",  L"gzip", L"GZip" },
        { L"SettingsPage/FormatBzip2.Text", L"bzip2",L"BZip2"},
        { L"SettingsPage/FormatXz.Text",    L"xz",   L"xz"   },
    };

    // 压缩级别(值与压缩对话框一致:0/1/3/5/7/9)。
    struct CompressionLevelItem
    {
        std::wstring_view Key;
        std::wstring_view Fallback;
        int Level;
    };

    CompressionLevelItem const kCompressionLevels[] =
    {
        { L"SettingsPage/LevelStore.Text",   L"Store",   0 },
        { L"SettingsPage/LevelFastest.Text", L"Fastest", 1 },
        { L"SettingsPage/LevelFast.Text",    L"Fast",    3 },
        { L"SettingsPage/LevelNormal.Text",  L"Normal",  5 },
        { L"SettingsPage/LevelMaximum.Text", L"Maximum", 7 },
        { L"SettingsPage/LevelUltra.Text",   L"Ultra",   9 },
    };

    // 解压路径模式(NExtract::NPathMode::EEnum,与解压对话框一致)。
    struct ExtractionPathModeItem
    {
        std::wstring_view Key;
        std::wstring_view Fallback;
        int Value;
    };

    ExtractionPathModeItem const kExtractionPathModes[] =
    {
        { L"SettingsPage/PathModeFull.Text", L"Full pathnames",       0 },
        { L"SettingsPage/PathModeNo.Text",   L"No pathnames",         2 },
        { L"SettingsPage/PathModeAbs.Text",  L"Absolute pathnames",   3 },
    };

    // 覆盖模式(NExtract::NOverwriteMode::EEnum,与解压对话框一致)。
    struct ExtractionOverwriteItem
    {
        std::wstring_view Key;
        std::wstring_view Fallback;
        int Value;
    };

    ExtractionOverwriteItem const kExtractionOverwriteModes[] =
    {
        { L"SettingsPage/OverwriteAsk.Text",        L"Ask before overwrite",           0 },
        { L"SettingsPage/OverwriteAlways.Text",     L"Overwrite without prompt",       1 },
        { L"SettingsPage/OverwriteSkip.Text",       L"Skip existing files",            2 },
        { L"SettingsPage/OverwriteRename.Text",     L"Auto rename",                    3 },
        { L"SettingsPage/OverwriteRenameOld.Text",  L"Auto rename existing files",     4 },
    };

    // 文件关联分类展示的常见格式(MSIX 声明的完整集合以包清单为准)。
    wchar_t const* const kCommonAssociationExtensions[] =
    {
        L".7z",  L".zip",  L".rar",  L".tar",  L".gz",   L".tgz",
        L".bz2", L".tbz2", L".xz",   L".txz",  L".zst",  L".lz4",
        L".iso", L".cab",  L".wim",  L".arj",  L".lzh",  L".001",
    };

    template <typename T, std::size_t N>
    constexpr std::size_t ArraySize(T const (&)[N]) noexcept
    {
        return N;
    }

    std::wstring ReplaceAll(
        std::wstring Text,
        std::wstring const& From,
        std::wstring const& To)
    {
        if (From.empty())
        {
            return Text;
        }
        std::size_t Position = 0;
        while ((Position = Text.find(From, Position)) != std::wstring::npos)
        {
            Text.replace(Position, From.length(), To);
            Position += To.length();
        }
        return Text;
    }
}

namespace winrt
{
    using Windows::UI::Xaml::Controls::Button;
    using Windows::UI::Xaml::Controls::ColumnDefinition;
    using Windows::UI::Xaml::Controls::ComboBox;
    using Windows::UI::Xaml::Controls::ComboBoxItem;
    using Windows::UI::Xaml::Controls::FontIcon;
    using Windows::UI::Xaml::Controls::Grid;
    using Windows::UI::Xaml::Controls::ListView;
    using Windows::UI::Xaml::Controls::ListViewItem;
    using Windows::UI::Xaml::Controls::ScrollViewer;
    using Windows::UI::Xaml::Controls::ListViewSelectionMode;
    using Windows::UI::Xaml::Controls::StackPanel;
    using Windows::UI::Xaml::Controls::TextBlock;
    using Windows::UI::Xaml::Controls::ToggleSwitch;
    using Windows::UI::Xaml::Controls::Orientation;
    using Windows::UI::Xaml::GridLength;
    using Windows::UI::Xaml::GridLengthHelper;
    using Windows::UI::Xaml::GridUnitType;
    using Windows::UI::Xaml::ThicknessHelper;
    using Windows::UI::Xaml::Media::FontFamily;
    using Windows::UI::Xaml::VerticalAlignment;
    using Windows::UI::Xaml::Visibility;
    using Windows::UI::Xaml::Thickness;
}

namespace
{
    // 构建一个 TextBlock;secondary 为真时用次级文本色(Light/Dark 均随主题)。
    winrt::Windows::UI::Xaml::Controls::TextBlock MakeText(
        std::wstring_view const& key,
        std::wstring_view const& fallback,
        double fontSize,
        bool semiBold,
        bool secondary,
        winrt::Windows::UI::Xaml::Thickness const& margin)
    {
        winrt::Windows::UI::Xaml::Controls::TextBlock text;
        text.Text(winrt::NanaZip::Modern::GetUiString(key, fallback));
        if (fontSize > 0)
        {
            text.FontSize(fontSize);
        }
        if (semiBold)
        {
            text.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        }
        text.Margin(margin);
        text.TextWrapping(winrt::Windows::UI::Xaml::TextWrapping::Wrap);

        if (secondary)
        {
            // 与 StatusBarTemplate 一致:次级文本走主题资源。
            auto brush = winrt::Windows::UI::Xaml::Application::Current()
                .Resources().TryLookup(
                    winrt::box_value(winrt::hstring(L"TextFillColorSecondaryBrush")))
                .try_as<winrt::Windows::UI::Xaml::Media::Brush>();
            if (brush)
            {
                text.Foreground(brush);
            }
        }
        return text;
    }

    winrt::Windows::UI::Xaml::Thickness UniformThickness(double value)
    {
        return winrt::Windows::UI::Xaml::ThicknessHelper::FromUniformLength(value);
    }

    winrt::Windows::UI::Xaml::Thickness LengthsThickness(double left, double top, double right, double bottom)
    {
        return winrt::Windows::UI::Xaml::ThicknessHelper::FromLengths(left, top, right, bottom);
    }
}

namespace winrt::NanaZip::Modern::implementation
{
    SettingsPage::SettingsPage(
        _In_opt_ HWND WindowHandle,
        _In_opt_ const K7_MODERN_SETTINGS_CALLBACKS* Callbacks) :
        m_WindowHandle(WindowHandle)
    {
        if (Callbacks)
        {
            m_Callbacks = *Callbacks;
        }
        this->InitializeComponent();
    }

    void SettingsPage::LoadHostSettings()
    {
        if (m_Callbacks.Load)
        {
            m_Callbacks.Load(&m_Appearance);
        }
        if (m_Callbacks.ContextMenuLoad)
        {
            m_Callbacks.ContextMenuLoad(&m_ContextMenu);
        }
        if (m_Callbacks.CompressionLoad)
        {
            m_Callbacks.CompressionLoad(&m_Compression);
        }
        if (m_Callbacks.ExtractionLoad)
        {
            m_Callbacks.ExtractionLoad(&m_Extraction);
        }
    }

    void SettingsPage::InitializeComponent()
    {
        using namespace winrt;

        this->LoadHostSettings();

        // ================== 根布局:左右两栏 ==================
        Grid root;
        ColumnDefinition leftColumn;
        leftColumn.Width(GridLengthHelper::FromPixels(220));
        ColumnDefinition rightColumn;
        rightColumn.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
        root.ColumnDefinitions().Append(leftColumn);
        root.ColumnDefinitions().Append(rightColumn);

        // ================== 左栏:标题 + 分类导航 ==================
        StackPanel navigation;
        navigation.Margin(UniformThickness(0));

        navigation.Children().Append(MakeText(
            L"SettingsPage/SettingsTitle.Text",
            L"Settings",
            20, true, false, LengthsThickness(16, 16, 8, 8)));

        m_NavList = ListView();
        m_NavList.SelectionMode(ListViewSelectionMode::Single);
        m_NavList.Margin(ThicknessHelper::FromLengths(0, 0, 0, 8));

        struct NavItem
        {
            std::wstring_view Key;
            std::wstring_view Fallback;
            wchar_t const* Glyph;
        };

        static NavItem const navItems[8] =
        {
            { L"SettingsPage/NavGeneralText.Text",          L"General",           L"\xE713" },
            { L"SettingsPage/NavCompressionText.Text",      L"Compression",       L"\xE8C8" },
            { L"SettingsPage/NavExtractionText.Text",       L"Extraction",        L"\xE8E5" },
            { L"SettingsPage/NavFileAssociationsText.Text", L"File Associations", L"\xE8A7" },
            { L"SettingsPage/NavContextMenuText.Text",      L"Context Menu",      L"\xE700" },
            { L"SettingsPage/NavAppearanceText.Text",       L"Appearance",        L"\xE790" },
            { L"SettingsPage/NavAdvancedText.Text",         L"Advanced",          L"\xE9D9" },
            { L"SettingsPage/NavAboutText.Text",            L"About",             L"\xE946" },
        };

        winrt::Windows::UI::Xaml::Media::FontFamily symbolFont(
            L"Segoe Fluent Icons,Segoe MDL2 Assets");

        for (auto const& item : navItems)
        {
            ListViewItem container;

            StackPanel host;
            host.Orientation(Orientation::Horizontal);
            host.Spacing(12);

            FontIcon icon;
            icon.FontFamily(symbolFont);
            icon.FontSize(16);
            icon.Glyph(item.Glyph);
            host.Children().Append(icon);

            TextBlock label;
            label.Text(winrt::NanaZip::Modern::GetUiString(item.Key, item.Fallback));
            label.VerticalAlignment(VerticalAlignment::Center);
            host.Children().Append(label);

            container.Content(host);
            m_NavList.Items().Append(container);
        }

        m_NavList.SelectionChanged({ this, &SettingsPage::NavListSelectionChanged });
        navigation.Children().Append(m_NavList);

        Grid::SetColumn(navigation, 0);
        root.Children().Append(navigation);

        // ================== 右栏:分类内容(滚动) ==================
        ScrollViewer viewer;
        auto content = StackPanel();
        content.Margin(LengthsThickness(20, 16, 20, 20));
        content.Spacing(8);

        // ---------- General(CFmSettings 真实配置源) ----------
        m_Panels[CategoryGeneral] = StackPanel();
        {
            auto& panel = m_Panels[CategoryGeneral];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/GeneralTitle.Text", L"General",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/GeneralDescription.Text",
                L"Core file manager behavior options. Changes are saved immediately.",
                0, false, true, UniformThickness(0)));

            m_ShowSystemMenuToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowSystemMenuToggle.Header",
                L"Show system menu",
                m_Appearance.ShowSystemMenu != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_ArcHistoryToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ArcHistoryToggle.Header",
                L"Keep archive history",
                m_Appearance.ArcHistory != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_PathHistoryToggle = this->BuildToggle(
                panel,
                L"SettingsPage/PathHistoryToggle.Header",
                L"Keep path history",
                m_Appearance.PathHistory != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_CopyHistoryToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CopyHistoryToggle.Header",
                L"Keep copy history",
                m_Appearance.CopyHistory != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_FolderHistoryToggle = this->BuildToggle(
                panel,
                L"SettingsPage/FolderHistoryToggle.Header",
                L"Keep folder history",
                m_Appearance.FolderHistory != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });

            this->BuildButton(
                panel,
                L"SettingsPage/GeneralLegacyButton.Content",
                L"Open classic settings (General)",
                { this, &SettingsPage::GeneralLegacyButtonClick });
        }
        content.Children().Append(m_Panels[CategoryGeneral]);

        // ---------- Compression(NCompression::CInfo 真实配置源) ----------
        m_Panels[CategoryCompression] = StackPanel();
        {
            auto& panel = m_Panels[CategoryCompression];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/CompressionTitle.Text", L"Compression",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/CompressionDescription.Text",
                L"Default format and compression level for the Add to Archive dialog. Detailed options are available in the archive creation dialog.",
                0, false, true, UniformThickness(0)));

            std::vector<std::pair<std::wstring, int>> formatItems;
            int selectedFormat = 0;
            for (std::size_t i = 0; i < ArraySize(kCompressionFormats); ++i)
            {
                auto const& Item = kCompressionFormats[i];
                formatItems.push_back(std::make_pair(
                    std::wstring(
                        winrt::NanaZip::Modern::GetUiString(Item.Key, Item.Fallback)),
                    static_cast<int>(i)));
                if (m_Compression.ArchiveType == Item.ArcType)
                {
                    selectedFormat = static_cast<int>(i);
                }
            }
            m_FormatCombo = this->BuildCombo(
                panel,
                L"SettingsPage/FormatComboHeader.Text",
                L"Default archive format",
                formatItems,
                selectedFormat,
                [this]() { this->ApplyCompressionSettings(); });

            std::vector<std::pair<std::wstring, int>> levelItems;
            int selectedLevel = 3;
            for (std::size_t i = 0; i < ArraySize(kCompressionLevels); ++i)
            {
                auto const& Item = kCompressionLevels[i];
                levelItems.push_back(std::make_pair(
                    std::wstring(
                        winrt::NanaZip::Modern::GetUiString(Item.Key, Item.Fallback)),
                    Item.Level));
                if (static_cast<int>(m_Compression.Level) == Item.Level)
                {
                    selectedLevel = static_cast<int>(i);
                }
            }
            m_LevelCombo = this->BuildCombo(
                panel,
                L"SettingsPage/LevelComboHeader.Text",
                L"Default compression level",
                levelItems,
                selectedLevel,
                [this]() { this->ApplyCompressionSettings(); });

            panel.Children().Append(MakeText(
                L"SettingsPage/CompressionNote.Text",
                L"New defaults apply to the next Add to Archive dialog.",
                0, false, true, UniformThickness(0)));
        }
        content.Children().Append(m_Panels[CategoryCompression]);

        // ---------- Extraction(NExtract::CInfo 真实配置源) ----------
        m_Panels[CategoryExtraction] = StackPanel();
        {
            auto& panel = m_Panels[CategoryExtraction];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/ExtractionTitle.Text", L"Extraction",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/ExtractionDescription.Text",
                L"Default destination and overwrite behavior for the Extract dialog.",
                0, false, true, UniformThickness(0)));

            std::vector<std::pair<std::wstring, int>> pathModeItems;
            int selectedPathMode = 0;
            for (std::size_t i = 0; i < ArraySize(kExtractionPathModes); ++i)
            {
                auto const& Item = kExtractionPathModes[i];
                pathModeItems.push_back(std::make_pair(
                    std::wstring(
                        winrt::NanaZip::Modern::GetUiString(Item.Key, Item.Fallback)),
                    Item.Value));
                if (static_cast<int>(m_Extraction.PathMode) == Item.Value)
                {
                    selectedPathMode = static_cast<int>(i);
                }
            }
            m_PathModeCombo = this->BuildCombo(
                panel,
                L"SettingsPage/PathModeComboHeader.Text",
                L"Default path mode",
                pathModeItems,
                selectedPathMode,
                [this]() { this->ApplyExtractionSettings(); });

            std::vector<std::pair<std::wstring, int>> overwriteItems;
            int selectedOverwrite = 0;
            for (std::size_t i = 0; i < ArraySize(kExtractionOverwriteModes); ++i)
            {
                auto const& Item = kExtractionOverwriteModes[i];
                overwriteItems.push_back(std::make_pair(
                    std::wstring(
                        winrt::NanaZip::Modern::GetUiString(Item.Key, Item.Fallback)),
                    Item.Value));
                if (static_cast<int>(m_Extraction.OverwriteMode) == Item.Value)
                {
                    selectedOverwrite = static_cast<int>(i);
                }
            }
            m_OverwriteModeCombo = this->BuildCombo(
                panel,
                L"SettingsPage/OverwriteComboHeader.Text",
                L"Default overwrite mode",
                overwriteItems,
                selectedOverwrite,
                [this]() { this->ApplyExtractionSettings(); });

            m_OpenFolderToggle = this->BuildToggle(
                panel,
                L"SettingsPage/OpenFolderToggle.Header",
                L"Open folder after extraction",
                m_Extraction.OpenFolderAfterExtraction != FALSE,
                [this]() { this->ApplyExtractionSettings(); });

            panel.Children().Append(MakeText(
                L"SettingsPage/ExtractionNote.Text",
                L"New defaults apply to the next Extract dialog.",
                0, false, true, UniformThickness(0)));

            this->BuildButton(
                panel,
                L"SettingsPage/ExtractionFoldersButton.Content",
                L"Working folder settings",
                { this, &SettingsPage::ExtractionFoldersButtonClick });
            this->BuildButton(
                panel,
                L"SettingsPage/ExtractionIntegrationButton.Content",
                L"Extraction integration options",
                { this, &SettingsPage::ExtractionIntegrationButtonClick });
        }
        content.Children().Append(m_Panels[CategoryExtraction]);

        // ---------- File Associations(只读状态 + 官方设置入口) ----------
        m_Panels[CategoryFileAssociations] = StackPanel();
        {
            auto& panel = m_Panels[CategoryFileAssociations];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/FileAssociationsTitle.Text", L"File Associations",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/FileAssociationsDescription.Text",
                L"Choose which file types open with CuinZip. Associations are managed by Windows.",
                0, false, true, UniformThickness(0)));

            m_ExtractOnOpenToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ExtractOnOpenToggle.Header",
                L"Extract archives to a temporary folder when opening them",
                m_ContextMenu.ExtractOnOpen != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });

            panel.Children().Append(MakeText(
                L"SettingsPage/AssocListHeader.Text",
                L"Common archive formats",
                14, true, false, LengthsThickness(0, 12, 0, 0)));

            m_AssociationList = ListView();
            m_AssociationList.SelectionMode(ListViewSelectionMode::None);
            m_AssociationList.MaxHeight(280);
            panel.Children().Append(m_AssociationList);

            this->BuildButton(
                panel,
                L"SettingsPage/AssocRefreshButton.Content",
                L"Refresh status",
                { this, &SettingsPage::FileAssociationsRefreshButtonClick });
            this->BuildButton(
                panel,
                L"SettingsPage/FileAssociationsSystemButton.Content",
                L"Open Windows Settings (Default apps)",
                { this, &SettingsPage::FileAssociationsSystemButtonClick });
            this->BuildButton(
                panel,
                L"SettingsPage/FileAssociationsLegacyButton.Content",
                L"Open classic settings (Integration)",
                { this, &SettingsPage::FileAssociationsLegacyButtonClick });
        }
        content.Children().Append(m_Panels[CategoryFileAssociations]);

        // ---------- Context Menu(CContextMenuInfo 真实配置源) ----------
        m_Panels[CategoryContextMenu] = StackPanel();
        {
            auto& panel = m_Panels[CategoryContextMenu];
            panel.Spacing(4);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuTitle.Text", L"Context Menu",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuDescription.Text",
                L"Configure the entries shown in the Windows Explorer context menu. Changes apply the next time you right-click.",
                0, false, true, UniformThickness(0)));

            m_CtxOpenToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxOpenToggle.Header",
                L"Open archive",
                m_ContextMenu.ShowOpen != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxExtractHereToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxExtractHereToggle.Header",
                L"Extract Here",
                m_ContextMenu.ShowExtractHere != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxExtractToToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxExtractToToggle.Header",
                L"Extract to <folder>",
                m_ContextMenu.ShowExtractTo != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressToggle.Header",
                L"Add to archive...",
                m_ContextMenu.ShowCompress != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressTo7zToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressTo7zToggle.Header",
                L"Add to <archive>.7z",
                m_ContextMenu.ShowCompressTo7z != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressToZipToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressToZipToggle.Header",
                L"Add to <archive>.zip",
                m_ContextMenu.ShowCompressToZip != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });

            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuMoreHeader.Text",
                L"More items",
                14, true, false, LengthsThickness(0, 12, 0, 0)));

            m_CtxTestToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxTestToggle.Header",
                L"Test archive",
                m_ContextMenu.ShowTest != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxExtractToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxExtractToggle.Header",
                L"Extract files...",
                m_ContextMenu.ShowExtract != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxExtractHereSmartToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxExtractHereSmartToggle.Header",
                L"Extract Here (Smart)",
                m_ContextMenu.ShowExtractHereSmart != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressEmailToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressEmailToggle.Header",
                L"Compress and email...",
                m_ContextMenu.ShowCompressEmail != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressTo7zEmailToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressTo7zEmailToggle.Header",
                L"Compress to <archive>.7z and email",
                m_ContextMenu.ShowCompressTo7zEmail != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxCompressToZipEmailToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCompressToZipEmailToggle.Header",
                L"Compress to <archive>.zip and email",
                m_ContextMenu.ShowCompressToZipEmail != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxHashToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxHashToggle.Header",
                L"Hash (CRC / SHA)",
                m_ContextMenu.ShowHash != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            m_CtxElimDupToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxElimDupToggle.Header",
                L"Eliminate duplicate folders",
                m_ContextMenu.EliminateDuplicateFiles != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });

            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuLayoutHeader.Text",
                L"Menu layout",
                14, true, false, LengthsThickness(0, 12, 0, 0)));

            m_CtxCascadedToggle = this->BuildToggle(
                panel,
                L"SettingsPage/CtxCascadedToggle.Header",
                L"Show all items under one CuinZip submenu",
                m_ContextMenu.CascadedMenu != FALSE,
                [this]() { this->ApplyContextMenuSettings(); });
            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuCascadedNote.Text",
                L"When off, the most common items appear directly in the context menu.",
                0, false, true, UniformThickness(0)));
        }
        content.Children().Append(m_Panels[CategoryContextMenu]);

        // ---------- Appearance(即时生效的外观开关) ----------
        m_Panels[CategoryAppearance] = StackPanel();
        {
            auto& panel = m_Panels[CategoryAppearance];
            panel.Spacing(4);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/AppearanceTitle.Text", L"Appearance",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/AppearanceDescription.Text",
                L"Changes apply immediately to the file lists.",
                0, false, true, UniformThickness(0)));

            m_ShowDotsToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowDotsToggle.Header",
                L"Show \"..\" item",
                m_Appearance.ShowDots != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_ShowRealFileIconsToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowRealFileIconsToggle.Header",
                L"Show real file icons",
                m_Appearance.ShowRealFileIcons != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_FullRowToggle = this->BuildToggle(
                panel,
                L"SettingsPage/FullRowToggle.Header",
                L"Full row select",
                m_Appearance.FullRow != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_ShowGridToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowGridToggle.Header",
                L"Show grid lines",
                m_Appearance.ShowGrid != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_SingleClickToggle = this->BuildToggle(
                panel,
                L"SettingsPage/SingleClickToggle.Header",
                L"Single-click to open an item",
                m_Appearance.SingleClick != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });
            m_AlternativeSelectionToggle = this->BuildToggle(
                panel,
                L"SettingsPage/AlternativeSelectionToggle.Header",
                L"Alternative selection mode",
                m_Appearance.AlternativeSelection != FALSE,
                [this]() { this->ApplyAppearanceSettings(); });

            this->BuildButton(
                panel,
                L"SettingsPage/AppearanceLegacyButton.Content",
                L"More appearance options (classic settings)",
                { this, &SettingsPage::AppearanceLegacyButtonClick });
        }
        content.Children().Append(m_Panels[CategoryAppearance]);

        // ---------- Advanced ----------
        m_Panels[CategoryAdvanced] = StackPanel();
        {
            auto& panel = m_Panels[CategoryAdvanced];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/AdvancedTitle.Text", L"Advanced",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/AdvancedDescription.Text",
                L"System menu, memory pages, history and other advanced options.",
                0, false, true, UniformThickness(0)));
            this->BuildButton(
                panel,
                L"SettingsPage/AdvancedLegacyButton.Content",
                L"Open classic settings (Advanced)",
                { this, &SettingsPage::AdvancedLegacyButtonClick });
            this->BuildButton(
                panel,
                L"SettingsPage/AdvancedEditorButton.Content",
                L"Editor settings",
                { this, &SettingsPage::AdvancedEditorButtonClick });
        }
        content.Children().Append(m_Panels[CategoryAdvanced]);

        // ---------- About ----------
        m_Panels[CategoryAbout] = StackPanel();
        {
            auto& panel = m_Panels[CategoryAbout];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/AboutTitle.Text", L"About",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/AboutDescription.Text",
                L"CuinZip is an open source file archiver based on NanaZip and 7-Zip.",
                0, false, true, UniformThickness(0)));
            this->BuildButton(
                panel,
                L"SettingsPage/AboutDialogButton.Content",
                L"Open the About dialog",
                { this, &SettingsPage::AboutDialogButtonClick });
        }
        content.Children().Append(m_Panels[CategoryAbout]);

        viewer.Content(content);
        Grid::SetColumn(viewer, 1);
        root.Children().Append(viewer);

        this->Content(root);

        // 默认选中第一个分类(General)。
        this->m_Initializing = true;
        m_NavList.SelectedIndex(0);
        this->m_Initializing = false;
    }

    void SettingsPage::NavListSelectionChanged(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        int index = m_NavList.SelectedIndex();
        if (index < 0)
        {
            return;
        }
        this->ShowCategory(index);

        // 文件关联状态可能在设置窗口外被 Windows 更改,每次进入该分类
        // 都重新查询,保证展示的是实时状态。
        if (index == CategoryFileAssociations)
        {
            this->RefreshFileAssociationList();
        }
    }

    void SettingsPage::ShowCategory(int index)
    {
        const std::size_t panelCount =
            sizeof(m_Panels) / sizeof(*m_Panels);
        for (std::size_t i = 0; i < panelCount; ++i)
        {
            if (m_Panels[i])
            {
                m_Panels[i].Visibility(
                    (static_cast<int>(i) == index)
                    ? winrt::Windows::UI::Xaml::Visibility::Visible
                    : winrt::Windows::UI::Xaml::Visibility::Collapsed);
            }
        }
    }

    winrt::Windows::UI::Xaml::Controls::ToggleSwitch SettingsPage::BuildToggle(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        std::wstring_view const& headerKey,
        std::wstring_view const& headerFallback,
        bool isOn,
        std::function<void()> const& applyHandler)
    {
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch toggle;
        toggle.Header(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(headerKey, headerFallback)));
        toggle.Margin(winrt::Windows::UI::Xaml::ThicknessHelper::FromLengths(0, 4, 0, 0));
        toggle.IsOn(isOn);
        toggle.Toggled([this, applyHandler](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::RoutedEventArgs const&)
        {
            if (this->m_Initializing)
            {
                return;
            }
            applyHandler();
        });
        parent.Children().Append(toggle);
        return toggle;
    }

    winrt::Windows::UI::Xaml::Controls::ComboBox SettingsPage::BuildCombo(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        std::wstring_view const& headerKey,
        std::wstring_view const& headerFallback,
        std::vector<std::pair<std::wstring, int>> const& items,
        int selectedValue,
        std::function<void()> const& applyHandler)
    {
        parent.Children().Append(MakeText(
            headerKey,
            headerFallback,
            14, true, false, LengthsThickness(0, 12, 0, 0)));

        winrt::Windows::UI::Xaml::Controls::ComboBox combo;
        combo.Margin(winrt::Windows::UI::Xaml::ThicknessHelper::FromLengths(0, 4, 0, 0));
        combo.MinWidth(220);
        combo.HorizontalAlignment(
            winrt::Windows::UI::Xaml::HorizontalAlignment::Left);

        int selectedIndex = 0;
        for (auto const& Item : items)
        {
            winrt::Windows::UI::Xaml::Controls::ComboBoxItem element;
            element.Content(winrt::box_value(winrt::hstring(Item.first)));
            element.Tag(winrt::box_value(Item.second));
            combo.Items().Append(element);
        }
        for (std::size_t i = 0; i < items.size(); ++i)
        {
            if (items[i].second == selectedValue)
            {
                selectedIndex = static_cast<int>(i);
                break;
            }
        }
        if (!items.empty())
        {
            combo.SelectedIndex(selectedIndex);
        }
        combo.SelectionChanged([this, applyHandler](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::RoutedEventArgs const&)
        {
            if (this->m_Initializing)
            {
                return;
            }
            applyHandler();
        });
        parent.Children().Append(combo);
        return combo;
    }

    winrt::Windows::UI::Xaml::Controls::Button SettingsPage::BuildButton(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        std::wstring_view const& textKey,
        std::wstring_view const& textFallback,
        winrt::Windows::UI::Xaml::RoutedEventHandler const& handler)
    {
        winrt::Windows::UI::Xaml::Controls::Button button;
        button.Content(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(textKey, textFallback)));
        button.Margin(winrt::Windows::UI::Xaml::ThicknessHelper::FromLengths(0, 8, 0, 0));
        button.HorizontalAlignment(
            winrt::Windows::UI::Xaml::HorizontalAlignment::Left);
        button.Click(handler);
        parent.Children().Append(button);
        return button;
    }

    void SettingsPage::ApplyAppearanceSettings()
    {
        if (!m_Callbacks.Apply)
        {
            return;
        }

        K7_MODERN_APPEARANCE_SETTINGS settings = {};
        settings.ShowDots = m_ShowDotsToggle.IsOn() ? TRUE : FALSE;
        settings.ShowRealFileIcons = m_ShowRealFileIconsToggle.IsOn() ? TRUE : FALSE;
        settings.FullRow = m_FullRowToggle.IsOn() ? TRUE : FALSE;
        settings.ShowGrid = m_ShowGridToggle.IsOn() ? TRUE : FALSE;
        settings.SingleClick = m_SingleClickToggle.IsOn() ? TRUE : FALSE;
        settings.AlternativeSelection = m_AlternativeSelectionToggle.IsOn() ? TRUE : FALSE;
        settings.ShowSystemMenu = m_ShowSystemMenuToggle.IsOn() ? TRUE : FALSE;
        settings.ArcHistory = m_ArcHistoryToggle.IsOn() ? TRUE : FALSE;
        settings.PathHistory = m_PathHistoryToggle.IsOn() ? TRUE : FALSE;
        settings.CopyHistory = m_CopyHistoryToggle.IsOn() ? TRUE : FALSE;
        settings.FolderHistory = m_FolderHistoryToggle.IsOn() ? TRUE : FALSE;

        m_Callbacks.Apply(&settings);
    }

    void SettingsPage::ApplyContextMenuSettings()
    {
        if (!m_Callbacks.ContextMenuApply)
        {
            return;
        }

        K7_MODERN_CONTEXT_MENU_SETTINGS settings = {};
        settings.ShowOpen = m_CtxOpenToggle.IsOn() ? TRUE : FALSE;
        settings.ShowTest = m_CtxTestToggle.IsOn() ? TRUE : FALSE;
        settings.ShowExtract = m_CtxExtractToggle.IsOn() ? TRUE : FALSE;
        settings.ShowExtractHere = m_CtxExtractHereToggle.IsOn() ? TRUE : FALSE;
        settings.ShowExtractHereSmart = m_CtxExtractHereSmartToggle.IsOn() ? TRUE : FALSE;
        settings.ShowExtractTo = m_CtxExtractToToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompress = m_CtxCompressToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompressTo7z = m_CtxCompressTo7zToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompressToZip = m_CtxCompressToZipToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompressEmail = m_CtxCompressEmailToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompressTo7zEmail = m_CtxCompressTo7zEmailToggle.IsOn() ? TRUE : FALSE;
        settings.ShowCompressToZipEmail = m_CtxCompressToZipEmailToggle.IsOn() ? TRUE : FALSE;
        settings.ShowHash = m_CtxHashToggle.IsOn() ? TRUE : FALSE;
        settings.CascadedMenu = m_CtxCascadedToggle.IsOn() ? TRUE : FALSE;
        settings.EliminateDuplicateFiles = m_CtxElimDupToggle.IsOn() ? TRUE : FALSE;
        settings.ExtractOnOpen = m_ExtractOnOpenToggle.IsOn() ? TRUE : FALSE;

        m_Callbacks.ContextMenuApply(&settings);
    }

    void SettingsPage::ApplyCompressionSettings()
    {
        if (!m_Callbacks.CompressionApply)
        {
            return;
        }

        K7_MODERN_COMPRESSION_SETTINGS settings = {};

        int formatIndex = m_FormatCombo.SelectedIndex();
        if (formatIndex >= 0 &&
            formatIndex < static_cast<int>(ArraySize(kCompressionFormats)))
        {
            ::wcsncpy_s(
                settings.ArchiveType,
                ArraySize(settings.ArchiveType),
                kCompressionFormats[formatIndex].ArcType.data(),
                _TRUNCATE);
        }
        else
        {
            ::wcsncpy_s(
                settings.ArchiveType,
                ArraySize(settings.ArchiveType),
                L"7z",
                _TRUNCATE);
        }

        int levelIndex = m_LevelCombo.SelectedIndex();
        if (levelIndex >= 0 &&
            levelIndex < static_cast<int>(ArraySize(kCompressionLevels)))
        {
            settings.Level = kCompressionLevels[levelIndex].Level;
        }
        else
        {
            settings.Level = 5;
        }

        m_Callbacks.CompressionApply(&settings);
    }

    void SettingsPage::ApplyExtractionSettings()
    {
        if (!m_Callbacks.ExtractionApply)
        {
            return;
        }

        K7_MODERN_EXTRACTION_SETTINGS settings = {};

        int pathModeIndex = m_PathModeCombo.SelectedIndex();
        if (pathModeIndex >= 0 &&
            pathModeIndex < static_cast<int>(ArraySize(kExtractionPathModes)))
        {
            settings.PathMode = kExtractionPathModes[pathModeIndex].Value;
        }

        int overwriteIndex = m_OverwriteModeCombo.SelectedIndex();
        if (overwriteIndex >= 0 &&
            overwriteIndex < static_cast<int>(ArraySize(kExtractionOverwriteModes)))
        {
            settings.OverwriteMode = kExtractionOverwriteModes[overwriteIndex].Value;
        }

        settings.OpenFolderAfterExtraction =
            m_OpenFolderToggle.IsOn() ? TRUE : FALSE;

        m_Callbacks.ExtractionApply(&settings);
    }

    void SettingsPage::RefreshFileAssociationList()
    {
        if (!m_AssociationList)
        {
            return;
        }

        m_AssociationList.Items().Clear();


        for (wchar_t const* const* it = kCommonAssociationExtensions;
            it != kCommonAssociationExtensions + ArraySize(kCommonAssociationExtensions);
            ++it)
        {
            std::wstring extension(*it);


            BOOL IsDefault = FALSE;
            wchar_t CurrentAppName[MAX_PATH] = {};
            try
            {
            ::K7ModernQueryFileAssociation(
                extension.c_str(),
                &IsDefault,
                CurrentAppName,
                static_cast<UINT32>(ArraySize(CurrentAppName)));
            }
            catch (...)
            {
            }


            std::wstring statusText;
            if (IsDefault)
            {
                statusText = std::wstring(
                    winrt::NanaZip::Modern::GetUiString(
                        L"SettingsPage/AssocStatusDefault.Text",
                        L"Default app"));
            }
            else if (CurrentAppName[0] != L'\0')
            {
                std::wstring format = std::wstring(
                    winrt::NanaZip::Modern::GetUiString(
                        L"SettingsPage/AssocStatusOther.Text",
                        L"Opens with {0}"));
                statusText = ReplaceAll(format, L"{0}", CurrentAppName);
            }
            else
            {
                statusText = std::wstring(
                    winrt::NanaZip::Modern::GetUiString(
                        L"SettingsPage/AssocStatusNone.Text",
                        L"No default app"));
            }

            winrt::Windows::UI::Xaml::Controls::StackPanel row;
            row.Orientation(winrt::Windows::UI::Xaml::Controls::Orientation::Horizontal);
            row.Spacing(12);

            winrt::Windows::UI::Xaml::Controls::TextBlock extensionText;
            extensionText.Text(winrt::hstring(extension));
            extensionText.MinWidth(56);
            extensionText.FontWeight(
                winrt::Windows::UI::Text::FontWeights::SemiBold());
            row.Children().Append(extensionText);

            winrt::Windows::UI::Xaml::Controls::TextBlock statusBlock;
            statusBlock.Text(winrt::hstring(statusText));
            statusBlock.VerticalAlignment(VerticalAlignment::Center);
            row.Children().Append(statusBlock);

            winrt::Windows::UI::Xaml::Controls::ListViewItem item;
            item.Content(row);
            m_AssociationList.Items().Append(item);

        }
    }

    void SettingsPage::PostLegacyCommand(int command)
    {
        if (this->m_WindowHandle)
        {
            ::PostMessageW(
                this->m_WindowHandle,
                WM_COMMAND,
                MAKEWPARAM(command, BN_CLICKED),
                0);
        }
    }

    void SettingsPage::GeneralLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::ExtractionFoldersButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyFolders);
    }

    void SettingsPage::ExtractionIntegrationButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyIntegration);
    }

    void SettingsPage::FileAssociationsSystemButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        // 打开 Windows 官方"默认应用"设置页(携带当前包 AUMID),
        // 与经典 Integration 页按钮行为一致;不绕过 Windows 11 的
        // 用户确认机制。
        ::K7ModernLaunchDefaultAppsSettings();
    }

    void SettingsPage::FileAssociationsRefreshButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->RefreshFileAssociationList();
    }

    void SettingsPage::FileAssociationsLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyIntegration);
    }

    void SettingsPage::AppearanceLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::AdvancedLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::AdvancedEditorButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyEditor);
    }

    void SettingsPage::AboutDialogButtonClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        ::K7ModernShowAboutDialog(this->m_WindowHandle, nullptr);
    }
}

EXTERN_C LPVOID WINAPI K7ModernCreateSettingsPage(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ const K7_MODERN_SETTINGS_CALLBACKS* Callbacks)
{
    using Interface =
        winrt::NanaZip::Modern::SettingsPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::SettingsPage;

    Interface Window = winrt::make<Implementation>(
        ParentWindowHandle,
        Callbacks);
    return winrt::detach_abi(Window);
}

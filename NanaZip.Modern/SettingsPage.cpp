#include "pch.h"
#include "SettingsPage.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "UiStrings.h"

#include "NanaZip.Modern.h"

#include <shlobj.h>

#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Media.h>

// CuinZip P1-2: 经典设置页入口命令 ID。
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
}

namespace winrt
{
    using Windows::UI::Xaml::Controls::Button;
    using Windows::UI::Xaml::Controls::ColumnDefinition;
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
        _In_opt_ K7_MODERN_SETTINGS_LOAD_CALLBACK LoadCallback,
        _In_opt_ K7_MODERN_SETTINGS_APPLY_CALLBACK ApplyCallback) :
        m_WindowHandle(WindowHandle),
        m_LoadCallback(LoadCallback),
        m_ApplyCallback(ApplyCallback)
    {
        this->InitializeComponent();
    }

    void SettingsPage::InitializeComponent()
    {
        using namespace winrt;

        // ================== 根布局:左右两栏 ==================
        Grid root;
        ColumnDefinition leftColumn;
        leftColumn.Width(GridLengthHelper::FromPixels(200));
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

        // ---------- General ----------
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
                L"Core file manager behavior options are currently provided by the classic settings pages.",
                0, false, true, UniformThickness(0)));
            this->BuildButton(
                panel,
                L"SettingsPage/GeneralLegacyButton.Content",
                L"Open classic settings (General)",
                { this, &SettingsPage::GeneralLegacyButtonClick });
        }
        content.Children().Append(m_Panels[CategoryGeneral]);

        // ---------- Compression ----------
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
                L"Compression options are configured in the archive creation dialog. More options will be available in a future update.",
                0, false, true, UniformThickness(0)));
        }
        content.Children().Append(m_Panels[CategoryCompression]);

        // ---------- Extraction ----------
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
                L"Choose where extracted files are placed and how extraction integrates with your system.",
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

        // ---------- File Associations ----------
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

        // ---------- Context Menu ----------
        m_Panels[CategoryContextMenu] = StackPanel();
        {
            auto& panel = m_Panels[CategoryContextMenu];
            panel.Spacing(8);
            panel.Visibility(Visibility::Collapsed);
            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuTitle.Text", L"Context Menu",
                18, true, false, UniformThickness(0)));
            panel.Children().Append(MakeText(
                L"SettingsPage/ContextMenuDescription.Text",
                L"Configure the entries shown in the Windows Explorer context menu.",
                0, false, true, UniformThickness(0)));
            this->BuildButton(
                panel,
                L"SettingsPage/ContextMenuLegacyButton.Content",
                L"Open classic settings (Context menu)",
                { this, &SettingsPage::ContextMenuLegacyButtonClick });
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

            // 初始化期间 Toggled 被抑制,直接按当前设置构建开关。
            K7_MODERN_APPEARANCE_SETTINGS settings = {};
            if (this->m_LoadCallback)
            {
                this->m_LoadCallback(&settings);
            }

            m_ShowDotsToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowDotsToggle.Header",
                L"Show \"..\" item",
                settings.ShowDots != FALSE);
            m_ShowRealFileIconsToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowRealFileIconsToggle.Header",
                L"Show real file icons",
                settings.ShowRealFileIcons != FALSE);
            m_FullRowToggle = this->BuildToggle(
                panel,
                L"SettingsPage/FullRowToggle.Header",
                L"Full row select",
                settings.FullRow != FALSE);
            m_ShowGridToggle = this->BuildToggle(
                panel,
                L"SettingsPage/ShowGridToggle.Header",
                L"Show grid lines",
                settings.ShowGrid != FALSE);
            m_SingleClickToggle = this->BuildToggle(
                panel,
                L"SettingsPage/SingleClickToggle.Header",
                L"Single-click to open an item",
                settings.SingleClick != FALSE);
            m_AlternativeSelectionToggle = this->BuildToggle(
                panel,
                L"SettingsPage/AlternativeSelectionToggle.Header",
                L"Alternative selection mode",
                settings.AlternativeSelection != FALSE);

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
        bool isOn)
    {
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch toggle;
        toggle.Header(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(headerKey, headerFallback)));
        toggle.Margin(winrt::Windows::UI::Xaml::ThicknessHelper::FromLengths(0, 4, 0, 0));
        toggle.IsOn(isOn);
        toggle.Toggled({ this, &SettingsPage::AppearanceToggleToggled });
        parent.Children().Append(toggle);
        return toggle;
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
        button.Click(handler);
        parent.Children().Append(button);
        return button;
    }

    void SettingsPage::ApplyAppearanceSettings()
    {
        if (!this->m_ApplyCallback)
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

        this->m_ApplyCallback(&settings);
    }

    void SettingsPage::AppearanceToggleToggled(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        if (this->m_Initializing)
        {
            return;
        }
        this->ApplyAppearanceSettings();
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
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::ExtractionFoldersButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyFolders);
    }

    void SettingsPage::ExtractionIntegrationButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyIntegration);
    }

    void SettingsPage::FileAssociationsSystemButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        // 与经典 Integration 页的"打开 Windows 设置"按钮行为一致。
        SHELLEXECUTEINFOW execInfo = {};
        execInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
        execInfo.lpVerb = L"open";
        execInfo.lpFile = L"ms-settings:defaultapps";
        execInfo.nShow = SW_SHOWNORMAL;
        ::ShellExecuteExW(&execInfo);
    }

    void SettingsPage::FileAssociationsLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyIntegration);
    }

    void SettingsPage::ContextMenuLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyIntegration);
    }

    void SettingsPage::AppearanceLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::AdvancedLegacyButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacySettings);
    }

    void SettingsPage::AdvancedEditorButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        this->PostLegacyCommand(LegacyEditor);
    }

    void SettingsPage::AboutDialogButtonClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);
        ::K7ModernShowAboutDialog(this->m_WindowHandle, nullptr);
    }
}

EXTERN_C LPVOID WINAPI K7ModernCreateSettingsPage(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ K7_MODERN_SETTINGS_LOAD_CALLBACK LoadCallback,
    _In_opt_ K7_MODERN_SETTINGS_APPLY_CALLBACK ApplyCallback)
{
    using Interface =
        winrt::NanaZip::Modern::SettingsPage;
    using Implementation =
        winrt::NanaZip::Modern::implementation::SettingsPage;

    Interface Window = winrt::make<Implementation>(
        ParentWindowHandle,
        LoadCallback,
        ApplyCallback);
    return winrt::detach_abi(Window);
}

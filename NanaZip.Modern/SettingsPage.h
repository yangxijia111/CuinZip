#pragma once

#include "SettingsPage.g.h"

#include <Windows.h>

#include "NanaZip.Modern.h"

#include <functional>
#include <vector>

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaZip::Modern::implementation
{
    // CuinZip P1-4: Modern 设置窗口。
    // 说明:本页采用纯代码构建 UI(不走 x:Class XAML 绑定管道),
    //     规避旧版 XAML 编译器(V8.2 CompileXaml)对新页面的绑定注册
    //     缺陷;文本经 GetUiString 从 SettingsPage.resw(PRI)读取,
    //     本地化能力与 XAML x:Uid 等效。
    // - 左侧为八个分类,右侧为分类内容;
    // - 所有设置均通过宿主回调(FileManager)读写真实配置源:
    //   CFmSettings(外观/常规)、CContextMenuInfo(右键菜单)、
    //   NCompression::CInfo(压缩默认)、NExtract::CInfo(解压默认),
    //   本模块不直接访问注册表,不存在第二套设置;
    // - 文件关联状态经 K7ModernQueryFileAssociation 只读查询 Windows
    //   官方 UserChoice / Progid,默认应用的变更交由 Windows 设置;
    // - 其余复杂高级参数保留经典设置页入口,原有功能零删除。
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
    public:

        SettingsPage(
            _In_opt_ HWND WindowHandle = nullptr,
            _In_opt_ const K7_MODERN_SETTINGS_CALLBACKS* Callbacks = nullptr);

        void InitializeComponent();

    private:

        winrt::Windows::UI::Xaml::Controls::Button BuildButton(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            std::wstring_view const& textKey,
            std::wstring_view const& textFallback,
            winrt::Windows::UI::Xaml::RoutedEventHandler const& handler);

        winrt::Windows::UI::Xaml::Controls::ToggleSwitch BuildToggle(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            std::wstring_view const& headerKey,
            std::wstring_view const& headerFallback,
            bool isOn,
            std::function<void()> const& applyHandler);

        winrt::Windows::UI::Xaml::Controls::ComboBox BuildCombo(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            std::wstring_view const& headerKey,
            std::wstring_view const& headerFallback,
            std::vector<std::pair<std::wstring, int>> const& items,
            int selectedValue,
            std::function<void()> const& applyHandler);

        void ShowCategory(int index);

        // 各设置组的读取 / 应用(均经宿主回调,立即写真实配置源)。
        void LoadHostSettings();
        void ApplyAppearanceSettings();
        void ApplyContextMenuSettings();
        void ApplyCompressionSettings();
        void ApplyExtractionSettings();

        void RefreshFileAssociationList();

        void PostLegacyCommand(int command);

        void NavListSelectionChanged(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void GeneralLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void ExtractionFoldersButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void ExtractionIntegrationButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void FileAssociationsSystemButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void FileAssociationsRefreshButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void FileAssociationsLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void AppearanceLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void AdvancedLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void AdvancedEditorButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void AboutDialogButtonClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        HWND m_WindowHandle;
        K7_MODERN_SETTINGS_CALLBACKS m_Callbacks = {};
        bool m_Initializing = true;

        // 宿主设置快照(页面加载时读取,修改时整体应用)。
        K7_MODERN_APPEARANCE_SETTINGS m_Appearance = {};
        K7_MODERN_CONTEXT_MENU_SETTINGS m_ContextMenu = {};
        K7_MODERN_COMPRESSION_SETTINGS m_Compression = {};
        K7_MODERN_EXTRACTION_SETTINGS m_Extraction = {};

        // 控件引用(代码构建,不依赖 x:Name 绑定)。
        winrt::Windows::UI::Xaml::Controls::ListView m_NavList{ nullptr };
        winrt::Windows::UI::Xaml::Controls::StackPanel m_Panels[8]
        {
            nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr
        };

        // Appearance / General。
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowDotsToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowRealFileIconsToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_FullRowToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowGridToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_SingleClickToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_AlternativeSelectionToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowSystemMenuToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ArcHistoryToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_PathHistoryToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CopyHistoryToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_FolderHistoryToggle{ nullptr };

        // Compression。
        winrt::Windows::UI::Xaml::Controls::ComboBox m_FormatCombo{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ComboBox m_LevelCombo{ nullptr };

        // Extraction。
        winrt::Windows::UI::Xaml::Controls::ComboBox m_PathModeCombo{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ComboBox m_OverwriteModeCombo{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_OpenFolderToggle{ nullptr };

        // File Associations。
        winrt::Windows::UI::Xaml::Controls::ListView m_AssociationList{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ExtractOnOpenToggle{ nullptr };
        std::vector<std::wstring> m_AssociationExtensions;

        // Context Menu。
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxOpenToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxTestToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxExtractToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxExtractHereToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxExtractHereSmartToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxExtractToToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressTo7zToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressToZipToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressEmailToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressTo7zEmailToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCompressToZipEmailToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxHashToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxCascadedToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_CtxElimDupToggle{ nullptr };
    };
}

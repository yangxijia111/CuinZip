#pragma once

#include "SettingsPage.g.h"

#include <Windows.h>

#include "NanaZip.Modern.h"

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaZip::Modern::implementation
{
    // CuinZip P1-2: Modern 设置窗口(第一轮)。
    // 说明:本页采用纯代码构建 UI(不走 x:Class XAML 绑定管道),
    //     规避旧版 XAML 编译器(V8.2 CompileXaml)对新页面的绑定注册
    //     缺陷;文本经 GetUiString 从 SettingsPage.resw(PRI)读取,
    //     本地化能力与 XAML x:Uid 等效。
    // - 左侧为八个分类,右侧为分类内容;
    // - Appearance 分类提供即时生效的开关(通过宿主回调读写 CFmSettings,
    //   本模块不直接访问注册表);
    // - 其余分类提供对应经典设置页入口(WM_COMMAND 方式),
    //   原有设置功能全部保留。
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
    public:

        SettingsPage(
            _In_opt_ HWND WindowHandle = nullptr,
            _In_opt_ K7_MODERN_SETTINGS_LOAD_CALLBACK LoadCallback = nullptr,
            _In_opt_ K7_MODERN_SETTINGS_APPLY_CALLBACK ApplyCallback = nullptr);

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
            bool isOn);

        void ShowCategory(int index);
        void ApplyAppearanceSettings();
        void PostLegacyCommand(int command);

        void NavListSelectionChanged(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void AppearanceToggleToggled(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void GeneralLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void ExtractionFoldersButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void ExtractionIntegrationButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void FileAssociationsSystemButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void FileAssociationsLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void ContextMenuLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void AppearanceLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void AdvancedLegacyButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void AdvancedEditorButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        void AboutDialogButtonClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        HWND m_WindowHandle;
        K7_MODERN_SETTINGS_LOAD_CALLBACK m_LoadCallback;
        K7_MODERN_SETTINGS_APPLY_CALLBACK m_ApplyCallback;
        bool m_Initializing = true;

        // 控件引用(代码构建,不依赖 x:Name 绑定)。
        winrt::Windows::UI::Xaml::Controls::ListView m_NavList{ nullptr };
        winrt::Windows::UI::Xaml::Controls::StackPanel m_Panels[8]{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowDotsToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowRealFileIconsToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_FullRowToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_ShowGridToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_SingleClickToggle{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ToggleSwitch m_AlternativeSelectionToggle{ nullptr };
    };
}

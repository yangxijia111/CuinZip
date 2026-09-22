#pragma once

#include "ExtractDialogPage.g.h"

#include <Windows.h>

#include "NanaZip.Modern.h"

#include <vector>

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaZip::Modern::implementation
{
    // CuinZip P1-3:Modern "解压" 对话框(纯代码 UI)。
    // 分区:Destination / Options / Password;
    // 字段标签直接取自隐藏原对话框(沿用 7-Zip 本地化),新增文案
    // (分区标题/按钮/文件夹名标签)取自 ExtractDialogPage.resw。
    struct ExtractDialogPage : ExtractDialogPageT<ExtractDialogPage>
    {
    public:

        ExtractDialogPage(
            _In_opt_ HWND WindowHandle,
            _In_opt_ const K7_DIALOG_MIRROR_ENGINE* Engine);

        void InitializeComponent();

    private:

        // ---- 构建辅助 ----

        winrt::Windows::UI::Xaml::Controls::StackPanel BuildSection(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            std::wstring_view headerKey,
            std::wstring_view headerFallback,
            bool expanded);

        winrt::Windows::UI::Xaml::Controls::TextBlock BuildLabel(
            UINT controlId);

        void ApplyTextStyle(
            winrt::Windows::UI::Xaml::Controls::TextBlock const& target,
            wchar_t const* styleKey);

        void BuildComboField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT labelId,
            UINT comboId,
            bool editable);

        void BuildCheckField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT checkId);

        void BuildPasswordField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent);

        // ---- 状态同步 ----

        void SyncAll();
        void SyncCombos();
        void SyncChecks();
        void SyncPasswordSlot();
        void PushEditableTexts();

        // ---- 交互 ----

        void OnMirrorComboChanged(UINT comboId, int selection);
        void OnMirrorCheckClicked(UINT checkId, bool checked_);
        void OnBrowseClick();
        void OkButtonClick();
        void CancelButtonClick();
        void Finish(UINT resultCode);
        // 输入控件(组合框/文本/密码/复选)的 Enter/Esc 直达处理:
        // 可编辑组合框会吞掉 Enter,keydown 不冒泡,Windows 对话框习惯
        // 要求 Enter=确定/Esc=取消在任何焦点下成立
        void OnFieldKeyDown(
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        // Enter/Esc 键盘加速器(焦点在可编辑组合框内时 Enter 会被控件
        // 吞掉,KeyDown 不冒泡;加速器优先于焦点控件,保证对话框习惯)
        void InitializeAccelerators(
            winrt::Windows::UI::Xaml::Controls::Grid const& root);
        void OnRootKeyDown(
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e);

        HWND m_WindowHandle;
        const K7_DIALOG_MIRROR_ENGINE* m_Engine;
        bool m_Suppress = false;
        bool m_PasswordSeeded = false;

        struct ComboEntry
        {
            UINT Id;
            bool Editable;
            winrt::Windows::UI::Xaml::Controls::ComboBox Control{ nullptr };
        };
        struct CheckEntry
        {
            UINT Id;
            winrt::Windows::UI::Xaml::Controls::CheckBox Control{ nullptr };
        };
        std::vector<ComboEntry> m_Combos;
        std::vector<CheckEntry> m_Checks;

        winrt::Windows::UI::Xaml::Controls::PasswordBox m_PasswordBox{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBox m_PasswordPlain{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBox m_NameEdit{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Button m_OkButton{ nullptr };
    };
}

#pragma once

#include "CompressDialogPage.g.h"

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
    // CuinZip P1-3:Modern "添加到压缩包" 对话框(纯代码 UI)。
    // 分区:Archive / Compression / Encryption / Advanced;
    // 字段标签直接取自隐藏原对话框(沿用 7-Zip 本地化),新增文案
    // (分区标题/按钮)取自 CompressDialogPage.resw。
    struct CompressDialogPage : CompressDialogPageT<CompressDialogPage>
    {
    public:

        CompressDialogPage(
            _In_opt_ HWND WindowHandle,
            _In_opt_ const K7_DIALOG_MIRROR_ENGINE* Engine);

        void InitializeComponent();

    private:

        // ---- 构建辅助 ----

        // 建立可折叠分区(壳已附加到 parent),返回内容面板
        winrt::Windows::UI::Xaml::Controls::StackPanel BuildSection(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            std::wstring_view headerKey,
            std::wstring_view headerFallback,
            bool expanded);

        // 字段标签(自镜像对话框读取并清理加速键与冒号)
        winrt::Windows::UI::Xaml::Controls::TextBlock BuildLabel(
            UINT controlId);

        void ApplyTextStyle(
            winrt::Windows::UI::Xaml::Controls::TextBlock const& target,
            wchar_t const* styleKey);

        // 建立镜像组合框;editable 时文本经 PushEditableTexts 统一写回
        void BuildComboField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT labelId,
            UINT comboId,
            bool editable);

        // 建立镜像复选框
        void BuildCheckField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT checkId);

        // 建立镜像文本框(Parameters)
        void BuildTextField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT labelId,
            UINT editId);

        // 建立密码字段:标签 + (PasswordBox / 明文 TextBox) 叠放槽位
        void BuildPasswordField(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            UINT labelId,
            UINT editId);

        // ---- 状态同步 ----

        void SyncAll();
        void SyncCombos();
        void SyncChecks();
        void SyncTexts();
        void SyncPasswordSlot(
            UINT editId,
            UINT showCheckId,
            winrt::Windows::UI::Xaml::Controls::TextBlock& label,
            winrt::Windows::UI::Xaml::Controls::PasswordBox& passwordBox,
            winrt::Windows::UI::Xaml::Controls::TextBox& plainBox);
        void SyncStaticText(
            winrt::Windows::UI::Xaml::Controls::TextBlock& target,
            UINT sourceId);

        // 把可编辑组合框/密码/参数的当前文本写回引擎,
        // 必须在任何触发原对话框逻辑的操作前调用
        void PushEditableTexts();

        // ---- 交互 ----

        void OnMirrorComboChanged(UINT comboId, int selection);
        void OnMirrorCheckClicked(UINT checkId, bool checked_);
        void OnNestedModalButtonClick(UINT buttonId);
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

        // ---- 镜像控件登记表 ----
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
        struct TextEntry
        {
            UINT Id;
            winrt::Windows::UI::Xaml::Controls::TextBox Control{ nullptr };
        };
        std::vector<ComboEntry> m_Combos;
        std::vector<CheckEntry> m_Checks;
        std::vector<TextEntry> m_Texts;

        // ---- 专项控件 ----
        winrt::Windows::UI::Xaml::Controls::TextBlock m_FolderText{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_MemoryText{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_MemoryDeLabel{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_MemoryDeText{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_OptionsText{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_Password1Label{ nullptr };
        winrt::Windows::UI::Xaml::Controls::PasswordBox m_Password1Box{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBox m_Password1Plain{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_Password2Label{ nullptr };
        winrt::Windows::UI::Xaml::Controls::PasswordBox m_Password2Box{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBox m_Password2Plain{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Button m_OkButton{ nullptr };
    };
}

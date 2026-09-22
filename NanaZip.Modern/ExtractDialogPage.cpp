#include "pch.h"
#include "ExtractDialogPage.h"
#if __has_include("ExtractDialogPage.g.cpp")
#include "ExtractDialogPage.g.cpp"
#endif

#include "UiStrings.h"
#include "DialogMirrorUi.h"

#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.System.h>

#include <string>
#include <vector>

namespace winrt
{
    using Windows::UI::Xaml::Automation::AutomationProperties;
    using Windows::UI::Xaml::Controls::Button;
    using Windows::UI::Xaml::Controls::CheckBox;
    using Windows::UI::Xaml::Controls::ComboBox;
    using Windows::UI::Xaml::Controls::FontIcon;
    using Windows::UI::Xaml::Controls::Grid;
    using Windows::UI::Xaml::Controls::Panel;
    using Windows::UI::Xaml::Controls::PasswordBox;
    using Windows::UI::Xaml::Controls::RowDefinition;
    using Windows::UI::Xaml::Controls::ScrollViewer;
    using Windows::UI::Xaml::Controls::StackPanel;
    using Windows::UI::Xaml::Controls::TextBlock;
    using Windows::UI::Xaml::Controls::TextBox;
    using Windows::UI::Xaml::Controls::ColumnDefinition;
    using Windows::UI::Xaml::Controls::Orientation;
    using Windows::UI::Xaml::HorizontalAlignment;
    using Windows::UI::Xaml::Visibility;
    using Windows::System::VirtualKey;
    using Windows::UI::Xaml::Application;
    using Windows::UI::Xaml::Style;
    using Windows::UI::Xaml::GridUnitType;
    using Windows::UI::Xaml::GridLengthHelper;
    using Windows::UI::Xaml::ThicknessHelper;
}

namespace
{
    // 镜像控件 ID(与 ExtractDialogRes.h 对应)
    constexpr UINT IdPathCombo = 100;           // IDC_EXTRACT_PATH
    constexpr UINT IdBrowseButton = 101;        // IDB_EXTRACT_SET_PATH
    constexpr UINT IdPathModeCombo = 102;       // IDC_EXTRACT_PATH_MODE
    constexpr UINT IdOverwriteModeCombo = 103;  // IDC_EXTRACT_OVERWRITE_MODE
    constexpr UINT IdOpenTrgFoldCheck = 104;    // IDX_EXTRACT_OPEN_TRG_FLD(ZS)
    constexpr UINT IdPasswordEdit = 120;        // IDE_EXTRACT_PASSWORD
    constexpr UINT IdNameEdit = 130;            // IDE_EXTRACT_NAME
    constexpr UINT IdNameEnableCheck = 131;     // IDX_EXTRACT_NAME_ENABLE

    constexpr UINT IdExtractToLabel = 3401;     // IDT_EXTRACT_EXTRACT_TO
    constexpr UINT IdPathModeLabel = 3410;      // IDT_EXTRACT_PATH_MODE
    constexpr UINT IdOverwriteModeLabel = 3420; // IDT_EXTRACT_OVERWRITE_MODE
    constexpr UINT IdElimDupCheck = 3430;       // IDX_EXTRACT_ELIM_DUP
    constexpr UINT IdNtSecurCheck = 3431;       // IDX_EXTRACT_NT_SECUR
    constexpr UINT IdOpenFolderCheck = 3433;    // IDX_EXTRACT_OPEN_FOLDER(NanaZip)
    constexpr UINT IdShowPasswordCheck = 3803;  // IDX_PASSWORD_SHOW
}

namespace winrt::NanaZip::Modern::implementation
{
    using namespace MirrorUi;

    ExtractDialogPage::ExtractDialogPage(
        _In_opt_ HWND WindowHandle,
        _In_opt_ const K7_DIALOG_MIRROR_ENGINE* Engine) :
        m_WindowHandle(WindowHandle),
        m_Engine(Engine)
    {
    }

    void ExtractDialogPage::InitializeComponent()
    {
        // 纯代码 UI 页面:基类无 XAML InitializeComponent(同 SettingsPage)

        // 窗口标题沿用原对话框标题(已本地化,含 " : 压缩包名" 后缀)
        {
            TextSnapshot title = MirrorUi::ReadText(m_Engine, 0);
            if (m_WindowHandle && title.Ok && !title.Text.empty())
            {
                ::SetWindowTextW(m_WindowHandle, title.Text.c_str());
            }
        }

        Grid root;
        {
            RowDefinition contentRow;
            contentRow.Height(GridLengthHelper::FromValueAndType(
                1, GridUnitType::Star));
            root.RowDefinitions().Append(contentRow);
            RowDefinition buttonRow;
            buttonRow.Height(GridLengthHelper::Auto());
            root.RowDefinitions().Append(buttonRow);
        }

        ScrollViewer scroll;
        scroll.VerticalScrollBarVisibility(
            winrt::Windows::UI::Xaml::Controls::ScrollBarVisibility::Auto);
        winrt::Grid::SetRow(scroll, 0);

        StackPanel content;
        content.Margin(winrt::ThicknessHelper::FromLengths(24, 16, 24, 8));
        content.Spacing(4);
        scroll.Content(content);
        root.Children().Append(scroll);

        // ==================== Destination 分区 ====================
        StackPanel destinationSection = this->BuildSection(
            content, L"DestinationSection", L"Destination", true);

        this->BuildComboField(
            destinationSection, IdExtractToLabel, IdPathCombo, true);

        // 浏览按钮(标题沿用原对话框本地化文本)
        {
            Button browse;
            TextSnapshot caption = MirrorUi::ReadText(
                m_Engine, IdBrowseButton);
            browse.Content(winrt::box_value(
                caption.Ok && !caption.Text.empty()
                ? StripAccelerator(caption.Text)
                : winrt::hstring(L"Browse...")));
            browse.Margin(winrt::ThicknessHelper::FromLengths(0, 6, 0, 4));
            browse.Click([this](auto&&, auto&&)
            {
                this->OnBrowseClick();
            });
            destinationSection.Children().Append(browse);
        }

        // 拆分目标名(启用复选框 + 名称输入行;无原生标签,用 resw)
        this->BuildCheckField(destinationSection, IdNameEnableCheck);
        {
            TextBlock label;
            label.Text(winrt::NanaZip::Modern::GetUiString(
                L"ExtractDialogPage/NameLabelText", L"Folder name"));
            label.Margin(winrt::ThicknessHelper::FromLengths(0, 8, 0, 2));
            destinationSection.Children().Append(label);

            m_NameEdit = winrt::TextBox();
            m_NameEdit.KeyDown([this](
                winrt::IInspectable const&,
                winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
            {
                this->OnFieldKeyDown(e);
            });
            winrt::AutomationProperties::SetName(
                m_NameEdit, winrt::hstring(label.Text()));
            destinationSection.Children().Append(m_NameEdit);
        }

        this->BuildComboField(
            destinationSection, IdPathModeLabel, IdPathModeCombo, false);
        this->BuildComboField(
            destinationSection, IdOverwriteModeLabel, IdOverwriteModeCombo,
            false);

        // ==================== Options 分区 ====================
        StackPanel optionsSection = this->BuildSection(
            content, L"OptionsSection", L"Options", true);

        this->BuildCheckField(optionsSection, IdElimDupCheck);
        this->BuildCheckField(optionsSection, IdNtSecurCheck);
        this->BuildCheckField(optionsSection, IdOpenTrgFoldCheck);
        this->BuildCheckField(optionsSection, IdOpenFolderCheck);

        // ==================== Password 分区 ====================
        StackPanel passwordSection = this->BuildSection(
            content, L"PasswordSection", L"Password", true);

        this->BuildPasswordField(passwordSection);
        this->BuildCheckField(passwordSection, IdShowPasswordCheck);

        // ==================== 按钮栏 ====================
        StackPanel buttons;
        buttons.Orientation(winrt::Orientation::Horizontal);
        buttons.HorizontalAlignment(winrt::HorizontalAlignment::Right);
        buttons.Spacing(8);
        buttons.Margin(winrt::ThicknessHelper::FromLengths(24, 12, 24, 16));
        winrt::Grid::SetRow(buttons, 1);

        Button cancel;
        cancel.Content(winrt::box_value(winrt::NanaZip::Modern::GetUiString(
            L"ExtractDialogPage/CancelButtonText", L"Cancel")));
        cancel.Click([this](auto&&, auto&&)
        {
            this->CancelButtonClick();
        });
        buttons.Children().Append(cancel);

        m_OkButton = winrt::Button();
        m_OkButton.Content(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(
                L"ExtractDialogPage/OkButtonText", L"Extract")));
        try
        {
            m_OkButton.Style(Application::Current().Resources()
                .Lookup(winrt::box_value(L"AccentButtonStyle"))
                .as<winrt::Style>());
        }
        catch (...)
        {
        }
        m_OkButton.Click([this](auto&&, auto&&)
        {
            this->OkButtonClick();
        });
        buttons.Children().Append(m_OkButton);
        root.Children().Append(buttons);

        // Enter=Extract / Esc=Cancel;焦点在可编辑组合框/文本框内时
        // 按键可能被控件处理而不冒泡,用键盘加速器兜底 + KeyDown 双保险
        this->InitializeAccelerators(root);
        // 隧道事件:在焦点控件(可编辑组合框)处理 Enter 之前截获
        root.PreviewKeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        root.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnRootKeyDown(e);
        });

        this->Content(root);

        this->SyncAll();
    }

    void ExtractDialogPage::InitializeAccelerators(
        winrt::Windows::UI::Xaml::Controls::Grid const& root)
    {
        using winrt::Windows::UI::Xaml::Input::KeyboardAccelerator;

        KeyboardAccelerator enterKey;
        enterKey.Key(winrt::VirtualKey::Enter);
        enterKey.Invoked([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::
                KeyboardAcceleratorInvokedEventArgs const& e)
        {
            e.Handled(true);
            this->OkButtonClick();
        });
        root.KeyboardAccelerators().Append(enterKey);

        KeyboardAccelerator escapeKey;
        escapeKey.Key(winrt::VirtualKey::Escape);
        escapeKey.Invoked([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::
                KeyboardAcceleratorInvokedEventArgs const& e)
        {
            e.Handled(true);
            this->CancelButtonClick();
        });
        root.KeyboardAccelerators().Append(escapeKey);
    }

    winrt::Windows::UI::Xaml::Controls::StackPanel
    ExtractDialogPage::BuildSection(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        std::wstring_view headerKey,
        std::wstring_view headerFallback,
        bool expanded)
    {
        StackPanel section;
        section.Spacing(4);
        section.Margin(winrt::ThicknessHelper::FromLengths(0, 0, 0, 8));

        StackPanel sectionContent;
        sectionContent.Visibility(expanded
            ? winrt::Visibility::Visible
            : winrt::Visibility::Collapsed);
        sectionContent.Spacing(4);
        sectionContent.Margin(winrt::ThicknessHelper::FromLengths(2, 6, 2, 0));

        Button header;
        header.HorizontalAlignment(winrt::HorizontalAlignment::Stretch);
        header.HorizontalContentAlignment(
            winrt::HorizontalAlignment::Left);
        header.Padding(winrt::ThicknessHelper::FromLengths(8, 6, 8, 6));
        {
            Grid headerGrid;
            ColumnDefinition star;
            star.Width(GridLengthHelper::FromValueAndType(
                1, GridUnitType::Star));
            ColumnDefinition tail;
            tail.Width(GridLengthHelper::Auto());
            headerGrid.ColumnDefinitions().Append(star);
            headerGrid.ColumnDefinitions().Append(tail);

            TextBlock text;
            text.Text(winrt::NanaZip::Modern::GetUiString(
                headerKey, headerFallback));
            text.FontWeight(
                winrt::Windows::UI::Text::FontWeights::SemiBold());
            winrt::Grid::SetColumn(text, 0);
            headerGrid.Children().Append(text);

            FontIcon chevron;
            chevron.Glyph(expanded ? L"\uE70E" : L"\uE76C");
            chevron.FontSize(12.0);
            winrt::Grid::SetColumn(chevron, 1);
            headerGrid.Children().Append(chevron);

            header.Content(headerGrid);

            header.Click(
                [sectionContent, chevron](auto&&, auto&&)
            {
                bool show =
                    sectionContent.Visibility() == winrt::Visibility::Collapsed;
                sectionContent.Visibility(show
                    ? winrt::Visibility::Visible
                    : winrt::Visibility::Collapsed);
                chevron.Glyph(show ? L"\uE70E" : L"\uE76C");
            });
        }

        section.Children().Append(header);
        section.Children().Append(sectionContent);
        parent.Children().Append(section);

        return sectionContent;
    }

    winrt::Windows::UI::Xaml::Controls::TextBlock
    ExtractDialogPage::BuildLabel(UINT controlId)
    {
        TextBlock label;
        TextSnapshot text = MirrorUi::ReadText(m_Engine, controlId);
        label.Text(text.Ok
            ? StripAcceleratorAndColon(text.Text)
            : winrt::hstring());
        label.Margin(winrt::ThicknessHelper::FromLengths(0, 8, 0, 2));
        return label;
    }

    void ExtractDialogPage::ApplyTextStyle(
        winrt::Windows::UI::Xaml::Controls::TextBlock const& target,
        wchar_t const* styleKey)
    {
        try
        {
            target.Style(Application::Current().Resources()
                .Lookup(winrt::box_value(styleKey))
                .as<winrt::Style>());
        }
        catch (...)
        {
        }
    }

    void ExtractDialogPage::BuildComboField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        UINT labelId,
        UINT comboId,
        bool editable)
    {
        TextBlock label = this->BuildLabel(labelId);
        winrt::AutomationProperties::SetName(
            label, winrt::hstring(label.Text()));

        ComboBox combo;
        combo.IsEditable(editable);
        // UWP 可编辑 ComboBox 无文本变更事件,文本统一经
        // PushEditableTexts 在触发引擎操作前写回
        combo.SelectionChanged([this, comboId](
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress)
                return;
            ComboBox source = sender.as<ComboBox>();
            this->OnMirrorComboChanged(comboId, source.SelectedIndex());
        });
        combo.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });

        winrt::AutomationProperties::SetName(
            combo, winrt::hstring(label.Text()));

        parent.Children().Append(label);
        parent.Children().Append(combo);

        ComboEntry entry;
        entry.Id = comboId;
        entry.Editable = editable;
        entry.Control = combo;
        m_Combos.push_back(std::move(entry));
    }

    void ExtractDialogPage::BuildCheckField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        UINT checkId)
    {
        CheckBox check;
        TextSnapshot caption = MirrorUi::ReadText(m_Engine, checkId);
        // IDX_EXTRACT_NAME_ENABLE 模板标题为空,用 resw 文案补齐
        winrt::hstring text;
        if (caption.Ok && !caption.Text.empty())
        {
            text = StripAccelerator(caption.Text);
        }
        else if (checkId == IdNameEnableCheck)
        {
            text = winrt::NanaZip::Modern::GetUiString(
                L"ExtractDialogPage/NameEnableText",
                L"Use folder name");
        }
        check.Content(winrt::box_value(text));
        check.Margin(winrt::ThicknessHelper::FromLengths(0, 8, 0, 0));
        check.Click([this, checkId](
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress)
                return;
            CheckBox source = sender.as<CheckBox>();
            this->OnMirrorCheckClicked(
                checkId, source.IsChecked().Value());
        });
        check.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        winrt::AutomationProperties::SetName(check, text);
        parent.Children().Append(check);

        CheckEntry entry;
        entry.Id = checkId;
        entry.Control = check;
        m_Checks.push_back(std::move(entry));
    }

    void ExtractDialogPage::BuildPasswordField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent)
    {
        TextBlock label = this->BuildLabel(3807); // IDG_PASSWORD "Password"

        m_PasswordBox = winrt::PasswordBox();
        m_PasswordPlain = winrt::TextBox();
        m_PasswordPlain.Visibility(winrt::Visibility::Collapsed);

        // 密码不落日志、不缓存到引擎以外的任何位置
        m_PasswordBox.PasswordChanged(
            [this](
                winrt::IInspectable const& sender,
                winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress || m_PasswordPlain.Visibility() ==
                winrt::Visibility::Visible)
                return;
            PasswordBox source = sender.as<PasswordBox>();
            if (m_Engine && m_Engine->SetText)
            {
                m_Engine->SetText(m_Engine->Context, IdPasswordEdit,
                    source.Password().c_str());
            }
        });

        m_PasswordPlain.TextChanged(
            [this](
                winrt::IInspectable const& sender,
                winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress)
                return;
            TextBox source = sender.as<TextBox>();
            if (m_Engine && m_Engine->SetText)
            {
                m_Engine->SetText(m_Engine->Context, IdPasswordEdit,
                    source.Text().c_str());
            }
        });

        m_PasswordBox.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        m_PasswordPlain.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        winrt::AutomationProperties::SetName(
            m_PasswordBox, winrt::hstring(label.Text()));
        winrt::AutomationProperties::SetName(
            m_PasswordPlain, winrt::hstring(label.Text()));

        Grid slot;
        slot.Children().Append(m_PasswordBox);
        slot.Children().Append(m_PasswordPlain);

        parent.Children().Append(label);
        parent.Children().Append(slot);
    }

    void ExtractDialogPage::SyncAll()
    {
        m_Suppress = true;

        this->SyncCombos();
        this->SyncChecks();
        this->SyncPasswordSlot();

        // 名称输入行跟随启用复选框显隐(原逻辑 ShowItem_Bool)
        if (m_NameEdit)
        {
            CheckSnapshot enable = MirrorUi::ReadCheck(
                m_Engine, IdNameEnableCheck);
            TextSnapshot snapshot = MirrorUi::ReadText(
                m_Engine, IdNameEdit);
            m_NameEdit.Text(snapshot.Ok
                ? winrt::hstring(snapshot.Text)
                : winrt::hstring());
            m_NameEdit.Visibility(enable.Ok && enable.Checked
                ? winrt::Visibility::Visible
                : winrt::Visibility::Collapsed);
        }

        m_Suppress = false;
    }

    void ExtractDialogPage::SyncCombos()
    {
        for (ComboEntry& entry : m_Combos)
        {
            ComboSnapshot snapshot = MirrorUi::ReadCombo(
                m_Engine, entry.Id);
            if (!snapshot.Ok || !entry.Control)
                continue;

            ComboBox combo = entry.Control;

            combo.Items().Clear();
            for (winrt::hstring const& item : snapshot.Items)
            {
                combo.Items().Append(winrt::box_value(item));
            }
            combo.SelectedIndex(snapshot.Selection);
            if (entry.Editable)
            {
                combo.Text(winrt::hstring(snapshot.Text));
            }
            combo.IsEnabled(snapshot.Enabled);
            combo.Visibility(snapshot.Visible
                ? winrt::Visibility::Visible
                : winrt::Visibility::Collapsed);
        }
    }

    void ExtractDialogPage::SyncChecks()
    {
        for (CheckEntry& entry : m_Checks)
        {
            CheckSnapshot snapshot = MirrorUi::ReadCheck(
                m_Engine, entry.Id);
            if (!snapshot.Ok || !entry.Control)
                continue;
            entry.Control.IsChecked(snapshot.Checked);
            entry.Control.IsEnabled(snapshot.Enabled);
            entry.Control.Visibility(snapshot.Visible
                ? winrt::Visibility::Visible
                : winrt::Visibility::Collapsed);
        }
    }

    void ExtractDialogPage::SyncPasswordSlot()
    {
        if (!m_PasswordBox || !m_PasswordPlain)
            return;

        CheckSnapshot show = MirrorUi::ReadCheck(
            m_Engine, IdShowPasswordCheck);
        TextSnapshot edit = MirrorUi::ReadText(m_Engine, IdPasswordEdit);
        bool showPlain = show.Ok ? show.Checked : false;

        if (!m_PasswordSeeded && edit.Ok)
        {
            m_PasswordSeeded = true;
            m_PasswordBox.Password(winrt::hstring(edit.Text));
            m_PasswordPlain.Text(winrt::hstring(edit.Text));
        }

        if (showPlain)
        {
            if (m_PasswordPlain.Visibility() == winrt::Visibility::Collapsed)
            {
                m_PasswordPlain.Text(
                    winrt::hstring(m_PasswordBox.Password()));
            }
            m_PasswordPlain.Visibility(winrt::Visibility::Visible);
            m_PasswordBox.Visibility(winrt::Visibility::Collapsed);
        }
        else
        {
            if (m_PasswordBox.Visibility() == winrt::Visibility::Collapsed)
            {
                m_PasswordBox.Password(m_PasswordPlain.Text());
            }
            m_PasswordBox.Visibility(winrt::Visibility::Visible);
            m_PasswordPlain.Visibility(winrt::Visibility::Collapsed);
        }
    }

    void ExtractDialogPage::PushEditableTexts()
    {
        if (!m_Engine)
            return;

        for (ComboEntry const& entry : m_Combos)
        {
            if (!entry.Editable || !entry.Control)
                continue;
            if (m_Engine->SetComboText)
            {
                m_Engine->SetComboText(
                    m_Engine->Context,
                    entry.Id,
                    entry.Control.Text().c_str());
            }
        }

        if (m_NameEdit && m_Engine->SetText)
        {
            m_Engine->SetText(
                m_Engine->Context,
                IdNameEdit,
                m_NameEdit.Text().c_str());
        }
    }

    void ExtractDialogPage::OnMirrorComboChanged(
        UINT comboId, int selection)
    {
        if (!m_Engine || !m_Engine->SetComboSelection)
            return;
        this->PushEditableTexts();
        m_Engine->SetComboSelection(m_Engine->Context, comboId, selection);
        this->SyncAll();
    }

    void ExtractDialogPage::OnMirrorCheckClicked(
        UINT checkId, bool checked_)
    {
        if (!m_Engine || !m_Engine->SetCheck)
            return;
        this->PushEditableTexts();
        m_Engine->SetCheck(
            m_Engine->Context, checkId, checked_ ? TRUE : FALSE);
        this->SyncAll();
    }

    void ExtractDialogPage::OnBrowseClick()
    {
        if (!m_Engine || !m_Engine->NotifyButtonClick)
            return;
        this->PushEditableTexts();
        if (m_WindowHandle)
            ::EnableWindow(m_WindowHandle, FALSE);
        m_Engine->NotifyButtonClick(m_Engine->Context, IdBrowseButton);
        if (m_WindowHandle)
        {
            ::EnableWindow(m_WindowHandle, TRUE);
            ::SetForegroundWindow(m_WindowHandle);
        }
        this->SyncAll();
    }

    void ExtractDialogPage::OkButtonClick()
    {
        if (!m_Engine || !m_Engine->PressOK)
            return;
        this->PushEditableTexts();
        if (m_Engine->PressOK(m_Engine->Context))
        {
            this->Finish(K7_DIALOG_MIRROR_RESULT_OK);
        }
        else
        {
            this->SyncAll();
        }
    }

    void ExtractDialogPage::CancelButtonClick()
    {
        this->Finish(K7_DIALOG_MIRROR_RESULT_CANCEL);
    }

    void ExtractDialogPage::Finish(UINT resultCode)
    {
        if (m_WindowHandle)
        {
            ::SendMessageW(
                m_WindowHandle,
                WM_COMMAND,
                MAKEWPARAM(resultCode, BN_CLICKED),
                0);
            ::PostMessageW(m_WindowHandle, WM_CLOSE, 0, 0);
        }
    }

    void ExtractDialogPage::OnFieldKeyDown(
        winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == winrt::VirtualKey::Enter)
        {
            e.Handled(true);
            this->OkButtonClick();
        }
        else if (e.Key() == winrt::VirtualKey::Escape)
        {
            e.Handled(true);
            this->CancelButtonClick();
        }
    }

    void ExtractDialogPage::OnRootKeyDown(
        winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == winrt::VirtualKey::Enter)
        {
            e.Handled(true);
            this->OkButtonClick();
        }
        else if (e.Key() == winrt::VirtualKey::Escape)
        {
            e.Handled(true);
            this->CancelButtonClick();
        }
    }
}

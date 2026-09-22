#include "pch.h"
#include "CompressDialogPage.h"
#if __has_include("CompressDialogPage.g.cpp")
#include "CompressDialogPage.g.cpp"
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
    using Windows::UI::Xaml::Thickness;
    using Windows::UI::Xaml::GridUnitType;
    using Windows::UI::Xaml::GridLengthHelper;
    using Windows::UI::Xaml::ThicknessHelper;
}

namespace
{
    // 镜像控件 ID(与 CompressDialogRes.h 对应;Modern 侧不包含 7-Zip 头)
    constexpr UINT IdArchiveCombo = 100;        // IDC_COMPRESS_ARCHIVE
    constexpr UINT IdArchiveBrowse = 101;       // IDB_COMPRESS_SET_ARCHIVE
    constexpr UINT IdLevelCombo = 102;          // IDC_COMPRESS_LEVEL
    constexpr UINT IdUpdateModeCombo = 103;     // IDC_COMPRESS_UPDATE_MODE
    constexpr UINT IdFormatCombo = 104;         // IDC_COMPRESS_FORMAT
    constexpr UINT IdVolumeCombo = 105;         // IDC_COMPRESS_VOLUME
    constexpr UINT IdMethodCombo = 106;         // IDC_COMPRESS_METHOD
    constexpr UINT IdDictionaryCombo = 107;     // IDC_COMPRESS_DICTIONARY
    constexpr UINT IdOrderCombo = 108;          // IDC_COMPRESS_ORDER
    constexpr UINT IdSolidCombo = 109;          // IDC_COMPRESS_SOLID
    constexpr UINT IdThreadsCombo = 110;        // IDC_COMPRESS_THREADS
    constexpr UINT IdParamsEdit = 111;          // IDE_COMPRESS_PARAMETERS
    constexpr UINT IdMemUseValue = 113;         // IDT_COMPRESS_MEMORY_VALUE
    constexpr UINT IdMemUseDeValue = 114;       // IDT_COMPRESS_MEMORY_DE_VALUE
    constexpr UINT IdPathModeCombo = 116;       // IDC_COMPRESS_PATH_MODE
    constexpr UINT IdMemUseCombo = 117;         // IDC_COMPRESS_MEM_USE

    constexpr UINT IdPassword1Edit = 120;       // IDE_COMPRESS_PASSWORD1
    constexpr UINT IdPassword2Edit = 121;       // IDE_COMPRESS_PASSWORD2
    constexpr UINT IdEncryptionMethod = 122;    // IDC_COMPRESS_ENCRYPTION_METHOD
    constexpr UINT IdFolderStatic = 130;        // IDT_COMPRESS_ARCHIVE_FOLDER
    constexpr UINT IdOptionsSummary = 141;      // IDT_COMPRESS_OPTIONS
    constexpr UINT IdOptionsButton = 2100;      // IDB_COMPRESS_OPTIONS
    constexpr UINT IdPathModeLabel = 3410;      // IDT_COMPRESS_PATH_MODE
    constexpr UINT IdShowPasswordCheck = 3803;  // IDX_PASSWORD_SHOW
    constexpr UINT IdPassword1Label = 3801;     // IDT_PASSWORD_ENTER
    constexpr UINT IdPassword2Label = 3802;     // IDT_PASSWORD_REENTER

    constexpr UINT IdArchiveLabel = 4001;       // IDT_COMPRESS_ARCHIVE
    constexpr UINT IdUpdateModeLabel = 4002;    // IDT_COMPRESS_UPDATE_MODE
    constexpr UINT IdFormatLabel = 4003;        // IDT_COMPRESS_FORMAT
    constexpr UINT IdLevelLabel = 4004;         // IDT_COMPRESS_LEVEL
    constexpr UINT IdMethodLabel = 4005;        // IDT_COMPRESS_METHOD
    constexpr UINT IdDictionaryLabel = 4006;    // IDT_COMPRESS_DICTIONARY
    constexpr UINT IdOrderLabel = 4007;         // IDT_COMPRESS_ORDER
    constexpr UINT IdSolidLabel = 4008;         // IDT_COMPRESS_SOLID
    constexpr UINT IdThreadsLabel = 4009;       // IDT_COMPRESS_THREADS
    constexpr UINT IdParamsLabel = 4010;        // IDT_COMPRESS_PARAMETERS
    constexpr UINT IdSfxCheck = 4012;           // IDX_COMPRESS_SFX
    constexpr UINT IdSharedCheck = 4013;        // IDX_COMPRESS_SHARED
    constexpr UINT IdEncryptMethodLabel = 4015; // IDT_COMPRESS_ENCRYPTION_METHOD
    constexpr UINT IdEncryptNamesCheck = 4016;  // IDX_COMPRESS_ENCRYPT_FILE_NAMES
    constexpr UINT IdMemUseLabel = 4017;        // IDT_COMPRESS_MEMORY
    constexpr UINT IdMemUseDeLabel = 4018;      // IDT_COMPRESS_MEMORY_DE
    constexpr UINT IdDeleteCheck = 4019;        // IDX_COMPRESS_DEL
    constexpr UINT IdVolumeLabel = 7302;        // IDT_SPLIT_TO_VOLUMES
}

namespace winrt::NanaZip::Modern::implementation
{
    using namespace MirrorUi;

    CompressDialogPage::CompressDialogPage(
        _In_opt_ HWND WindowHandle,
        _In_opt_ const K7_DIALOG_MIRROR_ENGINE* Engine) :
        m_WindowHandle(WindowHandle),
        m_Engine(Engine)
    {
    }

    void CompressDialogPage::InitializeComponent()
    {
        // 纯代码 UI 页面:基类无 XAML InitializeComponent(同 SettingsPage)

        // 窗口标题沿用原对话框标题(已本地化)
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

        // ==================== Archive 分区 ====================
        StackPanel archiveSection = this->BuildSection(
            content, L"ArchiveSection", L"Archive", true);

        this->BuildComboField(
            archiveSection, IdArchiveLabel, IdArchiveCombo, true);

        // 压缩包所在目录(只读展示,带 Tooltip 便于复制长路径)
        m_FolderText = winrt::TextBlock();
        m_FolderText.TextWrapping(
            winrt::Windows::UI::Xaml::TextWrapping::Wrap);
        this->ApplyTextStyle(m_FolderText, L"CaptionTextBlockStyle");
        archiveSection.Children().Append(m_FolderText);

        // 浏览按钮(标题沿用原对话框本地化文本)
        {
            Button browse;
            TextSnapshot caption = MirrorUi::ReadText(
                m_Engine, IdArchiveBrowse);
            browse.Content(winrt::box_value(
                caption.Ok && !caption.Text.empty()
                ? StripAccelerator(caption.Text)
                : winrt::hstring(L"Browse...")));
            browse.Margin(winrt::ThicknessHelper::FromLengths(0, 6, 0, 4));
            browse.Click([this](auto&&, auto&&)
            {
                this->OnNestedModalButtonClick(IdArchiveBrowse);
            });
            archiveSection.Children().Append(browse);
        }

        this->BuildComboField(
            archiveSection, IdFormatLabel, IdFormatCombo, false);
        this->BuildComboField(
            archiveSection, IdUpdateModeLabel, IdUpdateModeCombo, false);
        this->BuildComboField(
            archiveSection, IdPathModeLabel, IdPathModeCombo, false);
        this->BuildCheckField(archiveSection, IdSfxCheck);

        // ==================== Compression 分区 ====================
        StackPanel compressionSection = this->BuildSection(
            content, L"CompressionSection", L"Compression", true);

        this->BuildComboField(
            compressionSection, IdLevelLabel, IdLevelCombo, false);
        this->BuildComboField(
            compressionSection, IdMethodLabel, IdMethodCombo, false);
        this->BuildComboField(
            compressionSection, IdDictionaryLabel, IdDictionaryCombo, true);
        this->BuildComboField(
            compressionSection, IdOrderLabel, IdOrderCombo, true);
        this->BuildComboField(
            compressionSection, IdSolidLabel, IdSolidCombo, true);
        this->BuildComboField(
            compressionSection, IdThreadsLabel, IdThreadsCombo, false);
        this->BuildComboField(
            compressionSection, IdMemUseLabel, IdMemUseCombo, true);

        // 内存占用实时数值两行(压缩/解压)
        m_MemoryText = winrt::TextBlock();
        this->ApplyTextStyle(m_MemoryText, L"CaptionTextBlockStyle");
        compressionSection.Children().Append(m_MemoryText);

        {
            StackPanel deRow;
            deRow.Spacing(8);
            m_MemoryDeLabel = winrt::TextBlock();
            this->ApplyTextStyle(m_MemoryDeLabel, L"CaptionTextBlockStyle");
            deRow.Children().Append(m_MemoryDeLabel);
            m_MemoryDeText = winrt::TextBlock();
            this->ApplyTextStyle(m_MemoryDeText, L"CaptionTextBlockStyle");
            deRow.Children().Append(m_MemoryDeText);
            compressionSection.Children().Append(deRow);
        }

        // ==================== Encryption 分区 ====================
        StackPanel encryptionSection = this->BuildSection(
            content, L"EncryptionSection", L"Encryption", true);

        this->BuildPasswordField(
            encryptionSection, IdPassword1Label, IdPassword1Edit);
        this->BuildPasswordField(
            encryptionSection, IdPassword2Label, IdPassword2Edit);
        this->BuildCheckField(encryptionSection, IdShowPasswordCheck);
        this->BuildCheckField(encryptionSection, IdEncryptNamesCheck);
        this->BuildComboField(
            encryptionSection, IdEncryptMethodLabel, IdEncryptionMethod,
            false);

        // ==================== Advanced 分区(默认折叠) ====================
        StackPanel advancedSection = this->BuildSection(
            content, L"AdvancedSection", L"Advanced", false);

        this->BuildComboField(
            advancedSection, IdVolumeLabel, IdVolumeCombo, true);
        this->BuildTextField(
            advancedSection, IdParamsLabel, IdParamsEdit);
        this->BuildCheckField(advancedSection, IdSharedCheck);
        this->BuildCheckField(advancedSection, IdDeleteCheck);

        // 时间戳/NTFS 选项摘要 + "Options" 按钮(打开原选项子对话框)
        m_OptionsText = winrt::TextBlock();
        m_OptionsText.TextWrapping(
            winrt::Windows::UI::Xaml::TextWrapping::Wrap);
        this->ApplyTextStyle(m_OptionsText, L"CaptionTextBlockStyle");
        advancedSection.Children().Append(m_OptionsText);

        {
            Button options;
            TextSnapshot caption = MirrorUi::ReadText(
                m_Engine, IdOptionsButton);
            options.Content(winrt::box_value(
                caption.Ok && !caption.Text.empty()
                ? StripAccelerator(caption.Text)
                : winrt::hstring(L"Options...")));
            options.Margin(winrt::ThicknessHelper::FromLengths(0, 4, 0, 4));
            options.Click([this](auto&&, auto&&)
            {
                this->OnNestedModalButtonClick(IdOptionsButton);
            });
            advancedSection.Children().Append(options);
        }

        // ==================== 按钮栏 ====================
        StackPanel buttons;
        buttons.Orientation(winrt::Orientation::Horizontal);
        buttons.HorizontalAlignment(winrt::HorizontalAlignment::Right);
        buttons.Spacing(8);
        buttons.Margin(winrt::ThicknessHelper::FromLengths(24, 12, 24, 16));
        winrt::Grid::SetRow(buttons, 1);

        Button cancel;
        cancel.Content(winrt::box_value(winrt::NanaZip::Modern::GetUiString(
            L"CompressDialogPage/CancelButtonText", L"Cancel")));
        cancel.Click([this](auto&&, auto&&)
        {
            this->CancelButtonClick();
        });
        buttons.Children().Append(cancel);

        m_OkButton = winrt::Button();
        m_OkButton.Content(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(
                L"CompressDialogPage/OkButtonText", L"OK")));
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

        // Enter=OK / Esc=Cancel;焦点在可编辑组合框/文本框内时按键
        // 可能被控件处理而不冒泡,因此用键盘加速器兜底 + KeyDown 双保险
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

    void CompressDialogPage::InitializeAccelerators(
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
    CompressDialogPage::BuildSection(
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
    CompressDialogPage::BuildLabel(UINT controlId)
    {
        TextBlock label;
        TextSnapshot text = MirrorUi::ReadText(m_Engine, controlId);
        label.Text(text.Ok
            ? StripAcceleratorAndColon(text.Text)
            : winrt::hstring());
        label.Margin(winrt::ThicknessHelper::FromLengths(0, 8, 0, 2));
        return label;
    }

    void CompressDialogPage::ApplyTextStyle(
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
            // 样式缺失时使用默认样式
        }
    }

    void CompressDialogPage::BuildComboField(
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

    void CompressDialogPage::BuildCheckField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        UINT checkId)
    {
        CheckBox check;
        TextSnapshot caption = MirrorUi::ReadText(m_Engine, checkId);
        check.Content(winrt::box_value(
            caption.Ok ? StripAccelerator(caption.Text) : winrt::hstring()));
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
        winrt::AutomationProperties::SetName(
            check, winrt::hstring(caption.Ok
                ? StripAccelerator(caption.Text)
                : winrt::hstring()));
        parent.Children().Append(check);

        CheckEntry entry;
        entry.Id = checkId;
        entry.Control = check;
        m_Checks.push_back(std::move(entry));
    }

    void CompressDialogPage::BuildTextField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        UINT labelId,
        UINT editId)
    {
        TextBlock label = this->BuildLabel(labelId);
        TextBox edit;
        edit.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        winrt::AutomationProperties::SetName(
            edit, winrt::hstring(label.Text()));
        parent.Children().Append(label);
        parent.Children().Append(edit);

        TextEntry entry;
        entry.Id = editId;
        entry.Control = edit;
        m_Texts.push_back(std::move(entry));
    }

    void CompressDialogPage::BuildPasswordField(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        UINT labelId,
        UINT editId)
    {
        TextBlock label = this->BuildLabel(labelId);

        PasswordBox passwordBox;
        TextBox plainBox;
        plainBox.Visibility(winrt::Visibility::Collapsed);

        // 密码不落日志、不缓存到引擎以外的任何位置;
        // 仅写回隐藏对话框控件(与经典行为一致)
        passwordBox.PasswordChanged(
            [this, editId, plainBox](
                winrt::IInspectable const& sender,
                winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress || plainBox.Visibility() ==
                winrt::Visibility::Visible)
                return;
            PasswordBox source = sender.as<PasswordBox>();
            if (m_Engine && m_Engine->SetText)
            {
                m_Engine->SetText(
                    m_Engine->Context, editId, source.Password().c_str());
            }
        });

        plainBox.TextChanged(
            [this, editId](
                winrt::IInspectable const& sender,
                winrt::RoutedEventArgs const&)
        {
            UNREFERENCED_PARAMETER(sender);
            if (m_Suppress)
                return;
            TextBox source = sender.as<TextBox>();
            if (m_Engine && m_Engine->SetText)
            {
                m_Engine->SetText(
                    m_Engine->Context, editId, source.Text().c_str());
            }
        });

        passwordBox.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        plainBox.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            this->OnFieldKeyDown(e);
        });
        winrt::AutomationProperties::SetName(
            passwordBox, winrt::hstring(label.Text()));
        winrt::AutomationProperties::SetName(
            plainBox, winrt::hstring(label.Text()));

        Grid slot;
        slot.Children().Append(passwordBox);
        slot.Children().Append(plainBox);

        parent.Children().Append(label);
        parent.Children().Append(slot);

        if (editId == IdPassword1Edit)
        {
            m_Password1Label = label;
            m_Password1Box = passwordBox;
            m_Password1Plain = plainBox;
        }
        else
        {
            m_Password2Label = label;
            m_Password2Box = passwordBox;
            m_Password2Plain = plainBox;
        }
    }

    void CompressDialogPage::SyncAll()
    {
        m_Suppress = true;

        this->SyncCombos();
        this->SyncChecks();
        this->SyncTexts();
        this->SyncStaticText(m_FolderText, IdFolderStatic);
        this->SyncStaticText(m_MemoryText, IdMemUseValue);
        this->SyncStaticText(m_MemoryDeLabel, IdMemUseDeLabel);
        this->SyncStaticText(m_MemoryDeText, IdMemUseDeValue);
        this->SyncStaticText(m_OptionsText, IdOptionsSummary);
        this->SyncPasswordSlot(
            IdPassword1Edit, IdShowPasswordCheck,
            m_Password1Label, m_Password1Box, m_Password1Plain);
        this->SyncPasswordSlot(
            IdPassword2Edit, IdShowPasswordCheck,
            m_Password2Label, m_Password2Box, m_Password2Plain);

        m_Suppress = false;
    }

    void CompressDialogPage::SyncCombos()
    {
        for (ComboEntry& entry : m_Combos)
        {
            ComboSnapshot snapshot = MirrorUi::ReadCombo(
                m_Engine, entry.Id);
            if (!snapshot.Ok || !entry.Control)
                continue;

            ComboBox combo = entry.Control;

            // 重建列表(交互频率低、列表短,重建成本可忽略)
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

    void CompressDialogPage::SyncChecks()
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

    void CompressDialogPage::SyncTexts()
    {
        for (TextEntry& entry : m_Texts)
        {
            TextSnapshot snapshot = MirrorUi::ReadText(
                m_Engine, entry.Id);
            if (!snapshot.Ok || !entry.Control)
                continue;
            entry.Control.Text(winrt::hstring(snapshot.Text));
            entry.Control.IsEnabled(snapshot.Enabled);
            entry.Control.Visibility(snapshot.Visible
                ? winrt::Visibility::Visible
                : winrt::Visibility::Collapsed);
        }
    }

    void CompressDialogPage::SyncPasswordSlot(
        UINT editId,
        UINT showCheckId,
        winrt::Windows::UI::Xaml::Controls::TextBlock& label,
        winrt::Windows::UI::Xaml::Controls::PasswordBox& passwordBox,
        winrt::Windows::UI::Xaml::Controls::TextBox& plainBox)
    {
        if (!label || !passwordBox || !plainBox)
            return;

        CheckSnapshot show = MirrorUi::ReadCheck(m_Engine, showCheckId);
        TextSnapshot edit = MirrorUi::ReadText(m_Engine, editId);
        bool showPlain = show.Ok ? show.Checked : false;

        // 首次同步:把引擎侧已有密码(命令行/上次会话带入)填入
        if (!m_PasswordSeeded && edit.Ok)
        {
            m_PasswordSeeded = true;
            passwordBox.Password(winrt::hstring(edit.Text));
            plainBox.Text(winrt::hstring(edit.Text));
        }

        // 显示开关切换时在密文/明文控件间保持文本一致
        if (showPlain)
        {
            if (plainBox.Visibility() == winrt::Visibility::Collapsed)
            {
                plainBox.Text(winrt::hstring(passwordBox.Password()));
            }
            plainBox.Visibility(winrt::Visibility::Visible);
            passwordBox.Visibility(winrt::Visibility::Collapsed);
        }
        else
        {
            if (passwordBox.Visibility() == winrt::Visibility::Collapsed)
            {
                passwordBox.Password(plainBox.Text());
            }
            passwordBox.Visibility(winrt::Visibility::Visible);
            plainBox.Visibility(winrt::Visibility::Collapsed);
        }

        // 复刻 UpdatePasswordControl:显示密码时隐藏第二个输入框;
        // 不支持加密的格式由原逻辑禁用全部密码控件。
        // (TextBlock 非 Control,无 IsEnabled,用可见性表达禁用态)
        bool enabled = edit.Ok ? edit.Enabled : true;
        bool visible = (edit.Ok ? edit.Visible : true) && enabled;
        label.Visibility(visible
            ? winrt::Visibility::Visible
            : winrt::Visibility::Collapsed);
        passwordBox.IsEnabled(enabled);
        plainBox.IsEnabled(enabled);
        passwordBox.Visibility(visible
            ? passwordBox.Visibility()
            : winrt::Visibility::Collapsed);
        plainBox.Visibility(visible
            ? plainBox.Visibility()
            : winrt::Visibility::Collapsed);
    }

    void CompressDialogPage::SyncStaticText(
        winrt::Windows::UI::Xaml::Controls::TextBlock& target,
        UINT sourceId)
    {
        if (!target)
            return;
        TextSnapshot snapshot = MirrorUi::ReadText(m_Engine, sourceId);
        target.Text(snapshot.Ok
            ? StripAccelerator(snapshot.Text)
            : winrt::hstring());
        // 空文本折叠,避免占位空白
        target.Visibility(snapshot.Ok && !snapshot.Text.empty()
            ? winrt::Visibility::Visible
            : winrt::Visibility::Collapsed);
    }

    void CompressDialogPage::PushEditableTexts()
    {
        if (!m_Engine || !m_Engine->SetComboText)
            return;

        for (ComboEntry const& entry : m_Combos)
        {
            if (!entry.Editable || !entry.Control)
                continue;
            m_Engine->SetComboText(
                m_Engine->Context,
                entry.Id,
                entry.Control.Text().c_str());
        }
    }

    void CompressDialogPage::OnMirrorComboChanged(
        UINT comboId, int selection)
    {
        if (!m_Engine || !m_Engine->SetComboSelection)
            return;
        this->PushEditableTexts();
        m_Engine->SetComboSelection(m_Engine->Context, comboId, selection);
        this->SyncAll();
    }

    void CompressDialogPage::OnMirrorCheckClicked(
        UINT checkId, bool checked_)
    {
        if (!m_Engine || !m_Engine->SetCheck)
            return;
        this->PushEditableTexts();
        m_Engine->SetCheck(
            m_Engine->Context, checkId, checked_ ? TRUE : FALSE);
        this->SyncAll();
    }

    void CompressDialogPage::OnNestedModalButtonClick(UINT buttonId)
    {
        if (!m_Engine || !m_Engine->NotifyButtonClick)
            return;
        this->PushEditableTexts();
        // 浏览/选项子对话框期间禁用本窗口,保持模态语义
        if (m_WindowHandle)
            ::EnableWindow(m_WindowHandle, FALSE);
        m_Engine->NotifyButtonClick(m_Engine->Context, buttonId);
        if (m_WindowHandle)
        {
            ::EnableWindow(m_WindowHandle, TRUE);
            ::SetForegroundWindow(m_WindowHandle);
        }
        this->SyncAll();
    }

    void CompressDialogPage::OkButtonClick()
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
            // 原逻辑已弹出错误提示;重新同步(部分状态可能已变化)
            this->SyncAll();
        }
    }

    void CompressDialogPage::CancelButtonClick()
    {
        this->Finish(K7_DIALOG_MIRROR_RESULT_CANCEL);
    }

    void CompressDialogPage::Finish(UINT resultCode)
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

    void CompressDialogPage::OnFieldKeyDown(
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

    void CompressDialogPage::OnRootKeyDown(
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

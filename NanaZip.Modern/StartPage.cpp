#include "pch.h"
#include "StartPage.h"
#if __has_include("StartPage.g.cpp")
#include "StartPage.g.cpp"
#endif

#include "UiStrings.h"

#include <shlobj.h>

#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <cstring>

namespace winrt
{
    using Windows::UI::Xaml::Automation::AutomationProperties;
    using Windows::UI::Xaml::Controls::Button;
    using Windows::UI::Xaml::Controls::ColumnDefinition;
    using Windows::UI::Xaml::Controls::FontIcon;
    using Windows::UI::Xaml::Controls::Grid;
    using Windows::UI::Xaml::Controls::ListView;
    using Windows::UI::Xaml::Controls::ListViewItem;
    using Windows::UI::Xaml::Controls::RowDefinition;
    using Windows::UI::Xaml::Controls::ScrollViewer;
    using Windows::UI::Xaml::Controls::StackPanel;
    using Windows::UI::Xaml::Controls::TextBlock;
    using Windows::UI::Xaml::GridLengthHelper;
    using Windows::UI::Xaml::GridUnitType;
    using Windows::UI::Xaml::HorizontalAlignment;
    using Windows::UI::Xaml::Media::FontFamily;
    using Windows::UI::Xaml::ThicknessHelper;
    using Windows::UI::Xaml::VerticalAlignment;
    using Windows::UI::Xaml::Visibility;
}

namespace
{
    // 常见压缩包扩展名(拖放"直接打开"的判断;与 FM 侧工具栏
    // SelectionAreArchives 的常用子集保持一致)。
    wchar_t const* const kArchiveExtensions[] =
    {
        L".7z",  L".zip",  L".rar",  L".tar",  L".gz",   L".tgz",
        L".bz2", L".tbz2", L".xz",   L".txz",  L".zst",  L".tzst",
        L".lz4", L".lz",   L".lzma", L".lzh",  L".arj",
        L".cab", L".iso",  L".wim",  L".esd",  L".swm",  L".001",
        L".dmg", L".xar",  L".cpio", L".rpm",  L".deb",
    };

    bool HasArchiveExtension(std::wstring const& path)
    {
        std::size_t dot = path.find_last_of(L'.');
        if (dot == std::wstring::npos)
        {
            return false;
        }
        std::wstring extension = path.substr(dot);
        for (wchar_t const* known : kArchiveExtensions)
        {
            if (_wcsicmp(extension.c_str(), known) == 0)
            {
                return true;
            }
        }
        return false;
    }

    // 从完整路径取文件名(含扩展名)。
    std::wstring GetFileName(std::wstring const& path)
    {
        std::size_t slash = path.find_last_of(L"\\/");
        return (slash == std::wstring::npos)
            ? path
            : path.substr(slash + 1);
    }

    // 压缩包文件选择过滤器(IFileOpenDialog)。
    COMDLG_FILTERSPEC const kArchiveFilters[] =
    {
        { L"Archives", L"*.7z;*.zip;*.rar;*.tar;*.gz;*.tgz;*.bz2;*.tbz2;"
            L"*.xz;*.txz;*.zst;*.tzst;*.lz4;*.lz;*.lzma;*.lzh;*.arj;"
            L"*.cab;*.iso;*.wim;*.esd;*.swm;*.001;*.dmg;*.xar;*.cpio;"
            L"*.rpm;*.deb" },
        { L"All files", L"*.*" },
    };
}

namespace winrt::NanaZip::Modern::implementation
{
    // ==================== 拖放目标 ====================

    // 拖入单个压缩包 → 直接打开;拖入普通文件/多文件/文件夹 →
    // 进入"创建压缩包"待确认模式。只接受 CF_HDROP 文件列表。
    struct StartPage::DropTarget :
        winrt::implements<StartPage::DropTarget, IDropTarget>
    {
    public:

        explicit DropTarget(StartPage* page) : m_Page(page)
        {
        }

        STDMETHODIMP DragEnter(
            IDataObject* dataObject,
            DWORD keyState,
            POINTL point,
            DWORD* effect) override
        {
            UNREFERENCED_PARAMETER(keyState);
            UNREFERENCED_PARAMETER(point);
            *effect = DROPEFFECT_NONE;
            if (dataObject && this->HasFileList(dataObject))
            {
                *effect = DROPEFFECT_COPY;
            }
            return S_OK;
        }

        STDMETHODIMP DragOver(
            DWORD keyState,
            POINTL point,
            DWORD* effect) override
        {
            UNREFERENCED_PARAMETER(keyState);
            UNREFERENCED_PARAMETER(point);
            *effect = DROPEFFECT_COPY;
            return S_OK;
        }

        STDMETHODIMP DragLeave() override
        {
            return S_OK;
        }

        STDMETHODIMP Drop(
            IDataObject* dataObject,
            DWORD keyState,
            POINTL point,
            DWORD* effect) override
        {
            UNREFERENCED_PARAMETER(keyState);
            UNREFERENCED_PARAMETER(point);
            *effect = DROPEFFECT_NONE;
            if (!dataObject || !m_Page)
            {
                return S_OK;
            }

            FORMATETC format = {};
            format.cfFormat = CF_HDROP;
            format.dwAspect = DVASPECT_CONTENT;
            format.lindex = -1;
            format.tymed = TYMED_HGLOBAL;

            STGMEDIUM medium = {};
            if (FAILED(dataObject->GetData(&format, &medium))
                || !medium.hGlobal)
            {
                return S_OK;
            }

            std::vector<std::wstring> paths;
            HDROP drop = static_cast<HDROP>(medium.hGlobal);
            UINT count = ::DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < count; ++i)
            {
                UINT length = ::DragQueryFileW(drop, i, nullptr, 0);
                std::wstring buffer(length + 1, L'\0');
                ::DragQueryFileW(
                    drop, i, buffer.data(), length + 1);
                buffer.resize(length);
                paths.push_back(std::move(buffer));
            }

            ::ReleaseStgMedium(&medium);

            m_Page->HandleDroppedFiles(paths);
            *effect = DROPEFFECT_COPY;
            return S_OK;
        }

    private:

        bool HasFileList(IDataObject* dataObject)
        {
            FORMATETC format = {};
            format.cfFormat = CF_HDROP;
            format.dwAspect = DVASPECT_CONTENT;
            format.lindex = -1;
            format.tymed = TYMED_HGLOBAL;
            return dataObject->QueryGetData(&format) == S_OK;
        }

        StartPage* m_Page;
    };

    // ==================== 页面 ====================

    StartPage::StartPage(
        _In_opt_ HWND WindowHandle,
        _In_opt_ K7_START_GET_RECENT_CALLBACK GetRecent,
        _In_opt_ K7_MODERN_START_RESULT* Result) :
        m_WindowHandle(WindowHandle),
        m_GetRecent(GetRecent),
        m_Result(Result)
    {
        this->InitializeComponent();
    }

    StartPage::~StartPage()
    {
        // com_ptr<DropTarget> 在此释放(DropTarget 完整类型可见)
    }

    void StartPage::InitializeComponent()
    {
        // 纯代码 UI 页面:基类无 XAML InitializeComponent(同 SettingsPage)

        if (m_WindowHandle)
        {
            ::SetWindowTextW(m_WindowHandle, L"CuinZip");
        }

        // 最近列表经宿主回调读取(真实数据源在 FM 注册表)。
        // P1-6.1:显示前再做一次存在性检查(双保险,宿主清理与
        // 显示之间被删除的文件不再出现在列表中)。
        if (m_GetRecent)
        {
            K7_MODERN_START_RECENT_LIST recent = {};
            m_GetRecent(&recent);
            for (UINT32 i = 0; i < recent.Count && i < K7_START_RECENT_MAX;
                ++i)
            {
                if (recent.Paths[i][0] != L'\0'
                    && INVALID_FILE_ATTRIBUTES
                        != ::GetFileAttributesW(recent.Paths[i]))
                {
                    m_RecentPaths.push_back(std::wstring(recent.Paths[i]));
                }
            }
        }

        m_Root = winrt::Grid();
        this->BuildHomePanel();
        this->BuildCreatePanel();

        // 创建模式默认隐藏
        m_CreatePanel.Visibility(winrt::Visibility::Collapsed);

        m_Root.Children().Append(m_HomePanel);
        m_Root.Children().Append(m_CreatePanel);

        // P1-6.1:Esc = 关闭 Home 按默认行为进入文件管理器;
        // Enter 不绑定(避免误触发卡片按钮)。
        m_Root.KeyDown([this](
            winrt::IInspectable const&,
            winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& e)
        {
            if (e.Key() == winrt::Windows::System::VirtualKey::Escape)
            {
                e.Handled(true);
                this->Finish(K7_START_ACTION_NONE, {});
            }
        });

        this->Content(m_Root);

        this->RegisterDropTarget();
    }

    winrt::Windows::UI::Xaml::Controls::TextBlock StartPage::MakeText(
        std::wstring_view const& key,
        std::wstring_view const& fallback,
        double fontSize,
        bool semiBold,
        bool secondary)
    {
        winrt::TextBlock text;
        text.Text(winrt::NanaZip::Modern::GetUiString(key, fallback));
        if (fontSize > 0)
        {
            text.FontSize(fontSize);
        }
        if (semiBold)
        {
            text.FontWeight(
                winrt::Windows::UI::Text::FontWeights::SemiBold());
        }
        text.TextWrapping(winrt::Windows::UI::Xaml::TextWrapping::Wrap);
        if (secondary)
        {
            auto brush = winrt::Windows::UI::Xaml::Application::Current()
                .Resources().TryLookup(
                    winrt::box_value(
                        winrt::hstring(L"TextFillColorSecondaryBrush")))
                .try_as<winrt::Windows::UI::Xaml::Media::Brush>();
            if (brush)
            {
                text.Foreground(brush);
            }
        }
        return text;
    }

    void StartPage::BuildActionCard(
        winrt::Windows::UI::Xaml::Controls::Panel const& parent,
        int row,
        int column,
        wchar_t const* glyph,
        std::wstring_view const& titleKey,
        std::wstring_view const& titleFallback,
        std::wstring_view const& descriptionKey,
        std::wstring_view const& descriptionFallback,
        winrt::Windows::UI::Xaml::RoutedEventHandler const& click)
    {
        // 整卡可点 + 键盘可达(Button 承载,内容左上对齐)
        winrt::Button button;
        button.HorizontalAlignment(winrt::HorizontalAlignment::Stretch);
        button.VerticalAlignment(winrt::VerticalAlignment::Stretch);
        button.HorizontalContentAlignment(
            winrt::HorizontalAlignment::Left);
        button.VerticalContentAlignment(winrt::VerticalAlignment::Top);
        button.Padding(winrt::ThicknessHelper::FromUniformLength(0));
        button.UseSystemFocusVisuals(true);
        button.Click(click);

        winrt::StackPanel content;
        content.Spacing(8);

        winrt::FontIcon icon;
        icon.FontFamily(winrt::FontFamily(
            L"Segoe Fluent Icons,Segoe MDL2 Assets"));
        icon.FontSize(24.0);
        icon.Glyph(glyph);
        content.Children().Append(icon);

        winrt::TextBlock title = this->MakeText(
            titleKey, titleFallback, 16, true, false);
        content.Children().Append(title);
        winrt::AutomationProperties::SetName(button, title.Text());

        winrt::TextBlock description = this->MakeText(
            descriptionKey, descriptionFallback, 12, false, true);
        content.Children().Append(description);

        button.Content(content);

        winrt::Grid::SetRow(button, row);
        winrt::Grid::SetColumn(button, column);
        parent.Children().Append(button);
    }

    void StartPage::BuildHomePanel()
    {
        m_HomePanel = winrt::Grid();
        m_HomePanel.Margin(winrt::ThicknessHelper::FromLengths(24, 20, 24, 20));

        {
            winrt::RowDefinition headerRow;
            headerRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            m_HomePanel.RowDefinitions().Append(headerRow);
            winrt::RowDefinition actionsRow;
            actionsRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            m_HomePanel.RowDefinitions().Append(actionsRow);
            winrt::RowDefinition recentRow;
            recentRow.Height(winrt::GridLengthHelper::FromValueAndType(
                1, winrt::GridUnitType::Star));
            m_HomePanel.RowDefinitions().Append(recentRow);
        }

        // ---- 欢迎区(无 Recent 时即为首次启动的简洁空状态) ----
        winrt::StackPanel header;
        header.Spacing(4);
        winrt::Grid::SetRow(header, 0);
        header.Children().Append(this->MakeText(
            L"StartPage/WelcomeTitle.Text",
            L"Welcome to CuinZip",
            28, true, false));
        header.Children().Append(this->MakeText(
            L"StartPage/WelcomeSubtitle.Text",
            L"Open an archive or drag files here to get started.",
            14, false, true));
        m_HomePanel.Children().Append(header);

        // ---- 四个操作卡(2 x 2) ----
        winrt::Grid actions;
        winrt::Grid::SetRow(actions, 1);
        actions.Margin(winrt::ThicknessHelper::FromLengths(0, 16, 0, 0));
        for (int i = 0; i < 2; ++i)
        {
            winrt::ColumnDefinition half;
            half.Width(winrt::GridLengthHelper::FromValueAndType(
                1, winrt::GridUnitType::Star));
            actions.ColumnDefinitions().Append(half);
        }
        for (int i = 0; i < 2; ++i)
        {
            winrt::RowDefinition cardRow;
            cardRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            actions.RowDefinitions().Append(cardRow);
        }

        this->BuildActionCard(
            actions, 0, 0, L"\uE8E5",
            L"StartPage/ActionOpenArchiveTitle.Text", L"Open Archive",
            L"StartPage/ActionOpenArchiveDescription.Text",
            L"Browse and open an existing archive.",
            { this, &StartPage::OpenArchiveCardClick });
        this->BuildActionCard(
            actions, 0, 1, L"\uE8DE",
            L"StartPage/ActionExtractTitle.Text", L"Extract Archive",
            L"StartPage/ActionExtractDescription.Text",
            L"Extract files from an archive to a folder.",
            { this, &StartPage::ExtractCardClick });
        this->BuildActionCard(
            actions, 1, 0, L"\uE8C8",
            L"StartPage/ActionCreateTitle.Text", L"Create Archive",
            L"StartPage/ActionCreateDescription.Text",
            L"Compress files into a new archive.",
            { this, &StartPage::CreateCardClick });
        this->BuildActionCard(
            actions, 1, 1, L"\uE838",
            L"StartPage/ActionOpenFolderTitle.Text", L"Open Folder",
            L"StartPage/ActionOpenFolderDescription.Text",
            L"Browse files and folders in the file manager.",
            { this, &StartPage::OpenFolderCardClick });

        m_HomePanel.Children().Append(actions);

        // ---- Recent Archives(无记录时保持简洁,整区隐藏) ----
        if (!m_RecentPaths.empty())
        {
            m_RecentSection = winrt::StackPanel();
            m_RecentSection.Spacing(8);
            winrt::Grid::SetRow(m_RecentSection, 2);
            m_RecentSection.Margin(
                winrt::ThicknessHelper::FromLengths(0, 20, 0, 0));
            m_RecentSection.Children().Append(this->MakeText(
                L"StartPage/RecentHeader.Text",
                L"Recent archives",
                16, true, false));

            m_RecentList = winrt::ListView();
            m_RecentList.SelectionMode(
                winrt::Windows::UI::Xaml::Controls::
                    ListViewSelectionMode::Single);
            m_RecentList.IsItemClickEnabled(true);
            m_RecentList.ItemClick({ this, &StartPage::RecentItemClick });

            for (std::wstring const& path : m_RecentPaths)
            {
                winrt::StackPanel row;
                row.Spacing(2);
                row.Margin(winrt::ThicknessHelper::FromUniformLength(4));

                winrt::TextBlock name;
                name.Text(winrt::hstring(GetFileName(path)));
                name.FontWeight(
                    winrt::Windows::UI::Text::FontWeights::SemiBold());
                row.Children().Append(name);

                winrt::TextBlock location;
                location.Text(winrt::hstring(path));
                location.FontSize(12.0);
                location.TextWrapping(
                    winrt::Windows::UI::Xaml::TextWrapping::Wrap);
                row.Children().Append(location);

                winrt::ListViewItem item;
                item.Content(row);
                winrt::AutomationProperties::SetName(item,
                    winrt::hstring(GetFileName(path)));
                m_RecentList.Items().Append(item);
            }

            m_RecentSection.Children().Append(m_RecentList);
            m_HomePanel.Children().Append(m_RecentSection);
        }
    }

    void StartPage::BuildCreatePanel()
    {
        // 拖入普通文件后的"创建压缩包"待确认面板
        m_CreatePanel = winrt::Grid();
        m_CreatePanel.Margin(
            winrt::ThicknessHelper::FromLengths(24, 20, 24, 20));

        {
            winrt::RowDefinition contentRow;
            contentRow.Height(winrt::GridLengthHelper::FromValueAndType(
                1, winrt::GridUnitType::Star));
            m_CreatePanel.RowDefinitions().Append(contentRow);
            winrt::RowDefinition buttonRow;
            buttonRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            m_CreatePanel.RowDefinitions().Append(buttonRow);
        }

        winrt::StackPanel content;
        content.Spacing(12);
        winrt::Grid::SetRow(content, 0);

        content.Children().Append(this->MakeText(
            L"StartPage/CreateTitle.Text",
            L"Create Archive",
            24, true, false));

        m_CreateFilesText = this->MakeText(
            L"StartPage/CreateFilesText.Text",
            L"{0} files selected",
            14, false, true);
        content.Children().Append(m_CreateFilesText);

        winrt::TextBlock listHeader = this->MakeText(
            L"StartPage/CreateListHeader.Text",
            L"Files to compress",
            14, true, false);
        content.Children().Append(listHeader);

        winrt::ListView list;
        list.SelectionMode(
            winrt::Windows::UI::Xaml::Controls::ListViewSelectionMode::None);
        m_CreateFilesList = list;
        content.Children().Append(list);

        winrt::StackPanel buttons;
        buttons.Orientation(
            winrt::Windows::UI::Xaml::Controls::Orientation::Horizontal);
        buttons.Spacing(8);
        buttons.HorizontalAlignment(winrt::HorizontalAlignment::Right);
        buttons.Margin(winrt::ThicknessHelper::FromLengths(0, 16, 0, 0));
        winrt::Grid::SetRow(buttons, 1);

        winrt::Button cancel;
        cancel.Content(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(
                L"StartPage/CreateCancelText.Text", L"Cancel")));
        cancel.Click({ this, &StartPage::CreateCancelClick });
        buttons.Children().Append(cancel);

        winrt::Button confirm;
        confirm.Content(winrt::box_value(
            winrt::NanaZip::Modern::GetUiString(
                L"StartPage/CreateConfirmText.Text", L"Create Archive")));
        try
        {
            confirm.Style(
                winrt::Windows::UI::Xaml::Application::Current().Resources()
                .Lookup(winrt::box_value(L"AccentButtonStyle"))
                .as<winrt::Windows::UI::Xaml::Style>());
        }
        catch (...)
        {
        }
        confirm.Click({ this, &StartPage::CreateConfirmClick });
        buttons.Children().Append(confirm);

        m_CreatePanel.Children().Append(content);

        m_CreatePanel.Children().Append(buttons);
    }

    bool StartPage::PickPaths(
        bool pickFolders,
        bool multiSelect,
        std::vector<std::wstring>& paths)
    {
        paths.clear();

        winrt::com_ptr<IFileOpenDialog> dialog =
            winrt::try_create_instance<IFileOpenDialog>(
                CLSID_FileOpenDialog,
                CLSCTX_INPROC_SERVER);
        if (!dialog)
        {
            return false;
        }

        DWORD options = 0;
        dialog->GetOptions(&options);
        options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
        if (pickFolders)
        {
            options |= FOS_PICKFOLDERS;
        }
        else
        {
            options |= FOS_FILEMUSTEXIST;
            if (multiSelect)
            {
                options |= FOS_ALLOWMULTISELECT;
            }
            dialog->SetFileTypes(
                ARRAYSIZE(kArchiveFilters), kArchiveFilters);
        }
        dialog->SetOptions(options);

        HWND owner = m_WindowHandle ? m_WindowHandle : ::GetDesktopWindow();
        if (FAILED(dialog->Show(owner)))
        {
            // 用户取消或失败:静默返回
            return false;
        }

        winrt::com_ptr<IShellItemArray> items;
        if (FAILED(dialog->GetResults(items.put())) || !items)
        {
            return false;
        }

        DWORD count = 0;
        items->GetCount(&count);
        for (DWORD i = 0; i < count; ++i)
        {
            winrt::com_ptr<IShellItem> item;
            if (FAILED(items->GetItemAt(i, item.put())) || !item)
            {
                continue;
            }
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(
                SIGDN_FILESYSPATH, &path)) && path)
            {
                paths.push_back(std::wstring(path));
                ::CoTaskMemFree(path);
            }
        }

        return !paths.empty();
    }

    void StartPage::OpenArchiveCardClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        std::vector<std::wstring> paths;
        if (this->PickPaths(false, false, paths))
        {
            this->Finish(K7_START_ACTION_OPEN_ARCHIVE, paths);
        }
    }

    void StartPage::ExtractCardClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        std::vector<std::wstring> paths;
        if (this->PickPaths(false, false, paths))
        {
            this->Finish(K7_START_ACTION_EXTRACT, paths);
        }
    }

    void StartPage::CreateCardClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        std::vector<std::wstring> paths;
        if (this->PickPaths(false, true, paths))
        {
            this->EnterCreateList(paths);
        }
    }

    void StartPage::OpenFolderCardClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        std::vector<std::wstring> paths;
        if (this->PickPaths(true, false, paths))
        {
            this->Finish(K7_START_ACTION_OPEN_FOLDER, paths);
        }
    }

    void StartPage::RecentItemClick(
        winrt::IInspectable const& sender,
        winrt::Windows::UI::Xaml::Controls::ItemClickEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        int index = m_RecentList.SelectedIndex();
        if (index < 0
            || static_cast<std::size_t>(index) >= m_RecentPaths.size())
        {
            return;
        }

        std::vector<std::wstring> paths;
        paths.push_back(m_RecentPaths[static_cast<std::size_t>(index)]);
        this->Finish(K7_START_ACTION_OPEN_ARCHIVE, paths);
    }

    void StartPage::CreateConfirmClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        if (m_PendingFiles.empty())
        {
            return;
        }
        this->Finish(K7_START_ACTION_CREATE, m_PendingFiles);
    }

    void StartPage::CreateCancelClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        m_PendingFiles.clear();
        m_CreatePanel.Visibility(winrt::Visibility::Collapsed);
        m_HomePanel.Visibility(winrt::Visibility::Visible);
    }

    void StartPage::HandleDroppedFiles(
        std::vector<std::wstring> const& paths)
    {
        if (paths.empty())
        {
            return;
        }

        // 单个压缩包:直接打开,立刻进入文件管理器。
        if (paths.size() == 1 && HasArchiveExtension(paths[0]))
        {
            this->Finish(K7_START_ACTION_OPEN_ARCHIVE, paths);
            return;
        }

        // 普通文件 / 多文件 / 文件夹:进入创建压缩包待确认模式。
        this->EnterCreateList(paths);
    }

    void StartPage::EnterCreateList(std::vector<std::wstring> const& paths)
    {
        m_PendingFiles = paths;

        // 汇总文案:"{0} files selected"
        std::wstring summary(winrt::NanaZip::Modern::GetUiString(
            L"StartPage/CreateFilesText.Text", L"{0} files selected"));
        std::wstring countText = std::to_wstring(paths.size());
        std::size_t position = summary.find(L"{0}");
        if (position != std::wstring::npos)
        {
            summary.replace(position, 3, countText);
        }
        else
        {
            summary += L"  " + countText;
        }
        m_CreateFilesText.Text(winrt::hstring(summary));

        // 填充文件列表(每行一个文件名 + 完整路径小字)
        if (m_CreateFilesList)
        {
            m_CreateFilesList.Items().Clear();
            for (std::wstring const& path : paths)
            {
                winrt::StackPanel row;
                row.Spacing(2);
                row.Margin(winrt::ThicknessHelper::FromUniformLength(4));

                winrt::TextBlock name;
                name.Text(winrt::hstring(GetFileName(path)));
                name.FontWeight(
                    winrt::Windows::UI::Text::FontWeights::SemiBold());
                row.Children().Append(name);

                winrt::TextBlock location;
                location.Text(winrt::hstring(path));
                location.FontSize(12.0);
                location.TextWrapping(
                    winrt::Windows::UI::Xaml::TextWrapping::Wrap);
                row.Children().Append(location);

                winrt::ListViewItem item;
                item.Content(row);
                m_CreateFilesList.Items().Append(item);
            }
        }

        m_HomePanel.Visibility(winrt::Visibility::Collapsed);
        m_CreatePanel.Visibility(winrt::Visibility::Visible);
    }

    void StartPage::Finish(
        INT32 action,
        std::vector<std::wstring> const& paths)
    {
        if (m_Result)
        {
            m_Result->Action = action;
            m_Result->PathCount = 0;
            if (m_Result->PathBuffer && m_Result->PathBufferCapacity > 0)
            {
                wchar_t* cursor = m_Result->PathBuffer;
                wchar_t* end = cursor + m_Result->PathBufferCapacity;
                for (std::wstring const& path : paths)
                {
                    // 每条路径 + 结尾 NUL;最后保留一个 NUL 作双结尾。
                    if (cursor + path.size() + 1 >= end)
                    {
                        break;
                    }
                    std::wmemcpy(cursor, path.c_str(), path.size() + 1);
                    cursor += path.size() + 1;
                    ++m_Result->PathCount;
                }
                *cursor = L'\0';
            }
        }

        if (m_WindowHandle)
        {
            ::PostMessageW(m_WindowHandle, WM_CLOSE, 0, 0);
        }
    }

    void StartPage::RegisterDropTarget()
    {
        if (!m_WindowHandle)
        {
            return;
        }

        m_DropTarget = winrt::make_self<DropTarget>(this);
        if (FAILED(::RegisterDragDrop(
            m_WindowHandle,
            static_cast<IDropTarget*>(m_DropTarget.get()))))
        {
            m_DropTarget = nullptr;
        }
    }
}

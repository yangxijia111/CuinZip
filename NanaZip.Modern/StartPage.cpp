#include "pch.h"
#include "StartPage.h"
#if __has_include("StartPage.g.cpp")
#include "StartPage.g.cpp"
#endif

#include "UiStrings.h"
#include "StartPagePaths.h"

#include <shlobj.h>

#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <algorithm>
#include <cstring>

namespace winrt
{
    using Windows::UI::Xaml::Automation::AutomationProperties;
    using Windows::UI::Xaml::Controls::Border;
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
    using Windows::UI::Xaml::CornerRadiusHelper;
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
        DWORD attributes = ::GetFileAttributesW(path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES
            || (attributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;
        }
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

    // 文件大小人性化显示(B/KB/MB/GB/TB,一位小数)。
    std::wstring FormatFileSize(unsigned long long bytes)
    {
        wchar_t const* const units[] =
            { L"B", L"KB", L"MB", L"GB", L"TB" };
        double value = static_cast<double>(bytes);
        int unit = 0;
        while (value >= 1024.0 && unit < 4)
        {
            value /= 1024.0;
            ++unit;
        }
        wchar_t buffer[64] = {};
        if (unit == 0)
        {
            swprintf_s(buffer, L"%.0f %s", value, units[unit]);
        }
        else
        {
            swprintf_s(buffer, L"%.1f %s", value, units[unit]);
        }
        return buffer;
    }

    // 文件修改时间按用户区域设置显示(短日期 + 时:分)。
    std::wstring FormatFileTime(FILETIME const& fileTime)
    {
        SYSTEMTIME utc = {};
        SYSTEMTIME local = {};
        if (!::FileTimeToSystemTime(&fileTime, &utc)
            || !::SystemTimeToTzSpecificLocalTime(
                nullptr, &utc, &local))
        {
            return {};
        }

        wchar_t buffer[160] = {};
        int written = ::GetDateFormatW(
            LOCALE_USER_DEFAULT, DATE_SHORTDATE,
            &local, nullptr, buffer, 80);
        // GetDateFormatW 返回值含结尾 NUL;拼一个空格再接时间
        std::size_t offset = (written > 0 && written < 80)
            ? static_cast<std::size_t>(written - 1)
            : 0;
        buffer[offset] = L' ';
        ::GetTimeFormatW(
            LOCALE_USER_DEFAULT, TIME_NOSECONDS,
            &local, nullptr, buffer + offset + 1, 80);
        return buffer;
    }

    // Recent 条目第二行:路径 + 大小 + 修改时间(任一缺失自动跳过)。
    std::wstring MakeRecentMetaText(std::wstring const& path)
    {
        WIN32_FILE_ATTRIBUTE_DATA info = {};
        if (!::GetFileAttributesExW(
                path.c_str(), GetFileExInfoStandard, &info))
        {
            return path;
        }

        unsigned long long size =
            (static_cast<unsigned long long>(info.nFileSizeHigh) << 32)
            | info.nFileSizeLow;
        std::wstring meta = FormatFileSize(size);
        std::wstring modified = FormatFileTime(info.ftLastWriteTime);
        if (!modified.empty())
        {
            meta += L"  \u00B7  " + modified;
        }

        if (meta.empty())
        {
            return path;
        }
        return path + L"  \u00B7  " + meta;
    }

    // Pattern only; filter labels are localized when the picker is opened.
    wchar_t const* const kArchivePattern =
        L"*.7z;*.zip;*.rar;*.tar;*.gz;*.tgz;*.bz2;*.tbz2;"
            L"*.xz;*.txz;*.zst;*.tzst;*.lz4;*.lz;*.lzma;*.lzh;*.arj;"
            L"*.cab;*.iso;*.wim;*.esd;*.swm;*.001;*.dmg;*.xar;*.cpio;"
            L"*.rpm;*.deb";
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
            m_AcceptsFiles = dataObject && this->HasFileList(dataObject);
            if (m_AcceptsFiles)
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
            *effect = m_AcceptsFiles ? DROPEFFECT_COPY : DROPEFFECT_NONE;
            return S_OK;
        }

        STDMETHODIMP DragLeave() override
        {
            m_AcceptsFiles = false;
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
        bool m_AcceptsFiles = false;
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
        if (m_DropRetryTimer)
        {
            m_DropRetryTimer.Stop();
            m_DropRetryTimer = nullptr;
        }
        // 子窗口随顶层窗口销毁时由系统自动撤销注册;此处显式清理
        // 记录的句柄,避免页面先于窗口销毁的边界情况
        for (HWND child : m_DropChildWindows)
        {
            if (::IsWindow(child))
            {
                ::RevokeDragDrop(child);
            }
        }
        if (m_DropTarget && m_WindowHandle && ::IsWindow(m_WindowHandle))
        {
            ::RevokeDragDrop(m_WindowHandle);
        }
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
                DWORD attributes = ::GetFileAttributesW(recent.Paths[i]);
                if (recent.Paths[i][0] != L'\0'
                    && attributes != INVALID_FILE_ATTRIBUTES
                    && !(attributes & FILE_ATTRIBUTE_DIRECTORY))
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
                if (m_CreatePanel.Visibility() == winrt::Visibility::Visible)
                {
                    this->CreateCancelClick(nullptr, nullptr);
                }
                else
                {
                    this->Finish(K7_START_ACTION_NONE, {});
                }
            }
        });

        this->Content(m_Root);

        // P1-8:右上角语言切换按钮(zh-Hans <-> en-US 一步切换),
        // 叠在两个面板之上,位置不随面板切换变化
        m_LanguageButton = winrt::Button();
        m_LanguageButton.Margin(
            winrt::ThicknessHelper::FromLengths(0, 8, 12, 0));
        m_LanguageButton.HorizontalAlignment(
            winrt::HorizontalAlignment::Right);
        m_LanguageButton.VerticalAlignment(winrt::VerticalAlignment::Top);
        m_LanguageButton.MinWidth(72.0);
        m_LanguageButton.Click({ this, &StartPage::ToggleLanguageClick });
        this->UpdateLanguageButtonCaption();
        m_Root.Children().Append(m_LanguageButton);

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
            if (auto brush = this->GetSecondaryBrush())
            {
                text.Foreground(brush);
            }
        }

        // P1-8:登记到 resw 文本注册表,语言切换时统一重取
        LocalizedTextEntry entry;
        entry.Control = text;
        entry.Key = std::wstring(key);
        entry.Fallback = std::wstring(fallback);
        m_LocalizedTexts.push_back(std::move(entry));

        return text;
    }

    winrt::Windows::UI::Xaml::Media::Brush StartPage::GetSecondaryBrush()
    {
        try
        {
            return winrt::Windows::UI::Xaml::Application::Current()
                .Resources().TryLookup(
                    winrt::box_value(
                        winrt::hstring(L"TextFillColorSecondaryBrush")))
                .try_as<winrt::Windows::UI::Xaml::Media::Brush>();
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void StartPage::RefreshTexts()
    {
        // 语言切换后:重取全部登记文本;动态文本(选中计数)单独重建
        for (LocalizedTextEntry& entry : m_LocalizedTexts)
        {
            if (entry.Control)
            {
                entry.Control.Text(winrt::NanaZip::Modern::GetUiString(
                    entry.Key, entry.Fallback));
            }
        }

        if (m_CreatePanel
            && m_CreatePanel.Visibility() == winrt::Visibility::Visible
            && !m_PendingFiles.empty())
        {
            // 重建 "{0} items selected" 计数行
            std::vector<std::wstring> current = m_PendingFiles;
            this->EnterCreateList(current);
        }

        this->UpdateLanguageButtonCaption();
    }

    void StartPage::UpdateLanguageButtonCaption()
    {
        if (!m_LanguageButton)
        {
            return;
        }

        // 按钮显示目标语言的自称(不随界面语言本地化):
        // 当前中文 → 显示 English;否则 → 显示 中文
        std::wstring current = ::K7ModernGetAppLanguage()
            ? std::wstring(::K7ModernGetAppLanguage())
            : std::wstring();
        m_LanguageButton.Content(winrt::box_value(
            winrt::hstring(current == L"zh-Hans"
                ? L"English"
                : L"\u4E2D\u6587")));
    }

    void StartPage::ToggleLanguageClick(
        winrt::IInspectable const& sender,
        winrt::RoutedEventArgs const& e)
    {
        UNREFERENCED_PARAMETER(sender);
        UNREFERENCED_PARAMETER(e);

        std::wstring current = ::K7ModernGetAppLanguage()
            ? std::wstring(::K7ModernGetAppLanguage())
            : std::wstring();
        wchar_t const* next = (current == L"zh-Hans")
            ? L"en-US"
            : L"zh-Hans";

        if (::K7ModernSetAppLanguage(next))
        {
            this->RefreshTexts();
        }
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
        // 整卡可点 + 键盘可达(Button 承载)。
        // P1-7:模仿 Windows 11 系统应用的横向入口卡片——左侧强调色
        // 圆底图标 + 右侧标题/描述垂直居中;主题资源缺失时退回纯图标。
        winrt::Button button;
        button.HorizontalAlignment(winrt::HorizontalAlignment::Stretch);
        button.VerticalAlignment(winrt::VerticalAlignment::Stretch);
        button.HorizontalContentAlignment(
            winrt::HorizontalAlignment::Left);
        button.VerticalContentAlignment(
            winrt::VerticalAlignment::Center);
        button.Padding(winrt::ThicknessHelper::FromLengths(14, 10, 14, 10));
        button.Margin(winrt::ThicknessHelper::FromUniformLength(4));
        button.MinHeight(68.0);
        button.UseSystemFocusVisuals(true);
        button.Click(click);

        winrt::StackPanel content;
        content.Spacing(12);

        winrt::FontIcon icon;
        icon.FontFamily(winrt::FontFamily(
            L"Segoe Fluent Icons,Segoe MDL2 Assets"));
        icon.FontSize(18.0);
        icon.Glyph(glyph);
        icon.HorizontalAlignment(winrt::HorizontalAlignment::Center);
        icon.VerticalAlignment(winrt::VerticalAlignment::Center);

        // 强调色圆底:优先 AccentFillColorDefaultBrush(SunValley 主题),
        // 缺失时退回 SystemAccentColorBrush,再缺失则不带底色
        winrt::Windows::UI::Xaml::Controls::Border iconHost;
        iconHost.Width(36.0);
        iconHost.Height(36.0);
        iconHost.CornerRadius(
            winrt::Windows::UI::Xaml::CornerRadiusHelper::
                FromUniformRadius(18.0));
        iconHost.Child(icon);
        {
            auto resources = winrt::Windows::UI::Xaml::Application::Current()
                .Resources();
            bool hasBackground = false;
            for (wchar_t const* key :
                { L"AccentFillColorDefaultBrush",
                  L"SystemAccentColorBrush" })
            {
                try
                {
                    iconHost.Background(resources.Lookup(
                        winrt::box_value(winrt::hstring(key)))
                        .as<winrt::Windows::UI::Xaml::Media::Brush>());
                    hasBackground = true;
                    break;
                }
                catch (...)
                {
                }
            }
            if (hasBackground)
            {
                // 圆底上的图标用强调色上的文本前景,保证对比度
                try
                {
                    icon.Foreground(resources.Lookup(winrt::box_value(
                        winrt::hstring(
                            L"TextOnAccentFillColorPrimaryBrush")))
                        .as<winrt::Windows::UI::Xaml::Media::Brush>());
                }
                catch (...)
                {
                    icon.Foreground(winrt::Windows::UI::Xaml::Media::
                        SolidColorBrush(winrt::Windows::UI::Colors::White()));
                }
            }
        }
        content.Children().Append(iconHost);

        winrt::StackPanel texts;
        texts.Spacing(2);
        texts.VerticalAlignment(winrt::VerticalAlignment::Center);

        winrt::TextBlock title = this->MakeText(
            titleKey, titleFallback, 15, true, false);
        texts.Children().Append(title);
        winrt::AutomationProperties::SetName(button, title.Text());

        winrt::TextBlock description = this->MakeText(
            descriptionKey, descriptionFallback, 12, false, true);
        texts.Children().Append(description);

        content.Children().Append(texts);

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
            L"Extract an archive, or create one from files and folders. You can also drag them here.",
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
            L"Choose files and folders to compress.",
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
            m_RecentSection = winrt::Grid();
            winrt::RowDefinition titleRow;
            titleRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            m_RecentSection.RowDefinitions().Append(titleRow);
            winrt::RowDefinition listRow;
            listRow.Height(winrt::GridLengthHelper::FromValueAndType(
                1, winrt::GridUnitType::Star));
            m_RecentSection.RowDefinitions().Append(listRow);
            winrt::Grid::SetRow(m_RecentSection, 2);
            m_RecentSection.Margin(
                winrt::ThicknessHelper::FromLengths(0, 20, 0, 0));
            m_RecentSection.Children().Append(this->MakeText(
                L"StartPage/RecentHeader.Text",
                L"Recent archives",
                16, true, false));

            m_RecentList = winrt::ListView();
            winrt::Grid::SetRow(m_RecentList, 1);
            m_RecentList.Margin(
                winrt::ThicknessHelper::FromLengths(0, 8, 0, 0));
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

                // P1-7:第二行带大小与修改时间,帮助用户辨认;
                // 单行省略号,悬停 Tooltip 展示完整路径与元数据
                std::wstring metaText = MakeRecentMetaText(path);
                winrt::TextBlock location;
                location.Text(winrt::hstring(metaText));
                location.FontSize(12.0);
                location.TextTrimming(
                    winrt::Windows::UI::Xaml::TextTrimming::
                        CharacterEllipsis);
                location.Foreground(this->GetSecondaryBrush());
                row.Children().Append(location);

                winrt::ListViewItem item;
                item.Content(row);
                winrt::AutomationProperties::SetName(item,
                    winrt::hstring(GetFileName(path)));
                winrt::Windows::UI::Xaml::Controls::ToolTipService::
                    SetToolTip(item, winrt::box_value(metaText));
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
            winrt::RowDefinition headerRow;
            headerRow.Height(winrt::GridLengthHelper::FromValueAndType(
                0, winrt::GridUnitType::Auto));
            m_CreatePanel.RowDefinitions().Append(headerRow);
            winrt::RowDefinition listRow;
            listRow.Height(winrt::GridLengthHelper::FromValueAndType(
                1, winrt::GridUnitType::Star));
            m_CreatePanel.RowDefinitions().Append(listRow);
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
            L"{0} items selected",
            14, false, true);
        content.Children().Append(m_CreateFilesText);
        content.Children().Append(this->MakeText(
            L"StartPage/CreateSelectionHint.Text",
            L"Add files or folders, then choose Next to set the archive name and save location. ZIP is a good choice for sharing.",
            13, false, true));

        winrt::StackPanel selectionButtons;
        selectionButtons.Orientation(
            winrt::Windows::UI::Xaml::Controls::Orientation::Horizontal);
        selectionButtons.Spacing(8);
        // P1-8:按钮文字经 MakeText 登记,语言切换时随注册表刷新
        winrt::Button addFiles;
        addFiles.Content(this->MakeText(
            L"StartPage/AddFilesText.Text", L"Add files", 0, false, false));
        addFiles.Click({ this, &StartPage::AddFilesClick });
        selectionButtons.Children().Append(addFiles);
        winrt::Button addFolders;
        addFolders.Content(this->MakeText(
            L"StartPage/AddFoldersText.Text", L"Add folders", 0, false, false));
        addFolders.Click({ this, &StartPage::AddFoldersClick });
        selectionButtons.Children().Append(addFolders);
        m_RemoveFiles = winrt::Button();
        m_RemoveFiles.Content(this->MakeText(
            L"StartPage/RemoveFilesText.Text", L"Remove selected",
            0, false, false));
        m_RemoveFiles.IsEnabled(false);
        m_RemoveFiles.Click({ this, &StartPage::RemoveFilesClick });
        selectionButtons.Children().Append(m_RemoveFiles);
        content.Children().Append(selectionButtons);

        winrt::TextBlock listHeader = this->MakeText(
            L"StartPage/CreateListHeader.Text",
            L"Files and folders to compress",
            14, true, false);
        content.Children().Append(listHeader);

        winrt::ListView list;
        list.SelectionMode(
            winrt::Windows::UI::Xaml::Controls::ListViewSelectionMode::Multiple);
        winrt::Grid::SetRow(list, 1);
        list.Margin(winrt::ThicknessHelper::FromLengths(0, 8, 0, 0));
        list.SelectionChanged([this](auto const&, auto const&)
        {
            m_RemoveFiles.IsEnabled(m_CreateFilesList.SelectedItems().Size() > 0);
        });
        m_CreateFilesList = list;
        winrt::AutomationProperties::SetName(list, listHeader.Text());
        m_CreatePanel.Children().Append(list);

        winrt::StackPanel buttons;
        buttons.Orientation(
            winrt::Windows::UI::Xaml::Controls::Orientation::Horizontal);
        buttons.Spacing(8);
        buttons.HorizontalAlignment(winrt::HorizontalAlignment::Right);
        buttons.Margin(winrt::ThicknessHelper::FromLengths(0, 16, 0, 0));
        winrt::Grid::SetRow(buttons, 2);

        winrt::Button cancel;
        cancel.Content(this->MakeText(
            L"StartPage/CreateCancelText.Text", L"Cancel", 0, false, false));
        cancel.Click({ this, &StartPage::CreateCancelClick });
        buttons.Children().Append(cancel);

        winrt::Button confirm;
        confirm.Content(this->MakeText(
            L"StartPage/CreateConfirmText.Text", L"Next", 0, false, false));
        m_CreateConfirm = confirm;
        confirm.IsEnabled(false);
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
        std::vector<std::wstring>& paths,
        bool archivesOnly)
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
        options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST
            | FOS_DONTADDTORECENT;
        if (multiSelect)
        {
            options |= FOS_ALLOWMULTISELECT;
        }

        winrt::hstring archiveLabel = winrt::NanaZip::Modern::GetUiString(
            L"StartPage/ArchiveFilterText.Text", L"Archives");
        winrt::hstring allFilesLabel = winrt::NanaZip::Modern::GetUiString(
            L"StartPage/AllFilesFilterText.Text", L"All files");
        COMDLG_FILTERSPEC filters[] =
        {
            { archiveLabel.c_str(), kArchivePattern },
            { allFilesLabel.c_str(), L"*.*" },
        };
        if (pickFolders)
        {
            options |= FOS_PICKFOLDERS;
        }
        else
        {
            options |= FOS_FILEMUSTEXIST;
            // Creating an archive needs ordinary input files. Applying the
            // archive filter here hides the files users want to compress.
            if (archivesOnly)
            {
                dialog->SetFileTypes(ARRAYSIZE(filters), filters);
            }
            else
            {
                dialog->SetFileTypes(1, &filters[1]);
            }
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

        this->EnterCreateList({});
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
        UINT32 index = 0;
        if (!m_RecentList.Items().IndexOf(e.ClickedItem(), index)
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

    void StartPage::AddFilesClick(
        winrt::IInspectable const&,
        winrt::RoutedEventArgs const&)
    {
        std::vector<std::wstring> paths;
        if (this->PickPaths(false, true, paths, false))
        {
            this->AppendCreatePaths(paths);
        }
    }

    void StartPage::AddFoldersClick(
        winrt::IInspectable const&,
        winrt::RoutedEventArgs const&)
    {
        std::vector<std::wstring> paths;
        if (this->PickPaths(true, true, paths, false))
        {
            this->AppendCreatePaths(paths);
        }
    }

    void StartPage::RemoveFilesClick(
        winrt::IInspectable const&,
        winrt::RoutedEventArgs const&)
    {
        std::vector<std::wstring> remaining;
        for (UINT32 i = 0; i < m_PendingFiles.size(); ++i)
        {
            UINT32 selectedIndex = 0;
            if (!m_CreateFilesList.SelectedItems().IndexOf(
                m_CreateFilesList.Items().GetAt(i), selectedIndex))
            {
                remaining.push_back(m_PendingFiles[i]);
            }
        }
        this->EnterCreateList(remaining);
    }

    void StartPage::AppendCreatePaths(std::vector<std::wstring> const& paths)
    {
        std::vector<std::wstring> combined = m_PendingFiles;
        for (std::wstring const& path : paths)
        {
            if (std::none_of(combined.begin(), combined.end(),
                [&path](std::wstring const& existing)
                {
                    return _wcsicmp(existing.c_str(), path.c_str()) == 0;
                }))
            {
                combined.push_back(path);
            }
        }
        this->EnterCreateList(combined);
    }

    void StartPage::HandleDroppedFiles(
        std::vector<std::wstring> const& paths)
    {
        if (paths.empty())
        {
            return;
        }

        // Additional drops on the confirmation page append to the selection,
        // including archives that the user wants to compress together.
        if (m_CreatePanel.Visibility() == winrt::Visibility::Visible)
        {
            this->AppendCreatePaths(paths);
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

        m_CreateConfirm.IsEnabled(!paths.empty());
        m_RemoveFiles.IsEnabled(false);

        // Items can be files or folders.
        std::wstring summary(winrt::NanaZip::Modern::GetUiString(
            L"StartPage/CreateFilesText.Text", L"{0} items selected"));
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
                winrt::AutomationProperties::SetName(item, winrt::hstring(path));
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
            INT32 count = 0;
            if (!TryWriteStartPaths(paths, m_Result->PathBuffer,
                m_Result->PathBufferCapacity, count))
            {
                winrt::hstring message = winrt::NanaZip::Modern::GetUiString(
                    L"StartPage/SelectionTooLargeText.Text",
                    L"This selection contains too many or overly long paths. Remove some items, or select their parent folder, then try again.");
                ::MessageBoxW(m_WindowHandle, message.c_str(), L"CuinZip",
                    MB_OK | MB_ICONINFORMATION);
                return;
            }
            m_Result->Action = action;
            m_Result->PathCount = count;
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
            return;
        }

        // P1-9 修复:鼠标拖放会被 OLE 路由到鼠标下最深层的 HWND——
        // XAML island 的输入子窗口覆盖了整个客户区,只注册顶层窗口时
        // 拖入显示禁止光标。给全部子窗口(现在与将来创建的)幂等注册。
        this->RegisterDropTargetOnChildren();

        // island 子窗口在 XAML 内容挂载后才创建,构造时刻可能还不存在;
        // 用 DispatcherTimer 短期重试(每秒一次,12 次后停),幂等注册
        try
        {
            m_DropRetryTimer = winrt::Windows::UI::Xaml::DispatcherTimer();
            m_DropRetryTimer.Interval(std::chrono::seconds(1));
            m_DropRetryTimer.Tick([this](
                winrt::IInspectable const&, winrt::IInspectable const&)
            {
                ++m_DropRetryCount;
                if (m_WindowHandle && ::IsWindow(m_WindowHandle))
                {
                    this->RegisterDropTargetOnChildren();
                }
                if (m_DropRetryCount >= 12 || !m_WindowHandle
                    || !::IsWindow(m_WindowHandle))
                {
                    if (m_DropRetryTimer)
                    {
                        m_DropRetryTimer.Stop();
                        m_DropRetryTimer = nullptr;
                    }
                }
            });
            m_DropRetryTimer.Start();
        }
        catch (...)
        {
            // DispatcherTimer 不可用时退化为一次性注册
            m_DropRetryTimer = nullptr;
        }
    }

    void StartPage::RegisterDropTargetOnChildren()
    {
        if (!m_DropTarget || !m_WindowHandle || !::IsWindow(m_WindowHandle))
        {
            return;
        }

        // EnumChildWindows 的回调签名是传统函数指针,经结构体传上下文
        struct DropRegisterContext
        {
            IDropTarget* Target;
            std::vector<HWND>* Registered;
        } context = {
            static_cast<IDropTarget*>(m_DropTarget.get()),
            &m_DropChildWindows
        };

        ::EnumChildWindows(
            m_WindowHandle,
            [](HWND hWnd, LPARAM lParam) -> BOOL
            {
                DropRegisterContext* context = reinterpret_cast<
                    DropRegisterContext*>(lParam);
                HRESULT result = ::RegisterDragDrop(hWnd, context->Target);
                if (SUCCEEDED(result))
                {
                    context->Registered->push_back(hWnd);
                }
                // 已注册(DRAGDROP_E_ALREADYREGISTERED)与其他失败均忽略,
                // 下一次重试会再尝试
                return TRUE;
            },
            reinterpret_cast<LPARAM>(&context));
    }
}

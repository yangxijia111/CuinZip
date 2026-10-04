#pragma once

#include "StartPage.g.h"

#include <Windows.h>

#include "NanaZip.Modern.h"

#include <string>
#include <vector>

namespace winrt
{
    using Windows::Foundation::IInspectable;
    using Windows::UI::Xaml::RoutedEventArgs;
}

namespace winrt::NanaZip::Modern::implementation
{
    // CuinZip P1-6:Home / Start 启动首屏(纯代码 UI)。
    // 布局:欢迎标题 + 副标题(首次启动即简洁空状态)+ 四个操作卡
    // (Open Archive / Extract / Create / Open Folder)+ Recent Archives
    // 列表(无记录时整区隐藏,保持简洁)。
    // 拖放:单个压缩包直接打开;普通文件/多文件/文件夹进入
    // "创建压缩包"待确认模式(可添加/移除文件与文件夹,下一步选输出)。
    struct StartPage : StartPageT<StartPage>
    {
    public:

        StartPage(
            _In_opt_ HWND WindowHandle,
            _In_opt_ K7_START_GET_RECENT_CALLBACK GetRecent,
            _In_opt_ K7_MODERN_START_RESULT* Result);

        // DropTarget 为嵌套不完整类型,析构在 .cpp 中展开
        ~StartPage();

        void InitializeComponent();

    private:

        // ---- 构建 ----

        void BuildHomePanel();
        void BuildCreatePanel();

        // 四个操作卡之一(图标 + 标题 + 描述,整卡可点)
        void BuildActionCard(
            winrt::Windows::UI::Xaml::Controls::Panel const& parent,
            int row,
            int column,
            wchar_t const* glyph,
            std::wstring_view const& titleKey,
            std::wstring_view const& titleFallback,
            std::wstring_view const& descriptionKey,
            std::wstring_view const& descriptionFallback,
            winrt::Windows::UI::Xaml::RoutedEventHandler const& click);

        // 文本助手(次级色走主题资源,同 SettingsPage 的 MakeText)
        winrt::Windows::UI::Xaml::Controls::TextBlock MakeText(
            std::wstring_view const& key,
            std::wstring_view const& fallback,
            double fontSize,
            bool semiBold,
            bool secondary);

        // 次要文本前景画刷(TextFillColorSecondaryBrush,缺失返回空)
        winrt::Windows::UI::Xaml::Media::Brush GetSecondaryBrush();

        // ---- 交互 ----

        void OpenArchiveCardClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void ExtractCardClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void CreateCardClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void OpenFolderCardClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void RecentItemClick(
            winrt::IInspectable const& sender,
            winrt::Windows::UI::Xaml::Controls::ItemClickEventArgs const& e);
        void CreateConfirmClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void CreateCancelClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void AddFilesClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void AddFoldersClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);
        void RemoveFilesClick(
            winrt::IInspectable const& sender,
            winrt::RoutedEventArgs const& e);

        // 文件/文件夹选择(Win32 IFileOpenDialog)
        bool PickPaths(
            bool pickFolders,
            bool multiSelect,
            std::vector<std::wstring>& paths,
            bool archivesOnly = true);

        // 拖入的文件:单个压缩包直接打开,否则进入创建模式
        void HandleDroppedFiles(std::vector<std::wstring> const& paths);

        // 切换到"创建压缩包"待确认面板
        void EnterCreateList(std::vector<std::wstring> const& paths);
        void AppendCreatePaths(std::vector<std::wstring> const& paths);

        // 填充结果并关闭窗口
        void Finish(
            INT32 action,
            std::vector<std::wstring> const& paths);

        HWND m_WindowHandle;
        K7_START_GET_RECENT_CALLBACK m_GetRecent = nullptr;
        K7_MODERN_START_RESULT* m_Result = nullptr;

        std::vector<std::wstring> m_RecentPaths;
        std::vector<std::wstring> m_PendingFiles;

        // ---- 控件 ----
        winrt::Windows::UI::Xaml::Controls::Grid m_Root{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Grid m_HomePanel{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Grid m_CreatePanel{ nullptr };
        winrt::Windows::UI::Xaml::Controls::ListView m_RecentList{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Grid m_RecentSection{
            nullptr };
        winrt::Windows::UI::Xaml::Controls::Button m_CreateConfirm{
            nullptr };
        winrt::Windows::UI::Xaml::Controls::Button m_RemoveFiles{
            nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock m_CreateFilesText{
            nullptr };
        winrt::Windows::UI::Xaml::Controls::ListView m_CreateFilesList{
            nullptr };

        // ---- 拖放 ----
        void RegisterDropTarget();
        struct DropTarget;
        winrt::com_ptr<DropTarget> m_DropTarget;
    };
}

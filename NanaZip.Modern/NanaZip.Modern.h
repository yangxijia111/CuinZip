/*
 * PROJECT:    NanaZip.Modern
 * FILE:       NanaZip.Modern.h
 * PURPOSE:    Definition for NanaZip Modern Experience
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#ifndef NANAZIP_MODERN_EXPERIENCE
#define NANAZIP_MODERN_EXPERIENCE

#include <Windows.h>
#include <CommCtrl.h>

/**
 * @brief Get legacy string resource from NanaZip Modern Experience resources.
 * @param ResourceId The legacy string resource ID.
 * @return If the legacy string resource is found, it returns the pointer to the
 *         string. Otherwise, it returns nullptr.
 * @remark This function can be used without calling K7ModernInitialize.
 */
EXTERN_C LPCWSTR WINAPI K7ModernGetLegacyStringResource(
    _In_ UINT32 ResourceId);

/**
 * @brief Check whether NanaZip Modern Experience is available.
 * @return If NanaZip Modern Experience is available, it returns TRUE.
 *         Otherwise, it returns FALSE.
 */
EXTERN_C BOOL WINAPI K7ModernAvailable();

/**
 * @brief Initialize NanaZip Modern Experience.
 * @return If the function succeeds, it returns S_OK. Otherwise, it returns an
 *         HRESULT error code.
 */
EXTERN_C HRESULT WINAPI K7ModernInitialize();

/**
 * @brief Uninitialize NanaZip Modern Experience.
 * @return If the function succeeds, it returns S_OK. Otherwise, it returns an
 *         HRESULT error code.
 */
EXTERN_C HRESULT WINAPI K7ModernUninitialize();

/**
 * @brief Show the "Sponsor CuinZip" dialog.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowSponsorDialog(
    _In_opt_ HWND ParentWindowHandle);

/**
 * @brief Show the "About CuinZip" dialog.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param ExtendedMessage The extended message to be displayed. If this
 *                        parameter is nullptr, the extended message is empty.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowAboutDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR ExtendedMessage);

/**
 * @brief Show an information dialog with the specified title and content.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param Title The title of the information dialog. If this parameter is
 *              nullptr, the title is empty.
 * @param Content The content of the information dialog. If this parameter is
 *                nullptr, the content is empty.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowInformationDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_opt_ LPCWSTR Content);

/**
 * @brief The progress window status structure.
 */
typedef struct _K7_PROGRESS_WINDOW_STATUS
{
    /**
     * @brief If true, the progress is shown in bytes. If false, the progress is
     *        shown in files.
     */
    BOOLEAN BytesProgressMode;

    /**
     * @brief If true, the operation is compression. If false, the operation is
     *        extraction.
     */
    BOOLEAN CompressionMode;

    /**
     * @brief If true, there is an error. If false, there is no error.
     */
    BOOLEAN HaveError;

    /**
     * @brief The title of the progress window.
     */
    LPCWSTR Title;

    /**
     * @brief The current file path being processed.
     */
    LPCWSTR FilePath;

    /**
     * @brief The total size of the operation in bytes. If unknown, it is set to
     *        static_cast<UINT64>(-1).
     */
    UINT64 TotalSize;

    /**
     * @brief The processed size of the operation in bytes.
     */
    UINT64 ProcessedSize;

    /**
     * @brief The number of total files. If unknown, it is set to
     *        static_cast<UINT64>(-1).
     */
    UINT64 TotalFiles;

    /**
     * @brief The number of processed files.
     */
    UINT64 ProcessedFiles;

    /**
     * @brief The input size in bytes. For compression, it is the original size.
     *        For extraction, it is the compressed size. If unknown, it is set
     *        to static_cast<UINT64>(-1).
     */
    UINT64 InputSize;

    /**
     * @brief The output size in bytes. For compression, it is the compressed
     *        size. For extraction, it is the original size. If unknown, it is
     *        set to static_cast<UINT64>(-1).
     */
    UINT64 OutputSize;

    /**
     * @brief The status message.
     */
    LPCWSTR Status;
} K7_PROGRESS_WINDOW_STATUS, *PK7_PROGRESS_WINDOW_STATUS;

/**
 * @brief Update the progress window status.
 * @param WindowHandle A handle to the progress window which acquired from the
 *                     window subclass procedure.
 * @param Status The progress window status to be updated.
 * @remark You must call this function only in the window subclass procedure of
 *         the progress window.
 */
EXTERN_C VOID WINAPI K7ModernUpdateProgressWindowStatus(
    _In_ HWND WindowHandle,
    _In_ PK7_PROGRESS_WINDOW_STATUS Status);

/**
 * @brief The WM_COMMAND BN_CLICKED ID for the "Pause" button will be sent to
 *        the progress window when the "Pause" button is clicked.
 */
#define K7_PROGRESS_WINDOW_COMMAND_PAUSE 446

/**
 * @brief Set the paused mode of the progress window.
 * @param WindowHandle A handle to the progress window which acquired from the
 *                     window subclass procedure.
 * @param Paused If true, the UI of the progress window will be updated to the
 *               paused mode. If false, the UI of the progress window will be
 *               updated to the normal mode.
 * @remark You must call this function only in the window subclass procedure of
 *         the progress window.
 */
EXTERN_C VOID WINAPI K7ModernSetProgressWindowPausedMode(
    _In_ HWND WindowHandle,
    _In_ BOOL Paused);

/**
 * @brief Show the progress window.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param Title The title of the progress window. If this parameter is
 *              nullptr, the title is empty.
 * @param ShowCompressionInformation If true, the progress window will show
 *                                   the packed size and compression ratio.
 *                                   Otherwise, the packed size and compression
 *                                   ratio will be hidden.
 * @param WindowSubclassHandler The window subclass procedure for the progress
 *                              window.
 * @param WindowSubclassContext The context pointer for the window subclass
 *                              procedure.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowProgressWindow(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_ BOOL ShowCompressionInformation,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext);

/**
 * @brief The WM_COMMAND BN_CLICKED ID for the "OK" button will be sent to
 *        the copy location dialog when the "OK" button is clicked.
 */
#define K7_COPY_LOCATION_DIALOG_RESULT_OK 1
#define K7_COPY_LOCATION_DIALOG_RESULT_EXTRACT_ALL 2

/**
 * @brief Show the copy location dialog window.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param Title The title of the copy location dialog window.
                If this parameter is nullptr, the title is empty.
 * @param Subtitle The subtitle of the copy location dialog window.
                   If this parameter is nullptr, the subtitle is empty.
 * @param AdditionalInformation The additional information text of
 *                              the copy location dialog window.
 *                              If this parameter is nullptr, the additional
                                information text is empty.
 * @param InitialPath The initial path set on the copy location dialog.
 * @param WindowSubclassHandler The window subclass procedure for the progress
 *                              window.
 * @param WindowSubclassContext The context pointer for the window subclass
 *                              procedure.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowCopyLocationDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ LPCWSTR Title,
    _In_opt_ LPCWSTR Subtitle,
    _In_opt_ LPCWSTR AdditionalInformation,
    _In_opt_ LPCWSTR InitialPath,
    _In_ BOOL ShowExtractAll,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext);

/**
 * @brief Get the path of the copy location dialog.
 * @param WindowHandle A handle to the copy location dialog.
 * @return The path currently set.
 */
EXTERN_C LPCWSTR WINAPI K7ModernGetCopyLocationDialogPath(
    _In_ HWND WindowHandle);

/**
 * @brief Create the toolbar control for the main window.
 * @param ParentWindowHandle A handle to the owner window of the control to be
 *                           created. This parameter must be valid.
 * @param MoreMenuHandle A menu handle for the "More" menu. This parameter must
 *                       be valid.
 * @return The toolbar control instance pointer.
 */
EXTERN_C LPVOID WINAPI K7ModernCreateMainWindowToolBarPage(
    _In_ HWND ParentWindowHandle,
    _In_ HMENU MoreMenuHandle);

// **************** CuinZip P1-2 Modification Start ****************

/**
 * @brief 外观设置项的快照。NanaZip.Modern.dll 不直接读注册表,
 *        由宿主(FileManager)通过回调读写真实设置源(CFmSettings),
 *        避免双份注册表实现漂移。
 */
typedef struct K7_MODERN_APPEARANCE_SETTINGS
{
    BOOL ShowDots;
    BOOL ShowRealFileIcons;
    BOOL FullRow;
    BOOL ShowGrid;
    BOOL SingleClick;
    BOOL AlternativeSelection;
} K7_MODERN_APPEARANCE_SETTINGS;

typedef void(*K7_MODERN_SETTINGS_LOAD_CALLBACK)(
    _Out_ K7_MODERN_APPEARANCE_SETTINGS* Settings);

typedef void(*K7_MODERN_SETTINGS_APPLY_CALLBACK)(
    _In_ const K7_MODERN_APPEARANCE_SETTINGS* Settings);

/**
 * @brief Show the modern Settings window.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param LoadCallback The callback to load the current appearance settings.
 * @param ApplyCallback The callback to apply the changed appearance settings.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowSettingsDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_opt_ K7_MODERN_SETTINGS_LOAD_CALLBACK LoadCallback,
    _In_opt_ K7_MODERN_SETTINGS_APPLY_CALLBACK ApplyCallback);

/**
 * @brief Get a localized UI string from the modern resource file.
 * @param Name The resource name under the "NanaZip.Modern/Common" subtree.
 * @param Fallback The English fallback text if the resource is unavailable.
 * @return The localized string (UTF-16, owned by the module; valid until the
 *         next call).
 */
EXTERN_C LPCWSTR WINAPI K7ModernGetUiString(
    _In_ LPCWSTR Name,
    _In_opt_ LPCWSTR Fallback);

// **************** CuinZip P1-2 Modification End ****************

// **************** CuinZip P1-3 Modification Start ****************

/**
 * @brief 对话框镜像引擎:压缩/解压 Modern 对话框与 7-Zip 原对话框之间的
 *        通用桥接。宿主侧(NanaZip.Universal.Windows.exe)把原 rc 对话框
 *        以隐藏窗口方式创建为"数据引擎"(格式/方法/字典联动、注册表读写、
 *        OnOK 校验全部走原逻辑),Modern 页作为纯视图经本引擎读写控件,
 *        保证零业务逻辑复制、零行为漂移。
 */

/**
 * @brief Modern 页通过 WM_COMMAND(BN_CLICKED)发往宿主子类过程的结果码。
 *        OK 只有在原对话框 OnOK 完整通过校验后才会发送。
 */
#define K7_DIALOG_MIRROR_RESULT_OK     1
#define K7_DIALOG_MIRROR_RESULT_CANCEL 2

/** 组合框镜像读取的单项字符串容量(wchar 数,含终止符)。 */
#define K7_DIALOG_MIRROR_ITEM_TEXT 512

/**
 * @brief 镜像引擎回调表。所有回调均在宿主 UI 线程内同步调用。
 */
typedef struct K7_DIALOG_MIRROR_ENGINE
{
    /**
     * @brief 宿主侧上下文(隐藏对话框指针等),原样回传给每个回调。
     */
    void* Context;

    /**
     * @brief 读取组合框:项文本(每项 K7_DIALOG_MIRROR_ITEM_TEXT wchar)、
     *        项数据、当前选中项、编辑框当前文本、启用/可见状态。
     * @return 读到的项数;失败返回 (UINT)-1。MaxItems 为 Items/ItemData
     *         数组容量。
     */
    UINT (WINAPI* ReadCombo)(
        void* Context,
        UINT ControlId,
        wchar_t* Items,
        UINT MaxItems,
        LPARAM* ItemData,
        int* CurrentSelection,
        wchar_t* Text,
        UINT TextMax,
        BOOL* Enabled,
        BOOL* Visible);

    /**
     * @brief 读取文本(编辑框/静态/按钮标题);ControlId 为 0 时读窗口标题。
     */
    BOOL (WINAPI* ReadText)(
        void* Context,
        UINT ControlId,
        wchar_t* Text,
        UINT TextMax,
        BOOL* Enabled,
        BOOL* Visible);

    /**
     * @brief 读取复选框状态。
     */
    BOOL (WINAPI* ReadCheck)(
        void* Context,
        UINT ControlId,
        BOOL* Checked,
        BOOL* Enabled,
        BOOL* Visible);

    /**
     * @brief 设置组合框选中项,并向原对话框发送 CBN_SELCHANGE,
     *        触发原有的联动逻辑(格式/方法/字典刷新等)。
     */
    void (WINAPI* SetComboSelection)(
        void* Context,
        UINT ControlId,
        int Selection);

    /**
     * @brief 设置(可编辑)组合框编辑文本。原对话框无 EN_CHANGE 处理,
     *        不触发联动,仅在 OnOK 时读取。
     */
    void (WINAPI* SetComboText)(
        void* Context,
        UINT ControlId,
        const wchar_t* Text);

    /**
     * @brief 设置(非组合框)控件文本,如密码框。
     */
    void (WINAPI* SetText)(
        void* Context,
        UINT ControlId,
        const wchar_t* Text);

    /**
     * @brief 设置复选框勾选态,并向原对话框发送 BN_CLICKED,
     *        触发原有处理器(SFX 改名、显示密码切换等)。
     */
    void (WINAPI* SetCheck)(
        void* Context,
        UINT ControlId,
        BOOL Checked);

    /**
     * @brief 模拟按钮点击(浏览/选项等),触发原有 OnButtonClicked 处理器。
     *        可能嵌套打开经典子窗口(文件浏览/时间戳选项),Modern 页
     *        应在调用期间禁用自身以保持模态语义。
     */
    void (WINAPI* NotifyButtonClick)(
        void* Context,
        UINT ControlId);

    /**
     * @brief 模拟 IDOK:运行原 OnOK 全部校验(密码/内存/路径/分卷等)。
     *        校验失败时原逻辑会弹出错误提示且提前返回,此函数返回
     *        FALSE,Modern 页应保持打开并重新同步状态;全部通过返回
     *        TRUE,宿主侧 Info/注册表已被原逻辑完整写入。
     */
    BOOL (WINAPI* PressOK)(void* Context);
} K7_DIALOG_MIRROR_ENGINE, *PK7_DIALOG_MIRROR_ENGINE;

/**
 * @brief Show the modern "Add to Archive" dialog.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param Engine The dialog mirror engine bound to the hidden original dialog.
 * @param WindowSubclassHandler The window subclass procedure which receives
 *                              the result codes (K7_DIALOG_MIRROR_RESULT_*).
 * @param WindowSubclassContext The context pointer for the window subclass
 *                              procedure.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowCompressDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_ const K7_DIALOG_MIRROR_ENGINE* Engine,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext);

/**
 * @brief Show the modern "Extract" dialog.
 * @param ParentWindowHandle A handle to the owner window of the dialog to be
 *                           created. If this parameter is nullptr, the dialog
 *                           has no owner window.
 * @param Engine The dialog mirror engine bound to the hidden original dialog.
 * @param WindowSubclassHandler The window subclass procedure which receives
 *                              the result codes (K7_DIALOG_MIRROR_RESULT_*).
 * @param WindowSubclassContext The context pointer for the window subclass
 *                              procedure.
 * @return The message loop exit code of the dialog.
 */
EXTERN_C INT WINAPI K7ModernShowExtractDialog(
    _In_opt_ HWND ParentWindowHandle,
    _In_ const K7_DIALOG_MIRROR_ENGINE* Engine,
    _In_ SUBCLASSPROC WindowSubclassHandler,
    _In_ LPVOID WindowSubclassContext);

// **************** CuinZip P1-3 Modification End ****************

#endif // !NANAZIP_MODERN_EXPERIENCE

// **************** CuinZip P1-3 Modification Start ****************
// ModernDialogMirror.cpp

#include "StdAfx.h"

#include "ModernDialogMirror.h"

#include <Windowsx.h>

// 各 7-Zip 前端工程的主文件定义的本实例句柄
extern HINSTANCE g_hInstance;

// 镜像引擎上下文:隐藏对话框对象 + OnOK 完成标志。
struct CMirrorContext
{
  NWindows::NControl::CDialog *Dialog;
  bool *OKCompleted;
  HWND Window;
  INT_PTR Result;
};

namespace
{
  // 复刻 NWindows/Control/Dialog.cpp 中 DialogProcedure 的最小分发:
  // 首次消息携带 this → 存 GWLP_USERDATA,之后全部转给虚函数 OnMessage。
  // (该函数在 Dialog.cpp 中是文件静态,无法直接复用,此处仅复制窗口
  //  装配逻辑,不含任何业务代码。)
  INT_PTR APIENTRY ModernMirrorDialogProc(
      HWND dialogHWND, UINT message, WPARAM wParam, LPARAM lParam)
  {
    if (message == WM_INITDIALOG)
      ::SetWindowLongPtrW(dialogHWND, GWLP_USERDATA, (LONG_PTR)lParam);
    NWindows::NControl::CDialog *dialog =
        (NWindows::NControl::CDialog *)::GetWindowLongPtrW(
            dialogHWND, GWLP_USERDATA);
    if (!dialog)
      return FALSE;
    if (message == WM_INITDIALOG)
      dialog->Attach(dialogHWND);
    try
    {
      return dialog->OnMessage(message, wParam, lParam) ? TRUE : FALSE;
    }
    catch (...)
    {
      return TRUE;
    }
  }

  // 排空投递到隐藏对话框的后续消息(如 k_Message_ArcChanged),
  // 保证每次引擎操作返回后,控件状态已经稳定,Modern 侧同步读到的是终态。
  void DrainPostedMessages(HWND window)
  {
    MSG msg;
    while (::PeekMessageW(&msg, window, 0, 0, PM_REMOVE))
      ::DispatchMessageW(&msg);
  }

  HWND GetItem(HWND window, UINT id)
  {
    return ::GetDlgItem(window, (int)id);
  }

  // 隐藏窗口的子控件不能用 IsWindowVisible(父链隐藏恒为 FALSE),
  // 用 WS_VISIBLE 样式位判断 rc 模板中的初始可见性/ShowItem_Bool 结果。
  BOOL IsStyledVisible(HWND control)
  {
    return control && ((::GetWindowLongW(control, GWL_STYLE) & WS_VISIBLE) != 0);
  }

  void ReadControlState(
      HWND window, UINT id, BOOL *enabled, BOOL *visible)
  {
    if (enabled)
    {
      HWND control = GetItem(window, id);
      *enabled = control && ::IsWindowEnabled(control);
    }
    if (visible)
    {
      HWND control = GetItem(window, id);
      *visible = IsStyledVisible(control);
    }
  }

  UINT WINAPI MirrorReadCombo(
      void *context,
      UINT controlId,
      wchar_t *items,
      UINT maxItems,
      LPARAM *itemData,
      int *currentSelection,
      wchar_t *text,
      UINT textMax,
      BOOL *enabled,
      BOOL *visible)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return (UINT)-1;
    HWND combo = GetItem(ctx->Window, controlId);
    if (!combo)
      return (UINT)-1;

    ReadControlState(ctx->Window, controlId, enabled, visible);

    const int count = (int)::SendMessageW(combo, CB_GETCOUNT, 0, 0);
    if (count == CB_ERR)
      return (UINT)-1;
    const UINT num = (UINT)count < maxItems ? (UINT)count : maxItems;
    for (UINT i = 0; i < num; i++)
    {
      if (items)
      {
        wchar_t *dst = items + (size_t)i * K7_DIALOG_MIRROR_ITEM_TEXT;
        LRESULT len = ::SendMessageW(
            combo, CB_GETLBTEXTLEN, (WPARAM)i, 0);
        if (len == CB_ERR)
          len = 0;
        // 留出终止符空间
        if (len >= K7_DIALOG_MIRROR_ITEM_TEXT)
          len = K7_DIALOG_MIRROR_ITEM_TEXT - 1;
        if (len > 0)
        {
          LRESULT copied = ::SendMessageW(
              combo, CB_GETLBTEXT, (WPARAM)i, (LPARAM)dst);
          if (copied == CB_ERR)
            len = 0;
        }
        dst[len] = L'\0';
      }
      if (itemData)
        itemData[i] = (LPARAM)::SendMessageW(
            combo, CB_GETITEMDATA, (WPARAM)i, 0);
    }

    if (currentSelection)
      *currentSelection = (int)::SendMessageW(combo, CB_GETCURSEL, 0, 0);

    if (text && textMax)
    {
      // CBS_DROPDOWN 组合框:WM_GETTEXT 读取编辑部分当前文本
      ::SendMessageW(combo, WM_GETTEXT, (WPARAM)textMax, (LPARAM)text);
    }

    return (UINT)count;
  }

  BOOL WINAPI MirrorReadText(
      void *context,
      UINT controlId,
      wchar_t *text,
      UINT textMax,
      BOOL *enabled,
      BOOL *visible)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window || !text || !textMax)
      return FALSE;
    text[0] = L'\0';

    if (controlId == 0)
    {
      // 窗口标题(已含本地化的对话框标题与压缩包名后缀)
      ::GetWindowTextW(ctx->Window, text, (int)textMax);
      if (enabled)
        *enabled = TRUE;
      if (visible)
        *visible = TRUE;
      return TRUE;
    }

    HWND control = GetItem(ctx->Window, controlId);
    if (!control)
      return FALSE;

    ::SendMessageW(control, WM_GETTEXT, (WPARAM)textMax, (LPARAM)text);
    ReadControlState(ctx->Window, controlId, enabled, visible);
    return TRUE;
  }

  BOOL WINAPI MirrorReadCheck(
      void *context,
      UINT controlId,
      BOOL *checked,
      BOOL *enabled,
      BOOL *visible)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window || !checked)
      return FALSE;
    // 复刻 IsDlgButtonChecked 语义:仅 BST_CHECKED 视为勾选
    LRESULT state = ::SendDlgItemMessageW(
        ctx->Window, (int)controlId, BM_GETCHECK, 0, 0);
    if (state == (LRESULT)-1)
      return FALSE;
    *checked = (state == BST_CHECKED) ? TRUE : FALSE;
    ReadControlState(ctx->Window, controlId, enabled, visible);
    return TRUE;
  }

  void MirrorSetComboSelection(
      void *context, UINT controlId, int selection)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return;
    HWND combo = GetItem(ctx->Window, controlId);
    if (!combo)
      return;
    ::SendMessageW(combo, CB_SETCURSEL, (WPARAM)selection, 0);
    // 与原生行为一致:选中变化后对话框收到 CBN_SELCHANGE
    ::SendMessageW(
        ctx->Window,
        WM_COMMAND,
        MAKEWPARAM(controlId, CBN_SELCHANGE),
        (LPARAM)combo);
    DrainPostedMessages(ctx->Window);
  }

  void MirrorSetComboText(
      void *context, UINT controlId, const wchar_t *text)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return;
    HWND combo = GetItem(ctx->Window, controlId);
    if (!combo)
      return;
    ::SendMessageW(combo, WM_SETTEXT, 0, (LPARAM)(text ? text : L""));
    DrainPostedMessages(ctx->Window);
  }

  void MirrorSetText(
      void *context, UINT controlId, const wchar_t *text)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return;
    HWND control = GetItem(ctx->Window, controlId);
    if (!control)
      return;
    ::SendMessageW(control, WM_SETTEXT, 0, (LPARAM)(text ? text : L""));
    DrainPostedMessages(ctx->Window);
  }

  void MirrorSetCheck(void *context, UINT controlId, BOOL checked)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return;
    HWND control = GetItem(ctx->Window, controlId);
    if (!control || !::IsWindowEnabled(control))
      return;
    ::SendMessageW(
        control,
        BM_SETCHECK,
        checked ? BST_CHECKED : BST_UNCHECKED,
        0);
    // 与原生行为一致:用户点击复选框后对话框收到 BN_CLICKED,
    // 触发原处理器(SFX 改名、显示密码切换、SplitDest 显隐等)
    ::SendMessageW(
        ctx->Window,
        WM_COMMAND,
        MAKEWPARAM(controlId, BN_CLICKED),
        (LPARAM)control);
    DrainPostedMessages(ctx->Window);
  }

  void MirrorNotifyButtonClick(void *context, UINT controlId)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window)
      return;
    HWND control = GetItem(ctx->Window, controlId);
    if (!control)
      return;
    ::SendMessageW(
        ctx->Window,
        WM_COMMAND,
        MAKEWPARAM(controlId, BN_CLICKED),
        (LPARAM)control);
    DrainPostedMessages(ctx->Window);
  }

  BOOL WINAPI MirrorPressOK(void *context)
  {
    CMirrorContext *ctx = (CMirrorContext *)context;
    if (!ctx || !ctx->Window || !ctx->OKCompleted)
      return FALSE;
    *ctx->OKCompleted = false;
    // 经标准 WM_COMMAND(IDOK)分发,OnOK 内的全部校验与回写
    // (含注册表保存)与经典对话框完全一致
    ::SendMessageW(
        ctx->Window,
        WM_COMMAND,
        MAKEWPARAM(IDOK, BN_CLICKED),
        0);
    DrainPostedMessages(ctx->Window);
    return *ctx->OKCompleted ? TRUE : FALSE;
  }

  // Modern 窗口子类过程:接收结果码(K7_DIALOG_MIRROR_RESULT_*)
  LRESULT CALLBACK MirrorWindowHandler(
      _In_ HWND hWnd,
      _In_ UINT uMsg,
      _In_ WPARAM wParam,
      _In_ LPARAM lParam,
      _In_ UINT_PTR uIdSubclass,
      _In_ DWORD_PTR dwRefData)
  {
    UNREFERENCED_PARAMETER(uIdSubclass);
    CMirrorContext *ctx = (CMirrorContext *)dwRefData;
    if (ctx && uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
    {
      switch (LOWORD(wParam))
      {
      case K7_DIALOG_MIRROR_RESULT_OK:
        ctx->Result = IDOK;
        return 0;
      case K7_DIALOG_MIRROR_RESULT_CANCEL:
        ctx->Result = IDCANCEL;
        return 0;
      default:
        break;
      }
    }
    return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);
  }
}

namespace NModernDialogMirror
{
  INT_PTR Show(
      HWND wndParent,
      NWindows::NControl::CDialog &dialog,
      unsigned resourceId,
      bool &okCompleted,
      INT (WINAPI *showModern)(
          HWND ParentWindowHandle,
          const K7_DIALOG_MIRROR_ENGINE *Engine,
          SUBCLASSPROC WindowSubclassHandler,
          LPVOID WindowSubclassContext))
  {
    // 隐藏的无模式创建:OnInit(注册表读取、组合框填充、默认联动)
    // 在创建时同步执行,与模态路径一致
    HWND hidden = ::CreateDialogParamW(
        g_hInstance,
        MAKEINTRESOURCEW(resourceId),
        wndParent,
        ModernMirrorDialogProc,
        (LPARAM)&dialog);
    if (!hidden)
      return 0;

    CMirrorContext context;
    context.Dialog = &dialog;
    context.OKCompleted = &okCompleted;
    context.Window = hidden;
    context.Result = IDCANCEL;

    K7_DIALOG_MIRROR_ENGINE engine;
    engine.Context = &context;
    engine.ReadCombo = MirrorReadCombo;
    engine.ReadText = MirrorReadText;
    engine.ReadCheck = MirrorReadCheck;
    engine.SetComboSelection = MirrorSetComboSelection;
    engine.SetComboText = MirrorSetComboText;
    engine.SetText = MirrorSetText;
    engine.SetCheck = MirrorSetCheck;
    engine.NotifyButtonClick = MirrorNotifyButtonClick;
    engine.PressOK = MirrorPressOK;

    showModern(wndParent, &engine, MirrorWindowHandler, &context);

    // OnOK 全部通过时,原逻辑末尾的 EndDialog 对无模式对话框无效,
    // 此处统一销毁;窗口若已销毁则调用无副作用
    ::DestroyWindow(hidden);

    return context.Result;
  }
}
// **************** CuinZip P1-3 Modification End ****************

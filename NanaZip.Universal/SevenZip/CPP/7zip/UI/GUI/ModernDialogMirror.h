// **************** CuinZip P1-3 Modification Start ****************
// ModernDialogMirror.h
//
// 对话框镜像引擎(宿主侧)。把 7-Zip 原 rc 对话框以"隐藏的无模式对话框"
// 方式创建,继续作为数据引擎:OnInit 读注册表/填充组合框、CBN_SELCHANGE
// 与 BN_CLICKED 联动、OnOK 全部校验与回写都走原有代码;Modern 对话框
// (NanaZip.Modern.dll 的 CompressDialogPage/ExtractDialogPage)经
// K7_DIALOG_MIRROR_ENGINE 回调读写控件,是纯视图层。
//
// 设计约束(见 Docs/CUINZIP_UI_PLAN.md P1-3):
//  - 不重写 7-Zip 对话框业务逻辑,原对话框类零行为改动;
//  - 镜像对话框永不显示,只承载控件状态;
//  - K7ModernAvailable() 为假时调用方回退原模态对话框。

#ifndef ZIP7_INC_MODERN_DIALOG_MIRROR_H
#define ZIP7_INC_MODERN_DIALOG_MIRROR_H

#include <NanaZip.Modern.h>

#include "../../../Windows/Control/Dialog.h"

namespace NModernDialogMirror
{
  /**
   * 创建隐藏镜像对话框并进入 Modern 对话框消息循环。
   *  dialog       原 7-Zip 对话框对象(CModalDialog 派生);
   *  resourceId   rc 模板 ID(如 IDD_COMPRESS/IDD_EXTRACT);
   *  okCompleted  OnOK 完整通过校验的标志(对话框类在 OnOK 末尾置位);
   *  showModern   K7ModernShowCompressDialog / K7ModernShowExtractDialog。
   * 返回 IDOK/IDCANCEL;失败时返回 0(调用方应回退经典对话框)。
   */
  INT_PTR Show(
      HWND wndParent,
      NWindows::NControl::CDialog &dialog,
      unsigned resourceId,
      bool &okCompleted,
      INT (WINAPI *showModern)(
          HWND ParentWindowHandle,
          const K7_DIALOG_MIRROR_ENGINE *Engine,
          SUBCLASSPROC WindowSubclassHandler,
          LPVOID WindowSubclassContext));
}

#endif // ZIP7_INC_MODERN_DIALOG_MIRROR_H
// **************** CuinZip P1-3 Modification End ****************

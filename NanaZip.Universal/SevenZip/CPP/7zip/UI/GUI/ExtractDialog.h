// ExtractDialog.h

#ifndef ZIP7_INC_EXTRACT_DIALOG_H
#define ZIP7_INC_EXTRACT_DIALOG_H

#include "ExtractDialogRes.h"

#include "../../../Windows/Control/ComboBox.h"
#include "../../../Windows/Control/Edit.h"

#include "../Common/ExtractMode.h"

#include "../FileManager/DialogSize.h"

// **************** CuinZip P1-3 Modification Start ****************
// SFX 构建(NanaZip.Core.Sfx)不链接 NanaZip.Modern,编译剔除镜像接入
#ifndef Z7_SFX
#include <NanaZip.Modern.h>
#endif
// **************** CuinZip P1-3 Modification End ****************

#ifndef Z7_NO_REGISTRY
#include "../Common/ZipRegistry.h"
#endif

namespace NExtractionDialog
{
  /*
  namespace NFilesMode
  {
    enum EEnum
    {
      kSelected,
      kAll,
      kSpecified
    };
  }
  */
}

class CExtractDialog: public NWindows::NControl::CModalDialog
{
  #ifdef Z7_NO_REGISTRY
  NWindows::NControl::CDialogChildControl _path;
  #else
  NWindows::NControl::CComboBox _path;
  #endif

  #ifndef Z7_SFX
  NWindows::NControl::CEdit _pathName;
  NWindows::NControl::CEdit _passwordControl;
  NWindows::NControl::CComboBox _pathMode;
  NWindows::NControl::CComboBox _overwriteMode;
  #endif

  #ifndef Z7_SFX
  // int GetFilesMode() const;
  void UpdatePasswordControl();
  #endif

  void OnButtonSetPath();

  void CheckButton_TwoBools(UINT id, const CBoolPair &b1, const CBoolPair &b2);
  void GetButton_Bools(UINT id, CBoolPair &b1, CBoolPair &b2);
  virtual bool OnInit() Z7_override;
  virtual bool OnButtonClicked(unsigned buttonID, HWND buttonHWND) Z7_override;
  virtual void OnOK() Z7_override;

  #ifndef Z7_NO_REGISTRY

  // **************** NanaZip Modification Start ****************
  //virtual void OnHelp() Z7_override;
  // **************** NanaZip Modification End ****************

  NExtract::CInfo _info;

  #endif

  bool IsShowPasswordChecked() const { return IsButtonCheckedBool(IDX_PASSWORD_SHOW); }
public:
  // bool _enableSelectedFilesButton;
  // bool _enableFilesButton;
  // NExtractionDialog::NFilesMode::EEnum FilesMode;

  UString DirPath;
  UString ArcPath;

  #ifndef Z7_SFX
  UString Password;
  #endif
  bool PathMode_Force;
  bool OverwriteMode_Force;
  NExtract::NPathMode::EEnum PathMode;
  NExtract::NOverwriteMode::EEnum OverwriteMode;

  #ifndef Z7_SFX
  // CBoolPair AltStreams;
  CBoolPair NtSecurity;
  // **************** 7-Zip ZS Modification Start ****************
  CBoolPair OpnTrgFold;
  // **************** 7-Zip ZS Modification End ****************
  #endif

  CBoolPair ElimDup;
  // **************** NanaZip Modification Start ****************
  CBoolPair OpenFolder;
  // **************** NanaZip Modification End ****************

  INT_PTR Create(HWND aWndParent = NULL)
  {
    // **************** CuinZip P1-3 Modification Start ****************
    #ifndef Z7_SFX
    if (::K7ModernAvailable())
    {
      INT_PTR modernResult = this->ModernCreate(aWndParent);
      if (modernResult)
        return modernResult;
      // Modern 外壳创建失败(返回 0)时回退经典对话框
    }
    #endif
    // **************** CuinZip P1-3 Modification End ****************
    #ifdef Z7_SFX
    BIG_DIALOG_SIZE(240, 64);
    #else
    BIG_DIALOG_SIZE(300, 160);
    #endif
    return CModalDialog::Create(SIZED_DIALOG(IDD_EXTRACT), aWndParent);
  }

  // **************** CuinZip P1-3 Modification Start ****************
  #ifndef Z7_SFX
  // Modern 镜像模式:OnOK 完整通过校验(走到 CModalDialog::OnOK 前)
  // 置位;实现见 ExtractDialog.cpp。
  bool Modern_OK_Completed;

  INT_PTR ModernCreate(HWND wndParent);
  #endif
  // **************** CuinZip P1-3 Modification End ****************

  CExtractDialog():
    PathMode_Force(false),
    OverwriteMode_Force(false)
    #ifndef Z7_SFX
    , Modern_OK_Completed(false)
    #endif
  {
    ElimDup.Val = true;
  }

};

#endif

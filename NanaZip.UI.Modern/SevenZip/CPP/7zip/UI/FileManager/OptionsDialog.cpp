// OptionsDialog.cpp

#include "StdAfx.h"

#include "../../../Windows/Control/Dialog.h"
#include "../../../Windows/Control/PropertyPage.h"

#include "DialogSize.h"

#include "EditPage.h"
#include "EditPageRes.h"
#include "FoldersPage.h"
#include "FoldersPageRes.h"
#include "MenuPage.h"
#include "MenuPageRes.h"
#include "SettingsPage.h"
#include "SettingsPageRes.h"

#include "App.h"
#include "LangUtils.h"
#include "MyLoadMenu.h"

#include "resource.h"

using namespace NWindows;

// **************** CuinZip P1-2 Modification Start ****************
// 增加 startPage 参数:Modern 设置窗口的经典设置入口按分类直达
// 对应属性表页(0 = Integration, 1 = Folders, 2 = Edit, 3 = Settings)。
void OptionsDialog(HWND hwndOwner, HINSTANCE hInstance, unsigned startPage);
void OptionsDialog(HWND hwndOwner, HINSTANCE /* hInstance */, unsigned startPage)
// **************** CuinZip P1-2 Modification End ****************
{
  CMenuPage menuPage;
  CFoldersPage foldersPage;
  CEditPage editPage;
  CSettingsPage settingsPage;

  CObjectVector<NControl::CPageInfo> pages;
  BIG_DIALOG_SIZE(200, 200);

  const UINT pageIDs[] = {
      SIZED_DIALOG(IDD_MENU),
      SIZED_DIALOG(IDD_FOLDERS),
      SIZED_DIALOG(IDD_EDIT),
      SIZED_DIALOG(IDD_SETTINGS)
  };

  NControl::CPropertyPage *pagePointers[] = { &menuPage, &foldersPage, &editPage, &settingsPage };

  for (unsigned i = 0; i < ARRAY_SIZE(pageIDs); i++)
  {
    NControl::CPageInfo &page = pages.AddNew();
    page.ID = pageIDs[i];
    LangString_OnlyFromLangFile(page.ID, page.Title);
    page.Page = pagePointers[i];
  }

  INT_PTR res = NControl::MyPropertySheet(pages, hwndOwner, LangString(IDS_OPTIONS), startPage);

  if (res != -1 && res != 0)
  {
    g_App.SetListSettings();
    g_App.RefreshAllPanels();
    // ::PostMessage(hwndOwner, kLangWasChangedMessage, 0 , 0);
  }
}

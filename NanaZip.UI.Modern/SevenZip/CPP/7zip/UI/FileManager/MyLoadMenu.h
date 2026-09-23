// MyLoadMenu.h

#ifndef __MY_LOAD_MENU_H
#define __MY_LOAD_MENU_H

// CuinZip P1-2: 本头引用 K7_MODERN_APPEARANCE_SETTINGS,须先包含 K7 导出声明。
#include <NanaZip.Modern.h>

void OnMenuActivating(HWND hWnd, HMENU hMenu, int position);
// void OnMenuUnActivating(HWND hWnd, HMENU hMenu, int id);
// void OnMenuUnActivating(HWND hWnd);

bool OnMenuCommand(HWND hWnd, unsigned id);
void MyLoadMenu();

// **************** CuinZip P1-2 Modification Start ****************
// 经典设置属性表(Modern 设置窗口的经典入口也会用到)。
void OptionsDialog(HWND hwndOwner, HINSTANCE hInstance, unsigned startPage);
// Modern 设置窗口的外观 / 常规设置读取 / 应用回调(在 FM.cpp 实现)。
void FmModernSettingsLoad(K7_MODERN_APPEARANCE_SETTINGS* settings);
void FmModernSettingsApply(const K7_MODERN_APPEARANCE_SETTINGS* settings);
// **************** CuinZip P1-2 Modification End ****************

// **************** CuinZip P1-4 Modification Start ****************
// Modern 设置窗口的右键菜单 / 压缩 / 解压默认设置回调(在 FM.cpp 实现)。
// 真实设置源:CContextMenuInfo / NCompression::CInfo / NExtract::CInfo。
void FmModernContextMenuLoad(K7_MODERN_CONTEXT_MENU_SETTINGS* settings);
void FmModernContextMenuApply(const K7_MODERN_CONTEXT_MENU_SETTINGS* settings);
void FmModernCompressionLoad(K7_MODERN_COMPRESSION_SETTINGS* settings);
void FmModernCompressionApply(const K7_MODERN_COMPRESSION_SETTINGS* settings);
void FmModernExtractionLoad(K7_MODERN_EXTRACTION_SETTINGS* settings);
void FmModernExtractionApply(const K7_MODERN_EXTRACTION_SETTINGS* settings);
// **************** CuinZip P1-4 Modification End ****************

struct CFileMenu
{
  bool programMenu;
  bool readOnly;
  bool isHashFolder;
  bool isFsFolder;
  bool allAreFiles;
  bool isAltStreamsSupported;
  int numItems;
  
  FString FilePath;

  CFileMenu():
      programMenu(false),
      readOnly(false),
      isHashFolder(false),
      isFsFolder(false),
      allAreFiles(false),
      isAltStreamsSupported(true),
      numItems(0)
    {}

  void Load(HMENU hMenu, unsigned startPos);
};

bool ExecuteFileCommand(unsigned id);

#endif

// RegistryUtils.h

#ifndef __REGISTRY_UTILS_H
#define __REGISTRY_UTILS_H

#include "../../../Common/MyTypes.h"
#include "../../../Common/MyString.h"

void SaveRegLang(const UString &path);
void ReadRegLang(UString &path);

void SaveRegEditor(bool useEditor, const UString &path);
void ReadRegEditor(bool useEditor, UString &path);

void SaveRegDiff(const UString &path);
void ReadRegDiff(UString &path);

void ReadReg_VerCtrlPath(UString &path);

struct CFmSettings
{
  bool ShowDots;
  bool ShowRealFileIcons;
  bool FullRow;
  bool ShowGrid;
  bool SingleClick;
  bool AlternativeSelection;
  bool ArcHistory;
  bool PathHistory;
  bool CopyHistory;
  bool FolderHistory;
  bool LowercaseHashes;
  // bool Underline;

  bool ShowSystemMenu;

  // **************** CuinZip P1-6 Modification Start ****************
  // 启动时显示 Home / Start 页(默认 true)。
  bool ShowStartPage;
  // **************** CuinZip P1-6 Modification End ****************

  void Save() const;
  void Load();
};

// void SaveLockMemoryAdd(bool enable);
// bool ReadLockMemoryAdd();

bool ReadLockMemoryEnable();
void SaveLockMemoryEnable(bool enable);

bool WantArcHistory();
bool WantPathHistory();
bool WantCopyHistory();
bool WantFolderHistory();
bool WantLowercaseHashes();

void SaveFlatView(UInt32 panelIndex, bool enable);
bool ReadFlatView(UInt32 panelIndex);

// **************** CuinZip P1-6 Modification Start ****************
// 最近打开的压缩包(Home / Start 页 Recent Archives)。
// REG_MULTI_SZ 存储,最新在前,上限 10 条;SaveRecentArchive 幂等。
void ReadRecentArchives(UStringVector &paths);
void SaveRecentArchive(const UString &path);
// CuinZip P1-6.1:整列表写回(失效记录清理)。
void SaveRecentArchiveList(const UStringVector &paths);
// **************** CuinZip P1-6 Modification End ****************

/*
void Save_ShowDeleted(bool enable);
bool Read_ShowDeleted();
*/

#endif

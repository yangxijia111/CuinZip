// RegistryUtils.cpp

#include "StdAfx.h"

#include "../../../Common/IntToString.h"

#include "../../../Windows/Registry.h"

#include "RegistryUtils.h"

using namespace NWindows;
using namespace NRegistry;

#define REG_PATH_7Z TEXT("Software") TEXT(STRING_PATH_SEPARATOR) TEXT("CuinZip")

static LPCTSTR const kCUBasePath = REG_PATH_7Z;
static LPCTSTR const kCU_FMPath = REG_PATH_7Z TEXT(STRING_PATH_SEPARATOR) TEXT("FM");
// static LPCTSTR const kLM_Path = REG_PATH_7Z TEXT(STRING_PATH_SEPARATOR) TEXT("FM");

static LPCWSTR const kLangValueName = L"Lang";

static LPCWSTR const kViewer = L"Viewer";
static LPCWSTR const kEditor = L"Editor";
static LPCWSTR const kDiff = L"Diff";
static LPCWSTR const kVerCtrlPath = L"7vc";

static LPCTSTR const kShowDots = TEXT("ShowDots");
static LPCTSTR const kShowRealFileIcons = TEXT("ShowRealFileIcons");
static LPCTSTR const kFullRow = TEXT("FullRow");
static LPCTSTR const kShowGrid = TEXT("ShowGrid");
static LPCTSTR const kSingleClick = TEXT("SingleClick");
static LPCTSTR const kAlternativeSelection = TEXT("AlternativeSelection");
// static LPCTSTR const kUnderline = TEXT("Underline");

static LPCTSTR const kShowSystemMenu = TEXT("ShowSystemMenu");

// **************** CuinZip P1-6 Modification Start ****************
// 启动时显示 Home / Start 页(默认开,普通用户保持引导首屏)。
static LPCTSTR const kShowStartPage = TEXT("ShowStartPage");
// 最近打开的压缩包(Home 页 Recent Archives;REG_MULTI_SZ,最新在前)。
static LPCWSTR const kRecentArchives = L"RecentArchives";
static const unsigned kRecentArchivesMax = 10;
// **************** CuinZip P1-6 Modification End ****************
// static LPCTSTR const kLockMemoryAdd = TEXT("LockMemoryAdd");
static LPCTSTR const kLargePages = TEXT("LargePages");

// they default to off (0) in 7-Zip ZS /TR
static LPCTSTR const kArcHistory = TEXT("WantArcHistory");
static LPCTSTR const kPathHistory = TEXT("WantPathHistory");
static LPCTSTR const kCopyHistory = TEXT("WantCopyHistory");
static LPCTSTR const kFolderHistory = TEXT("WantFolderHistory");
static LPCTSTR const kLowercaseHashes = TEXT("LowercaseHashes");

static LPCTSTR const kFlatViewName = TEXT("FlatViewArc");
// static LPCTSTR const kShowDeletedFiles = TEXT("ShowDeleted");

static void SaveCuString(LPCTSTR keyPath, LPCWSTR valuePath, LPCWSTR value)
{
  CKey key;
  key.Create(HKEY_CURRENT_USER, keyPath);
  key.SetValue(valuePath, value);
}

static void ReadCuString(LPCTSTR keyPath, LPCWSTR valuePath, UString &res)
{
  res.Empty();
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, keyPath, KEY_READ) == ERROR_SUCCESS)
    key.QueryValue(valuePath, res);
}

void SaveRegLang(const UString &path) { SaveCuString(kCUBasePath, kLangValueName, path); }
void ReadRegLang(UString &path) { ReadCuString(kCUBasePath, kLangValueName, path); }

void SaveRegEditor(bool useEditor, const UString &path) { SaveCuString(kCU_FMPath, useEditor ? kEditor : kViewer, path); }
void ReadRegEditor(bool useEditor, UString &path) { ReadCuString(kCU_FMPath, useEditor ? kEditor : kViewer, path); }

void SaveRegDiff(const UString &path) { SaveCuString(kCU_FMPath, kDiff, path); }
void ReadRegDiff(UString &path) { ReadCuString(kCU_FMPath, kDiff, path); }

void ReadReg_VerCtrlPath(UString &path) { ReadCuString(kCU_FMPath, kVerCtrlPath, path); }

static void Save7ZipOption(LPCTSTR value, bool enabled)
{
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  key.SetValue(value, enabled);
}

static void SaveOption(LPCTSTR value, bool enabled)
{
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCU_FMPath);
  key.SetValue(value, enabled);
}

static bool Read7ZipOption(LPCTSTR value, bool defaultValue)
{
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) == ERROR_SUCCESS)
  {
    bool enabled;
    if (key.QueryValue(value, enabled) == ERROR_SUCCESS)
      return enabled;
  }
  return defaultValue;
}

static bool ReadFMOption(LPCTSTR value)
{
  CKey key;
  bool enabled = false;
  if (key.Open(HKEY_CURRENT_USER, kCU_FMPath, KEY_READ) == ERROR_SUCCESS)
  {
    if (key.QueryValue(value, enabled) == ERROR_SUCCESS)
      return enabled;
  }
  return enabled;
}

static void ReadOption(CKey &key, LPCTSTR value, bool &dest)
{
  bool enabled = false;
  if (key.QueryValue(value, enabled) == ERROR_SUCCESS)
    dest = enabled;
}

/*
static void SaveLmOption(LPCTSTR value, bool enabled)
{
  CKey key;
  key.Create(HKEY_LOCAL_MACHINE, kLM_Path);
  key.SetValue(value, enabled);
}

static bool ReadLmOption(LPCTSTR value, bool defaultValue)
{
  CKey key;
  if (key.Open(HKEY_LOCAL_MACHINE, kLM_Path, KEY_READ) == ERROR_SUCCESS)
  {
    bool enabled;
    if (key.QueryValue(value, enabled) == ERROR_SUCCESS)
      return enabled;
  }
  return defaultValue;
}
*/

void CFmSettings::Save() const
{
  SaveOption(kShowDots, ShowDots);
  SaveOption(kShowRealFileIcons, ShowRealFileIcons);
  SaveOption(kFullRow, FullRow);
  SaveOption(kShowGrid, ShowGrid);
  SaveOption(kSingleClick, SingleClick);
  SaveOption(kAlternativeSelection, AlternativeSelection);
  SaveOption(kArcHistory, ArcHistory);
  SaveOption(kPathHistory, PathHistory);
  SaveOption(kCopyHistory, CopyHistory);
  SaveOption(kFolderHistory, FolderHistory);
  SaveOption(kLowercaseHashes, LowercaseHashes);
  // SaveOption(kUnderline, Underline);

  SaveOption(kShowSystemMenu, ShowSystemMenu);

  // **************** CuinZip P1-6 Modification Start ****************
  SaveOption(kShowStartPage, ShowStartPage);
  // **************** CuinZip P1-6 Modification End ****************
}

void CFmSettings::Load()
{
  ShowDots = false;
  ShowRealFileIcons = false;
  FullRow = false;
  ShowGrid = false;
  SingleClick = false;
  AlternativeSelection = false;
  ArcHistory = false;
  PathHistory = false;
  CopyHistory = false;
  FolderHistory = false;
  LowercaseHashes = false;
  // Underline = false;

  ShowSystemMenu = false;

  // **************** CuinZip P1-6 Modification Start ****************
  ShowStartPage = true; // 默认显示 Home / Start 引导页
  // **************** CuinZip P1-6 Modification End ****************

  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCU_FMPath, KEY_READ) == ERROR_SUCCESS)
  {
    ReadOption(key, kShowDots, ShowDots);
    ReadOption(key, kShowRealFileIcons, ShowRealFileIcons);
    ReadOption(key, kFullRow, FullRow);
    ReadOption(key, kShowGrid, ShowGrid);
    ReadOption(key, kSingleClick, SingleClick);
    ReadOption(key, kAlternativeSelection, AlternativeSelection);
    ReadOption(key, kArcHistory, ArcHistory);
    ReadOption(key, kPathHistory, PathHistory);
    ReadOption(key, kCopyHistory, CopyHistory);
    ReadOption(key, kFolderHistory, FolderHistory);
    ReadOption(key, kLowercaseHashes, LowercaseHashes);
    // ReadOption(key, kUnderline, Underline);

    ReadOption(key, kShowSystemMenu, ShowSystemMenu );

    // **************** CuinZip P1-6 Modification Start ****************
    ReadOption(key, kShowStartPage, ShowStartPage);
    // **************** CuinZip P1-6 Modification End ****************
  }
}


// void SaveLockMemoryAdd(bool enable) { SaveLmOption(kLockMemoryAdd, enable); }
// bool ReadLockMemoryAdd() { return ReadLmOption(kLockMemoryAdd, true); }

void SaveLockMemoryEnable(bool enable) { Save7ZipOption(kLargePages, enable); }
bool ReadLockMemoryEnable() { return Read7ZipOption(kLargePages, false); }

bool WantArcHistory() { return ReadFMOption(kArcHistory); }
bool WantPathHistory() { return ReadFMOption(kPathHistory); }
bool WantCopyHistory() { return ReadFMOption(kCopyHistory); }
bool WantFolderHistory() { return ReadFMOption(kFolderHistory); }
bool WantLowercaseHashes() { return ReadFMOption(kLowercaseHashes); }

static CSysString GetFlatViewName(UInt32 panelIndex)
{
  TCHAR panelString[16];
  ConvertUInt32ToString(panelIndex, panelString);
  return (CSysString)kFlatViewName + panelString;
}

void SaveFlatView(UInt32 panelIndex, bool enable) { SaveOption(GetFlatViewName(panelIndex), enable); }

bool ReadFlatView(UInt32 panelIndex)
{
  bool enabled = false;
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCU_FMPath, KEY_READ) == ERROR_SUCCESS)
    ReadOption(key, GetFlatViewName(panelIndex), enabled);
  return enabled;
}

/*
void Save_ShowDeleted(bool enable) { SaveOption(kShowDeletedFiles, enable); }
bool Read_ShowDeleted() { return ReadOption(kShowDeletedFiles, false); }
*/

// **************** CuinZip P1-6 Modification Start ****************
// Recent Archives(Home / Start 页数据源)。
// 存储:HKCU\Software\CuinZip\FM\RecentArchives(REG_MULTI_SZ,最新在前,
// 上限 kRecentArchivesMax 条)。记录点为面板状态栏刷新(打开压缩包
// 后必然触发),SaveRecentArchive 幂等:头部为同一路径时直接返回,
// 避免每次刷新写注册表。

void ReadRecentArchives(UStringVector &paths)
{
  paths.Clear();

  HKEY key = 0;
  if (::RegOpenKeyExW(
      HKEY_CURRENT_USER, kCU_FMPath, 0, KEY_READ, &key) != ERROR_SUCCESS)
    return;

  DWORD type = 0;
  DWORD size = 0;
  if (::RegQueryValueExW(
      key, kRecentArchives, nullptr, &type, nullptr, &size) == ERROR_SUCCESS
      && type == REG_MULTI_SZ && size >= sizeof(wchar_t) * 2)
  {
    CByteArr data(size);
    if (::RegQueryValueExW(
        key, kRecentArchives, nullptr, nullptr, data, &size) == ERROR_SUCCESS)
    {
      const wchar_t *cur = (const wchar_t *)(void *)(BYTE *)data;
      const wchar_t *end = cur + size / sizeof(wchar_t);
      while (cur < end && *cur != 0)
      {
        // 每段以 NUL 结尾,直接构造
        paths.Add(UString(cur));
        while (cur < end && *cur != 0)
          cur++;
        cur++;
      }
    }
  }

  ::RegCloseKey(key);
}

// CuinZip P1-6.1:整列表写回(失效记录清理用)。
void SaveRecentArchiveList(const UStringVector &paths)
{
  // 组装 REG_MULTI_SZ(双 NUL 结尾)。
  size_t total = 1;
  for (unsigned i = 0; i < paths.Size(); i++)
    total += paths[i].Len() + 1;
  total *= sizeof(wchar_t);

  CByteArr data(total);
  wchar_t *cur = (wchar_t *)(void *)(BYTE *)data;
  for (unsigned i = 0; i < paths.Size(); i++)
  {
    const UString &s = paths[i];
    const size_t len = s.Len();
    memcpy(cur, (const wchar_t *)s, len * sizeof(wchar_t));
    cur += len;
    *cur++ = 0;
  }
  *cur = 0;

  HKEY key = 0;
  if (::RegCreateKeyExW(
      HKEY_CURRENT_USER, kCU_FMPath, 0, nullptr, 0, KEY_WRITE, nullptr,
      &key, nullptr) == ERROR_SUCCESS)
  {
    ::RegSetValueExW(
        key, kRecentArchives, 0, REG_MULTI_SZ, (const BYTE *)(void *)data,
        static_cast<DWORD>(total));
    ::RegCloseKey(key);
  }
}

void SaveRecentArchive(const UString &path)
{
  if (path.IsEmpty())
    return;

  UStringVector paths;
  ReadRecentArchives(paths);

  // 幂等:已是最新的则不写(状态栏刷新高频调用)。
  if (paths.Size() > 0 && paths[0].IsEqualTo_NoCase(path))
    return;

  for (unsigned i = 0; i < paths.Size();)
  {
    if (path.IsEqualTo_NoCase(paths[i]))
      paths.Delete(i);
    else
      i++;
  }
  paths.Insert(0, path);
  if (paths.Size() > kRecentArchivesMax)
    paths.DeleteFrom(kRecentArchivesMax);

  SaveRecentArchiveList(paths);
}
// **************** CuinZip P1-6 Modification End ****************

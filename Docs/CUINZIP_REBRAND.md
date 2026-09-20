# CuinZip Rebrand 记录(P0-3)

日期:2026-09-21
基线:`754758cd`(标签 `p0-3-pre-rebrand`)
范围:仅品牌 / 身份 / 用户可见名称;不新增功能,不改压缩算法与 ABI。

## 1. 新身份(B 类)

| 项 | 旧值(NanaZip) | 新值(CuinZip) |
| --- | --- | --- |
| Package Identity(Preview) | `40174MouriNaruto.NanaZipPreview` | `Cuin.CuinZipPreview` |
| Package Identity(Release 侧,替换表内) | `40174MouriNaruto.NanaZip` | `Cuin.CuinZip` |
| Publisher | `CN=E310A153-74A9-4D81-800B-857A8D58408A` | `CN=D4DFD125-41D0-4903-95E0-896394D6DD55` |
| Shell Extension CLSID(Preview) | `469D94E9-6AF4-4395-B396-99B1308F8CE5` | `788F8FA7-178F-40BE-BAF5-5D5D1335C0F9` |
| Shell Extension CLSID(Release 侧,替换表内) | `CAE3F1D4-7765-4D98-A060-52CD14D56EAB` | `C8F7BD19-04D0-4086-820C-AD2F35AB89B0` |
| Inno Setup AppId | `{42795434-AB1A-4197-A724-F13E08953DFC}` | `{9BBA69D3-509F-4EA5-A78D-86BD39492F37}` |
| Publisher 显示名 | Kenji Mouri | CuinZip Project |

**注意**:Publisher 为本地开发/测试身份(未购买代码签名证书)。**正式发布 Microsoft Store
时必须重新设置 Publisher** 并与签名证书匹配;届时用户无法原地升级,需卸载重装。

Shell CLSID 同步修改的全部位置(4 处):
`NanaZip.ShellExtension.cpp`(DECLSPEC_UUID)、`Package.appxmanifest`(Verb Id ×3 + ComServer
Class)、`RefreshProjectResources.cs`(替换表)、`Documents/ChannelSwitchNote.md`(加注记)。

## 2. 用户可见品牌(A 类)

- `Package.appxmanifest`:DisplayName / Description / ShortName / ComServer 名称 / Verb Id
  (`0000CuinZipShellExtension`)/ AppExecutionAlias(`NanaZip*.exe` → `CuinZip*.exe`)
- 执行别名:`CuinZip.exe`、`CuinZipC.exe`、`CuinZipG.exe` 新增;`K7*` 与 `7z*` 兼容别名保留
- Classic / Modern 主窗口标题(FM.cpp ×2):`NanaZip` → `CuinZip`
- Modern About 页:标题 `About CuinZip`、`CuinZip <版本>`、`CuinZip is free software`、项目链接指向
  `github.com/yangxijia111/CuinZip`
- Explorer 右键菜单名(ShellExtension.cpp / Classic ContextMenu.cpp):`CuinZip` / `CuinZip Preview`
- 全部 97 个含品牌串的 resw(`NanaZipPackage/Strings` 79 + `NanaZip.Modern/Strings` 18):
  所有语言的 `NanaZip` → `CuinZip`
- 文件属性(13 个 vcxproj + K7Base/K7User):CompanyName=`CuinZip Project`、ProductName=`CuinZip`、
  FileDescription=`CuinZip …`;LegalCopyright 改为
  `© CuinZip Project and Contributors. Based on NanaZip (M2-Team) and 7-Zip (Igor Pavlov).
  All rights reserved.`(保留上游 attribution)
- 控制台横幅:`CuinZip 7.0 Preview, version … : (c) CuinZip Project and Contributors.
  Based on NanaZip and 7-Zip.`
- 7-Zip 继承 UI 内约 100 处 MessageBox / 进度窗口标题 `NanaZip` → `CuinZip`
  (4 份 7-Zip UI 拷贝:Core / Universal / Classic / Modern)
- SFX 对话框标题:`CAPTION "CuinZip SFX"`(SFXCon / SFXSetup / SFXWin)
- 档案文件夹类型 ID 前缀(Agent.cpp 生成端与 Panel.cpp 消费端同步):`NanaZip.` → `CuinZip.`
- 文件关联 ProgID 前缀(RegistryAssociations.cpp,读写同源):`NanaZip.` → `CuinZip.`
- 设置存储注册表键(4 份拷贝同步):`Software\NanaZip` → `Software\CuinZip`
- 错误消息 `kNoFormats`、主窗口标题兜底等散点字符串
- `BuildAllTargets.proj`:MSIX bundle 产物名 `NanaZipPreview_*` / `NanaZip_*` →
  `CuinZipPreview_*` / `CuinZip_*`
- Inno Setup(`NanaZip.ExtensionPackage.Installer.iss`):名称 / 发布者 / 版权 / URL(指向
  CuinZip 仓库)/ AppId / 输出文件名 `CuinZip.ExtensionPackage_*`
- `ReadMe.md`:重写为 CuinZip 项目说明,明确 "CuinZip is based on NanaZip and 7-Zip",
  保留上游链接与许可 attribution

## 3. 图标(全新创作)

上游 `Assets/` 下 NanaZip 图标为 **CC BY-ND 4.0**,禁止修改后再分发。CuinZip 未改动、未删除
任何上游图标原件(`Assets/NanaZip*`、`OriginalAssets/`、`OriginalAssetsOptimized/`、
`PackageAssets/`、`PreviewPackageAssets/` 均保持原样),而是全新创作占位图标:

- 设计:Win11 风格圆角方块、蓝系对角渐变、白色 Segoe UI Bold "CZ";文件关联图标为纸张 + 底部
  品牌条;SFX 附加解压箭头徽章;含 contrast-black/white 单色变体
- 新增文件:
  - `Assets/CuinZipPreviewPackageAssets/`(370 个 PNG,与 PreviewPackageAssets 文件名集合一致)
  - `Assets/CuinZipPackageAssets/`(370 个 PNG,正式版侧替换表用)
  - `Assets/CuinZipPreview.ico`、`Assets/CuinZipPreviewSfx.ico`、`Assets/CuinZip.ico`、
    `Assets/CuinZipSfx.ico`
  - `Assets/CuinZip.png`(ReadMe 徽标)
- 引用切换:`NanaZipPackage.wapproj` 与全部 `.rc` 的图标路径改指新资产;全部尺寸经 MSIX 标准
  校验(scale-125:50→63、44→55、150→188、310→388、71→89 等,共 370×2 个全部通过)

后续 P1/正式发布时应替换为专门设计的正式 Logo。

## 4. RefreshProjectResources.cs(防品牌回滚)

`NanaZip.Build.Tasks/RefreshProjectResources.cs` 的 Release↔Preview 双向替换表重写为 CuinZip
双轨:两个列表分别对应 `Cuin.CuinZip`(正式)与 `Cuin.CuinZipPreview`(预览)的全部身份串、
CLSID、图标资产路径。构建时替换为 no-op(文件已处于目标状态),**不再出现改回 NanaZip 的情况**。
本阶段已实测:Restore 与多次构建后 `git diff` 中品牌串无回滚。

## 5. 有意保留的 NanaZip 内部名称(C 类)

以下内容**有意不改**,原因是内部加载契约 / 工程 结构 / 上游 attribution,改动有明显风险而无
用户可见收益:

- 工程名与二进制名:`NanaZip.Core.dll`、`NanaZip.Codecs.dll`、`NanaZip.Modern.dll`、
  `NanaZip.Modern.FileManager.exe`、`NanaZip.Universal.Console/Windows.exe`、
  `NanaZip.ShellExtension.dll`、`NanaZip.exe`(Classic)等,以及全部工程目录 / vcxproj /
  slnx / wapproj 文件名与 ProjectGuid。原因:Core↔Codecs 动态加载、wapproj 引用、SFX 模块名
  (`kDefaultSfxModule "NanaZip.Core.Console.sfx"`)等内部契约彼此咬合,改名属结构性重构,
  留待后续阶段单独评估。
- 命名空间与导出符号:`namespace NanaZip::…`、`winrt::NanaZip::Modern`、`NanaZipCodecs*`
  导出函数(Codecs ABI)。原因:二进制接口契约。
- 窗口类名:`L"NanaZip.Modern.FileManager"`、`L"NanaZip::Panel"`(K7UserDarkMode 匹配键)。
  原因:内部匹配契约。
- 语言资源签名键:`LangUtils.cpp` 的 `lang.Open(fileName, "NanaZip")`。原因:语言文件签名
  匹配键,属内部兼容机制。
- PRI 资源 URI:`L"NanaZip.Modern/<page>/…"`(与 Modern 工程结构绑定)。
- `Mile.` 前缀文件:上游禁改。
- `NanaZip Modification Start/End` 标记与文件头 `MAINTAINER: MouriNaruto` 署名:
  上游修改标记与 attribution。
- 上游文档目录 `Documents/`(网站、Section508、PolicyDefinitions、ReleaseNotes 等)与
  `License.md`、`CODE_OF_CONDUCT.md` 等:上游文档与许可文本保持原样。
  `Documents/ChannelSwitchNote.md` 仅顶部加 CuinZip 注记。
- 组策略注册表键 `Software\Policies\M2Team\NanaZip`(K7BasePolicies.cpp 与 PolicyDefinitions
  .admx 一致):保留上游策略通道。
- Sponsor 赞助页:入口文本已品牌化为 CuinZip,但赞助链接与授权检查仍指向上游 NanaZip 机制
  (`Software\NanaZip` Sponsor 键保留);该功能的去留/替换在 P1 UI/UX 重构时统一处理。

## 6. 未修改的 7-Zip ABI 内容

- 7-Zip 接口 GUID 段 `23170F69-40C1-278A-…`(`NanaZip.Specification.SevenZip.h` 及全部
  接口定义):一字未动
- 压缩 / 解压缩算法代码、编解码器实现:未动
- unRAR 代码及其使用限制:未动(RAR 仅解压)
- Codecs 导出函数名与 Core↔Codecs 加载契约:未动
- 7-Zip 源码中的 "7-Zip" 字样(About/对话框中的上游署名):保留,属 attribution

## 7. 构建与测试结果

环境:P0-2B 定稿流程(`subst X:` + vcvarsall amd64;构建经 `build_x64.cmd`)。
`build_x64.cmd` 本次修复:echo 行括号导致的 cmd 语法错误、EncodedCommand 中损坏的路径、
LF→CRLF 行尾(纯 ASCII 输出,免疫代码页问题)。

- `RestoreNuGetPackages.cmd`:PASS(0 错误)
- x64 Debug 全项:Core / Codecs / Universal Console / Universal Windows / Modern /
  Modern FileManager / Classic / ShellExtension / MSIX(wapproj)全部 PASS,0 错误
  (首次 MSIX 打包遇 MSB4181 打包任务偶发失败,重新执行通过)
- Smoke Test:
  - Classic UI 启动,主窗口标题 `CuinZip`:PASS
  - Modern File Manager 启动,主窗口标题 `CuinZip`:PASS
  - `.7z` 创建 / 完整性测试 / 解压(Console a/t/e):PASS
  - SHA-256 源与解压输出一致(文本 + 100KB 随机文件):PASS
  - MSIX bundle 生成:PASS;包内 AppxManifest 为 `Cuin.CuinZipPreview` /
    `CN=D4DFD125-…` / 新 CLSID / `CuinZip*.exe` 别名:PASS
  - 品牌回滚检查:Restore 与构建后源文件品牌串保持 CuinZip(PASS);文件属性
    ProductName/FileDescription = CuinZip(PASS);控制台横幅 CuinZip(PASS)
- `BuildAllTargets.cmd`(Debug+Release × x64+arm64 + Packaging):**PASS,0 错误 /
  42 警告(与 P0-2B 一致)/ 30 分 39 秒**。本机物理内存(27.7GB)不足以支撑默认全并发
  重编重型 C++/WinRT 文件(反复出现 C1060 编译器堆空间不足,均落在 NanaZip.Modern 重型
  生成头),最终经等效 MSBuild 命令以 `-m:1 -p:CL_MPCount=1` 降并发完成;上游
  `BuildAllTargets.cmd` 未修改,降并发参数记录于临时 runner(`p03_buildall_runner.cmd`,
  仓库外)。分发产物名已品牌化:`CuinZipPreview_7.0.1846.0_Binaries.zip` /
  `CuinZipPreview_7.0.1846.0_DebugSymbols.zip`

## 8. 遗留事项

- Publisher `CN=D4DFD125-…` 仅用于本地开发/测试;正式上架前必须换正式证书与 Publisher
- Sponsor 页链接与授权机制仍指向上游,P1 统一处理
- 正式 Logo 待专门设计(当前为 CZ 占位图标)

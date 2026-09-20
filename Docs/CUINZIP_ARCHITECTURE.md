# CuinZip 架构审计（Architecture Notes）

> 阶段：P0-1（Fork 基线建立）
> 日期：2026-09-20
> 基线提交：`1f9d7d93`（upstream/main，NanaZip Preview 通道，版本 7.0.1845.0 / 显示版本 2609.3）
> 本文档基于对仓库 3973 个文件的扫描，为后续 CuinZip Rebrand 与开发规划提供地图。

---

## 0. 一图流：组件与产物

```
NanaZipPackage (MSIX/WAP 打包, Package.appxmanifest)
 ├─ NanaZip.Modern.FileManager.exe   ← NanaZip.UI.Modern (MSIX 主入口)
 ├─ NanaZip.Universal.Console.exe    ← NanaZip.Universal (7z.exe 等价)
 ├─ NanaZip.Universal.Windows.exe    ← NanaZip.Universal (7zG.exe 等价)
 ├─ NanaZip.exe (Classic)            ← NanaZip.UI.Classic (7-Zip FM 风格)
 ├─ NanaZip.Core.dll                 ← NanaZip.Core (7z.dll 等价, 压缩内核)
 ├─ NanaZip.Codecs.dll               ← NanaZip.Codecs (外部编解码器/哈希)
 ├─ NanaZip.ShellExtension.dll       ← NanaZip.UI.Modern (右键菜单 COM)
 ├─ NanaZip.Modern.dll               ← NanaZip.Modern (WinUI3 对话框库)
 ├─ K7Base.dll / K7User.dll          ← K7Base / K7User (平台基础/用户库)
 └─ *.sfx                            ← NanaZip.Core.Sfx.* (自解压模块)
NanaZip.ExtensionPackage → Inno Setup 安装器（非 MSIX 分发渠道）
```

## 1. 压缩核心在哪里

`NanaZip.Core/` —— 7-Zip `7z.dll` 的等价物，产物 **NanaZip.Core.dll**：
- 工程 `NanaZip.Core.vcxproj`（`MileProjectType=DynamicLibrary`），模块定义 `SevenZip/CPP/7zip/Archive/Archive2.def`；
- `Wrappers/`：BCrypt 的 SHA-1/256/512 包装；
- 附带 4 个 SFX 工程：`NanaZip.Core.Sfx.Console.vcxproj`（→ `NanaZip.Core.Console.sfx`）、`Sfx.Windows`（→ `.Windows.sfx`，带 5612 行多语言 STRINGTABLE 的 `NanaZip.Core.Sfx.Windows.Resources.rc`）、`Sfx.Setup`、`Sfx.Shared`（静态库）。

## 2. 7-Zip 上游代码在哪里

仓库内共 **4 份 vendored `SevenZip/` 副本 + 1 处 ZS Codecs**（无 submodule，均为仓库内拷贝，同步方式见 `Documents/UpstreamSynchronization.md`）：

| 路径 | 内容 | 用途 |
|---|---|---|
| `NanaZip.Core/SevenZip/` | ASM + C + CPP 全量 | 压缩内核 |
| `NanaZip.Universal/SevenZip/` | ASM + C + CPP 全量 | 控制台/GUI 前端 |
| `NanaZip.UI.Classic/SevenZip/` | C + CPP 子集 | Classic UI |
| `NanaZip.UI.Modern/SevenZip/` | C + CPP 子集 | Modern UI 宿主 |
| `NanaZip.Core/Extensions/ZSCodecs/` | ZS 系编解码 | 来自 `mcmilk/7-Zip-zstd` 分支 |

**改名红线**：这些目录整体按 7-Zip License（LGPL 为主）授权（见 `Docs/CUINZIP_LICENSE_NOTES.md` 第 3 节），对其修改必须沿用 `NanaZip Modification Start/End` 标记规范以便上游同步。

## 3. Codecs 在哪里

`NanaZip.Codecs/` —— 独立编解码器/哈希 DLL，产物 **NanaZip.Codecs.dll**：
- 导出（`NanaZip.Codecs.def`）：`GetHashers`、`CreateObject`、`GetNumberOfFormats`、`GetHandlerProperty2` 等 7-Zip 编解码器标准接口；
- 第三方算法源码目录：`BLAKE3/ Brotli/ FastLZMA2/ FreeBSD/ GmSSL/ LZ4/ LZ5/ LittleFS/ Lizard/ RHash/ ZSTDMT/ Zstandard/ xxHash/`（许可证各异，见许可证笔记第 4 节）；
- NanaZip 自有封装（MIT）：`NanaZip.Codecs.Archive.*.cpp`（DotNetSingleFile、ElectronAsar、Littlefs、Romfs、Ufs、WebAssembly、Zealfs 七种自有格式）、`NanaZip.Codecs.Hash.*.cpp`（30+ 哈希：Blake2b/3、Ed2k、EdonR、Gost94/12、Has160、Ripemd160、Sha3、Sm3、Snefru、Tiger、Tth、Whirlpool、Xxh3 等）、`NanaZip.Codecs.MultiThreadWrapper.*.cpp`（Brotli/LZ4/LZ5/Lizard 多线程包装）、`NanaZip.Codecs.SevenZipWrapper.*`（对接 7z.dll 接口规范 `NanaZip.Specification/NanaZip.Specification.SevenZip.h`，内含 7-Zip 接口 GUID 段定义公式）。

## 4. Classic UI 在哪里

`NanaZip.UI.Classic/` —— 传统 Win32 界面（7-Zip File Manager 风格）：
- 工程文件名注意：**`NanaZip.vcxproj`**（不是 NanaZip.UI.Classic.vcxproj），RootNamespace=`NanaZip`，产物 **NanaZip.exe**；
- 源码主体在其 `SevenZip/CPP/7zip/UI/FileManager/`（上游代码）；
- 工具栏资源 `Assets/Toolbar/`、`ParentFolder.bmp`。

## 5. Modern UI 在哪里

分两层，注意区分：

- `NanaZip.UI.Modern/` —— **现代 UI 宿主层（C++）**：
  - `NanaZip.Modern.FileManager.vcxproj` → **NanaZip.Modern.FileManager.exe**（MSIX 主入口，内嵌 WinUI3 XAML 岛）；
  - `NanaZip.ShellExtension.vcxproj` → **NanaZip.ShellExtension.dll**（右键菜单 COM 服务器，见第 7 节）；
  - `NanaZip.UI.cpp/h`：新旧 UI 桥接。
- `NanaZip.Modern/` —— **WinUI 3 对话框/页面库（C++/WinRT + XAML）** → **NanaZip.Modern.dll**：
  - `NanaZip.Modern.def` 导出 14 个 C 接口（`K7ModernInitialize`、`K7ModernShowSponsorDialog`、`K7ModernGetLegacyStringResource` 等），供 Classic/Console 等旧式 C++ UI 调用现代对话框；
  - 9 个 XAML 页面：AboutPage、ProgressPage、SponsorPage、CopyLocationPage、MainWindowToolBarPage 等。

## 6. 文件管理器在哪里

两个并存：
- **Classic 文件管理器**：`NanaZip.UI.Classic/SevenZip/CPP/7zip/UI/FileManager/`（上游 FM 代码）→ NanaZip.exe；
- **Modern 文件管理器**：`NanaZip.UI.Modern/`（宿主 + XAML）→ NanaZip.Modern.FileManager.exe，MSIX 中的主应用 `NanaZip.Modern`，且带执行别名 `NanaZip.exe` / `K7.exe` / `7zFM.exe`。

## 7. Windows 右键菜单 / Shell Extension 在哪里

- 实现：`NanaZip.UI.Modern/NanaZip.ShellExtension.cpp`（单文件，IExplorerCommand / IEnumExplorerCommand / IClassFactory）；
- 导出：`NanaZip.UI.Modern/NanaZip.ShellExtension.def`（`DllCanUnloadNow`、`DllGetClassObject`）；
- COM 注册：`NanaZipPackage/Package.appxmanifest` 中 `com:ComServer/com:Class`（Path=NanaZip.ShellExtension.dll，STA）+ `desktop4/desktop5/desktop10:FileExplorerContextMenus` Verb 注册，Verb Id=`0000NanaZipShellExtension`，覆盖 `*`、`Directory`、`Drive`；
- **CLSID：`469D94E9-6AF4-4395-B396-99B1308F8CE5`**，出现于 4 处：
  1. `NanaZip.UI.Modern/NanaZip.ShellExtension.cpp:1066`（`DECLSPEC_UUID`）
  2. `NanaZipPackage/Package.appxmanifest`（Verb Clsid= 与 com:Class Id=）
  3. `NanaZip.Build.Tasks/RefreshProjectResources.cs`（Preview/正式版通道切换替换表）
  4. `Documents/ChannelSwitchNote.md`（文档）
- 经典注册表式关联（非 MSIX 场景）：`NanaZip.UI.Classic/SevenZip/CPP/7zip/UI/FileManager/RegistryAssociations.cpp` 与 `NanaZip.UI.Modern/SevenZip/CPP/7zip/UI/FileManager/RegistryAssociations.cpp`。

## 8. MSIX Package 在哪里

- 工程：`NanaZipPackage/NanaZipPackage.wapproj`（WAP；`AppxPackageSigningEnabled=false`、`AppxBundle=Always`；平台 x64/arm64；打包图片取 `..\Assets\PreviewPackageAssets`）；
- 清单：`NanaZipPackage/Package.appxmanifest`：
  - **Identity Name=`40174MouriNaruto.NanaZipPreview`，Publisher=`CN=E310A153-74A9-4D81-800B-857A8D58408A`**；
  - DisplayName=`NanaZip Preview`，PublisherDisplayName=`Kenji Mouri`；
  - 能力：`runFullTrust`、`unvirtualizedResources`；
  - 3 个 Application：`NanaZip.Modern`（FileManager，别名 NanaZip.exe/K7.exe/7zFM.exe）、`NanaZip.Console`（别名 NanaZipC.exe/K7C.exe/7z.exe）、`NanaZip.Windows`（别名 NanaZipG.exe/K7G.exe/7zG.exe）；
- 另一分发渠道：`NanaZip.ExtensionPackage/`（**无 appxmanifest**），Inno Setup 脚本 `NanaZip.ExtensionPackage.Installer.iss`（AppId GUID `{42795434-AB1A-4197-A724-F13E08953DFC}`）+ `.proj`（经 NuGet 的 Tools.InnoSetup 6.4.2 编译）。

## 9. 文件关联相关代码在哪里

- MSIX 路径：`NanaZipPackage/Package.appxmanifest` `uap3:FileTypeAssociation`（Name=`fileassociations`，Parameters=`"%1" -open`，Logo=`Assets\ArchiveFile.png`），支持 60+ 扩展名（.7z/.zip/.zstd/.apfs…）+ `uap10:FileType *`；
- 注册表路径（Classic/扩展包）：上节两份 `RegistryAssociations.cpp` 及同目录 `Associations.cpp`、`RegUtils.cpp`；
- 组策略模板：`Documents/PolicyDefinitions/NanaZip.admx`。

## 10. 图标和图片资源在哪里

- 母版：`Assets/OriginalAssets/{NanaZip,NanaZipPreview}/{Standard,ContrastBlack,ContrastWhite,ArchiveFile,SelfExtractingExecutable}`（+ `OriginalAssetsOptimized`）——**CC BY-ND 4.0，禁止修改后再分发**（见许可证笔记第 6 节）；
- 生成物：`Assets/PackageAssets/` 与 `Assets/PreviewPackageAssets/`（各约 370 个 targetsize/scale/contrast PNG；打包实际引用后者）；
- 生成工具：`NanaZip.ProjectAssetsGenerator/`（C# + ImageMagick，从母版批量生成）；
- 当前品牌文件：`Assets/NanaZip.ico/png`、`NanaZipPreview.ico/png`、`NanaZipSfx.ico`、`NanaZipPreviewSfx.ico`（Preview 通道当前用 Preview 版）；
- .rc 图标引用（由 `RefreshProjectResources.cs` 在通道间自动切换）：`NanaZip.UI.Classic/SevenZip/CPP/7zip/UI/FileManager/resource.rc:141`、`NanaZip.UI.Modern/SevenZip/CPP/7zip/UI/FileManager/resource.rc:118`、`NanaZip.Universal/SevenZip/CPP/7zip/UI/GUI/resource.rc`、`NanaZip.Core/SevenZip/CPP/7zip/Bundles/{SFXCon,SFXSetup,SFXWin}/resource.rc`。

## 11. 本地化字符串在哪里

- 现代 UI：`NanaZip.Modern/Strings/<lang>/`——**79 种语言** × 5 个 resw（Common、CopyLocationPage、MainWindowToolBarPage、ProgressPage、SponsorPage）；
- Classic/Console 桥接：`NanaZipPackage/Strings/<lang>/Legacy.resw`——79 语言 × 1 个，旧资源 ID（如 3900）字符串；运行时经 `NanaZip.Modern.dll` 导出 `K7ModernGetLegacyStringResource` 取回（桥接点：`NanaZip.UI.Modern/SevenZip/CPP/Common/Lang.cpp:33`、`NanaZip.Universal/SevenZip/CPP/Common/Lang.cpp`）；
- Classic 语言加载逻辑：`NanaZip.UI.Classic/SevenZip/CPP/7zip/UI/FileManager/LangUtils.cpp`（`lang.Open(fileName, "NanaZip")` —— 注意这里字符串 "NanaZip" 是资源匹配键）；
- 迁移工具：`NanaZip.MigrateLegacyStringResources/`（C#，Legacy.resw 旧 ID → 新 resw 键）；
- SFX 多语言：`NanaZip.Core/NanaZip.Core.Sfx.Windows.Resources.rc`（全语言 STRINGTABLE）。
- resw 的 comment 必须保持英文（上游贡献规范）。

## 12. 产品名 NanaZip 出现在哪些关键配置中

| 文件/位置 | 形式 | 改名敏感度 |
|---|---|---|
| `NanaZipPackage/Package.appxmanifest` | Identity Name=`40174MouriNaruto.NanaZipPreview`；DisplayName/ShortName/Description；Executable/Alias（NanaZip.exe 等）；Verb Id=`0000NanaZipShellExtension`；ComServer DisplayName | **极高**（Identity/Verb 见第 14 节） |
| 17 个 `.vcxproj` | RootNamespace、MileProjectOriginalFilename（产物名）、MileProjectFileDescription、MileProjectProductName | 中（产物名/命名空间） |
| `NanaZip.slnx`、`NanaZip.MaintainerTools.slnx` | 解决方案与工程路径名 | 低（仅构建组织） |
| `NanaZipPackage/NanaZipPackage.wapproj` | 工程引用路径 | 低 |
| `NanaZip.Project/NanaZip.Project*.props` | 属性名 NanaZipBuild*/NanaZipMajorVersion 等（构建系统内部约定） | 中 |
| 各模块 `.props`（Codecs/Core/Modern/Specification/K7*） | 头注释与路径 | 低 |
| `NanaZip.Build.Tasks/RefreshProjectResources.cs` | **Preview↔正式版全量替换表**（Identity、DisplayName、.ico 引用、CLSID 等，第 48-78 行） | **极高**（改名必须同步维护） |
| `BuildAllTargets.proj` Packaging 段 | 产物文件名硬编码（K7Base.dll、NanaZip.*.dll/exe/sfx…） | 高（改产物名必改此处） |
| 目录名本身 | NanaZip.Core/ 等 12+ 个目录 | 中（引用面广） |
| 源码字符串 | `LangUtils.cpp` 的 `"NanaZip"` 资源键、关于页文本、日志前缀等 | 中 |

仓库内不存在 `.targets` 文件与 `.appinstaller` 文件（已核实）。

## 13. 哪些名称未来可以安全改（Rebrand 阶段的"安全区"）

以下改名**不破坏 ABI、不影响系统注册**，属纯展示/文档层，风险主要在工作量与遗漏：

1. 用户可见显示名：appxmanifest 的 DisplayName/Description/PublisherDisplayName（新 UI 文案、关于页）；
2. 文档与 ReadMe、`Documents/Website`（Astro 官网源码）；
3. 解决方案/工程/目录文件名（slnx、vcxproj、目录名）——仅构建组织层面，但**必须**全量同步所有引用（工程引用、props、wapproj、CI yml、BuildAllTargets.proj），建议放在独立 PR 用脚本统一处理；
4. MSIX 之外的品牌字符串（安装器界面文案等）；
5. **全新增**的 CuinZip 自有模块/文档（如本 `Docs/` 目录）。

前提：任何改名都不得触碰第 14 节所列标识符；且 `License.md`、版权声明、上游溯源声明保持原样（见许可证笔记）。

## 14. 哪些名称涉及 ABI/Package Identity/COM/GUID，不能简单替换（改名红线区）

| 类别 | 当前值 | 位置 | 为什么不能盲目替换 |
|---|---|---|---|
| MSIX Package Identity Name | `40174MouriNaruto.NanaZipPreview` | `NanaZipPackage/Package.appxmanifest` | Identity=发布者+名称决定包身份与升级链路。CuinZip 必须改，但**必须同时改 Publisher 并重新生成证书**，且意味着无法从 NanaZip 无缝"升级"，用户需卸载重装；Store 分发需新开发者账户 |
| Publisher | `CN=E310A153-74A9-4D81-800B-857A8D58408A` | 同上 | 与签名证书主体必须一致，改 Identity 必改签名链 |
| Shell Extension CLSID | `469D94E9-6AF4-4395-B396-99B1308F8CE5` | ShellExtension.cpp:1066、appxmanifest、RefreshProjectResources.cs、ChannelSwitchNote.md | COM 服务器身份。若与 NanaZip 并存安装而共用 CLSID 会互相覆盖注册；CuinZip 应**新生成**自己的 GUID（4 处同步改），避免与用户已装的 NanaZip 冲突 |
| Verb Id | `0000NanaZipShellExtension` | appxmanifest（desktop4/5/10 三处） | 资源管理器菜单注册键；与 CLSID 联动 |
| Inno Setup AppId | `{42795434-AB1A-4197-A724-F13E08953DFC}` | `NanaZip.ExtensionPackage/…Installer.iss` | 安装器升级/卸载身份，复用会导致与 NanaZip 扩展包互相误删 |
| 7-Zip 接口 GUID 段（`23170F69-40C1-278A-…` 公式） | 全部编解码接口 IID | `NanaZip.Specification.SevenZip.h` 及 SevenZip 代码 | **绝对不可改**——这是 NanaZip.Core.dll / Codecs.dll / 前端之间的二进制 ABI，改了即与 7-Zip 生态不兼容 |
| 产物二进制名（NanaZip.Core.dll、NanaZip.Codecs.dll、K7Base.dll、NanaZip.exe 及全部执行别名） | 见第 0 节 | vcxproj 的 MileProjectOriginalFilename、appxmanifest Executable/Alias、BuildAllTargets.proj、ShellExtension 加载代码 | DLL/EXE 名是加载契约：改名需同步全部加载点（如 Classic 对 Core.dll 的 LoadLibrary、别名依赖脚本）；执行别名（7z.exe 等）是用户脚本生态兼容面，**建议长期保留 7z/K7 系别名** |
| `K7Modern*` 导出函数名 | `NanaZip.Modern.def` 的 14 个导出 | NanaZip.Modern.dll ↔ 各前端 | C 接口 ABI，改名需同步全部 import 方与 def |
| RootNamespace / C++ 命名空间 | `NanaZip` | 17 个 vcxproj 及源码 | 改动面极大且无功能收益，Rebrand 阶段建议**保留 namespace 不动**（对外不可见），只改可见层 |
| `Mile.` 前缀文件与目录 | 如 `Mile.Helpers.Portable.Base.Unstaged.cpp` | 各处 | 上游明令禁止修改（见许可证笔记第 6 节） |

**Rebrand 策略建议**（供后续阶段参考）：先可见层（DisplayName/文档/图标），再产物层（vcxproj 产物名+加载点），最后 Identity 层（新证书+新 Identity+新 CLSID+新 AppId，一次性完成并冻结）。每层独立 PR、独立构建验证。

## 15. 构建系统与 CI（P0-2 直接相关）

- 构建入口：`BuildAllTargets.cmd`（清 Output → vswhere 定位 VS → `vcvarsall.bat x86` → `MSBuild -m BuildAllTargets.proj`）；`RestoreNuGetPackages.cmd` 仅做 `-t:Restore`；`BuildAllTargetsAsReleaseMode.cmd`（正式版）、`BuildAllTargetsWithSanitizers.cmd`（ASan）；
- `BuildAllTargets.proj` 四阶段：RefreshVersion（4 个自研构建任务，从 git 提交日期算版本号写入 `NanaZip.Project.Version.props` 与 appxmanifest）→ Restore → Build → Packaging（汇总 arm64/x64 到 `Output/Binaries/Root/` 并打 zip）；
- `global.json`：MSBuild SDK `Mile.Project.Configurations 1.1.2116`（**必须先 NuGet Restore**）；
- `Directory.Build.props`：统一输出到根 `Output/`；
- CI（`.github/workflows/`）：`BuildBinaries.yml`（push/PR 全量构建+artifact 上传）、`UpdatePrecompiledBuildTools.yml`（重建并提交 `NanaZip.Project/NanaZip.Build.Tasks.dll` 到专用分支）、`UpdateWebsite.yml`（Astro 官网→Pages）；
- **CuinZip 注意**：`RefreshVersion` 基于 git 提交日期，fork 后首次构建版本号会跳变，属预期；CI 中 push 到 main 的触发条件在改名仓库后仍有效，但 badge/分支名引用需 Rebrand 时同步。

## 16. K7Base / K7User 说明（勿删，勿因"看不懂"清理）

- `K7Base/`：NanaZip 平台基础库（非 GUI），产物 K7Base.dll + K7BaseStatic.lib。功能：哈希包装、Detours 包装、策略（AllowedHandler/AllowedCodec/WriteZoneId，`K7BasePolicies.cpp`）、安全缓解（`K7BaseMitigations.cpp`）。导出见 `K7Base.def`；
- `K7User/`：平台用户库（GUI），产物 K7User.dll + K7UserStatic.lib：暗色模式、现代 MessageBox/文件对话框包装；
- 命名来源：K7 = Kagura + Nana(7)，是 NanaZip 的内部别名，也是 K7.exe/K7C.exe/K7G.exe 别名的出处；
- 这两层是 CuinZip 未来做"安全检查/智能解压"功能时最合适的挂载点（策略钩子 `K7BaseGetAllowedHandlerPolicy` 已存在）。

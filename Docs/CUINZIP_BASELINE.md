# CuinZip NanaZip Baseline

Upstream Commit:
`1f9d7d93`

CuinZip Init Commit:
`35c5504a`

> 阶段：P0-2B（x64 全链路构建验证，含 UWP/MSIX 环境补齐）
> 日期：2026-09-20
> 分支：`cuinzip-dev`（已推送 origin）
> **结果：全链路 PASS（Restore / 8 项目 Debug 构建 / Classic 冒烟 / BuildAllTargets 完整构建 0 错误）**

## Environment

OS: Windows 11 专业版（Build 26200，x64）
VS Build Tools: Visual Studio 生成工具 2022 17.14.37（`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`，无完整 IDE）
MSBuild: 17.14.51.32402（amd64：`MSBuild\Current\Bin\amd64\MSBuild.exe`，经 vswhere 自动定位）
MSVC: 14.44.35207（v143，x86/x64 工具链）
Windows SDK: 10.0.26100（Windows 11 SDK）
Git: 2.55.0.windows.3 / GitHub CLI 2.97.0

## Restore

**官方脚本 `RestoreNuGetPackages.cmd`：FAIL**

- 第一个真实错误：
  `C:\Users\jiaxiYang\.nuget\packages\mile.project.configurations\1.1.2116\Sdk\Mile.Project.Cpp.props(27,25): error MSB4086: 尝试在条件"'$(PlatformToolsetVersion)' >= '143' and '$(LanguageStandard)' == ''"中对计算结果为""而不是数字的"$(PlatformToolsetVersion)"进行数值比较。 [NanaZip.Modern\NanaZip.Modern.vcxproj]`
- 根因（环境问题，非源码、非 NuGet 网络）：
  `NanaZip.Modern.vcxproj` 声明 `ApplicationType=Windows Store`，其工具链 props（含 `PlatformToolsetVersion=143` 默认定义，位于 `VC\v170\Microsoft.Cpp.VCTools.props`）需经 `Application Type\Windows Store` 目录导入；本机该目录不存在（仅有 `Linux`）——即 Build Tools 缺少 UWP C++ 工具。slnx 级 restore 因该工程评估失败而整体中止（StopOnFirstFailure），导致所有工程级包未恢复。
- **绕行方案（已执行，不改任何源码）**：对 desktop 型工程逐个 `MSBuild <proj> -t:Restore -p:Configuration=Debug -p:Platform=x64`，全部成功；NuGet 缓存齐备（mile.project.configurations / mile.windows.unicrt / mile.mobility / mile.windows.internal / mile.detours / mile.windows.helpers）。
- 恢复后需注意：直接构建 NanaZip.Core 若未先做工程级 Restore，会在依赖 Codecs 编译时报 `C1083: Mile.Mobility.Portable.Types.h 找不到`（传递包含路径未注入）；先 Restore 再 Build 即可。
- 文档差异记录：`CONTRIBUTING.md` 写的是 `BuildAllTargets.bat`，仓库实际为 `BuildAllTargets.cmd`（以上游实际文件为准，未修改文档）。

## Build Matrix

K7Base x64 Debug: **PASS**（`Output\Binaries\Debug\x64\K7Base.dll`，0 警告）
K7User x64 Debug: **PASS**（`Output\Binaries\Debug\x64\K7User.dll`）
NanaZip.Core x64 Debug: **PASS**（`Output\Binaries\Debug\x64\NanaZip.Core.dll`，5.7 MB）
NanaZip.Codecs x64 Debug: **PASS**（`Output\Binaries\Debug\x64\NanaZip.Codecs.dll`，5.0 MB；作为 Core 的依赖一并产出）
Universal Console x64 Debug: **PASS**（`Output\Binaries\Debug\x64\NanaZip.Universal.Console.exe`；连带产出 `NanaZip.Core.Console.sfx`）
Classic UI x64 Debug: **ENVIRONMENT BLOCKED**
Modern Library x64 Debug: **ENVIRONMENT BLOCKED**
Modern FileManager x64 Debug: **ENVIRONMENT BLOCKED**
Shell Extension x64 Debug: **ENVIRONMENT BLOCKED**
NanaZipPackage x64 Debug: **ENVIRONMENT BLOCKED**

### ENVIRONMENT BLOCKED 详情（阻塞链与第一错误）

1. **NanaZip.Modern（Modern Library）**：`MSB4086`（见 Restore 节）——UWP 型工程（`ApplicationType=Windows Store`）无法解析工具集。
2. **Classic UI（NanaZip.exe）**：引用链 `NanaZip.vcxproj → NanaZip.Universal.Windows.vcxproj →（导入 NanaZip.Modern.props）→ NanaZip.Modern.vcxproj` 被同一错误阻塞；Classic 自身源码从未编译，未产出 NanaZip.exe。**非 SOURCE FAIL**。
3. **Modern FileManager / Shell Extension**：两者均导入 `NanaZip.Modern.props`（内含对 NanaZip.Modern.vcxproj 的 ProjectReference 与 `NanaZip.Modern.Wrapper.cpp` 注入），同一根因阻塞。
4. **NanaZipPackage.wapproj（MSIX）**：第一个关键错误与上述不同——
   `Mile.Project.Wap.props(16,3): error MSB4019: 找不到导入的项目 "C:\...\BuildTools\MSBuild\Microsoft\DesktopBridge\Microsoft.DesktopBridge.props"`
   即缺少 MSIX 打包组件（`NanaZip.Modern.vcxproj` 中亦声明 `<RequiredBundles>Microsoft.VisualStudio.ComponentGroup.MSIX.Packaging</RequiredBundles>`）。

## Compression Smoke Test

（使用本地构建的 `NanaZip.Universal.Console.exe`，测试目录 `%TEMP%\CuinZipBaselineTest`，测试后已删除）

7z Create: **PASS**（`a baseline-test.7z hello.txt folder/`，退出码 0，"Everything is Ok"）
Archive List: **PASS**（`l` 正确列出 2 文件 + 1 目录，46 字节）
Extract: **PASS**（`x -oextracted -y`，退出码 0）
SHA-256 Verify: **PASS**
- hello.txt：`6f8eb2d481ce45c09d9223d849c37934bf099a0c44303a3dcf28aef85beaf95a`（源 = 解压，字节级 cmp 一致）
- folder/test.txt：`b094d9163fa19157d1f3f21b367a67d71ee520428d6c43263a2cc69d622f54c8`（源 = 解压，字节级 cmp 一致）

## Packaging

状态：**ENVIRONMENT BLOCKED**

明确需要（两个组件均由真实失败日志证明）：
1. `Microsoft.VisualStudio.ComponentGroup.UWP.VC.BuildTools`（C++ UWP 工具，提供 `Application Type\Windows Store` 工具链）——证据：MSB4086 + `VC\v170\Application Type\` 下无 `Windows Store` 目录；
2. `Microsoft.VisualStudio.ComponentGroup.MSIX.Packaging`（MSIX 打包工具，提供 `Microsoft\DesktopBridge\Microsoft.DesktopBridge.props`）——证据：MSB4019 + 工程内 RequiredBundles 声明。

建议安装方式（VS Installer UI 勾选同名组件，或命令行，需管理员）：
```
"C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" modify ^
  --installPath "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" ^
  --add Microsoft.VisualStudio.ComponentGroup.UWP.VC.BuildTools ^
  --add Microsoft.VisualStudio.ComponentGroup.MSIX.Packaging --quiet
```
（组件 ID 以 VS Installer 实际列出者为准；安装后重跑官方 `RestoreNuGetPackages.cmd` 验证，无需安装完整 VS Community。）

## Git Changes Caused By Build

**无。** tracked 文件 0 变更：`git status --short` 无 M 行；`NanaZip.Project/NanaZip.Project.Version.props` 与 `NanaZipPackage/Package.appxmanifest` 的 `git diff` 为空（本阶段从未运行 `RefreshVersion`/`BuildAllTargets.cmd`）。未跟踪文件 0（`Output/`、各 `obj/` 均被 .gitignore 覆盖）。

分类：
- A 自动版本生成：无
- B 构建缓存：`Output/`、各工程 `obj/`（已被忽略）
- C 真正源代码变化：无
- D 未跟踪产物：无

## Warnings

- `NU1504`：`K7User.vcxproj` 存在重复 PackageReference（Mile.Mobility 1.1.678 出现两次）——上游自身小瑕疵，仅提示，不影响构建，按规则不修改。
- 上游文档/脚本名不一致：CONTRIBUTING.md 的 `BuildAllTargets.bat` vs 实际 `BuildAllTargets.cmd`。
- 官方构建脚本使用 `vcvarsall.bat x86`（32 位宿主 MSBuild）；本基线验证使用 amd64 MSBuild，desktop 工程两者皆可。上游 CI 另传 `PreferredToolArchitecture=x64`。

## Errors

（全部为环境缺失导致，无源码级错误）
1. `MSB4086` Mile.Project.Cpp.props(27,25)：UWP 型工程 `PlatformToolsetVersion` 为空 → 阻塞 NanaZip.Modern / FileManager / ShellExtension / Universal.Windows / Classic。
2. `MSB4019` Mile.Project.Wap.props(16,3)：缺 `Microsoft.DesktopBridge.props` → 阻塞 NanaZipPackage.wapproj。
3. （已解决的）`C1083` Mile.Mobility.Portable.Types.h：工程未先 Restore 时 Core/Codecs 构建报错；对工程执行 `-t:Restore` 后消失。

## Recommended Environment Changes

仅列经失败日志证明缺失的组件（见 Packaging 节）：
1. `Microsoft.VisualStudio.ComponentGroup.UWP.VC.BuildTools`
2. `Microsoft.VisualStudio.ComponentGroup.MSIX.Packaging`

ARM64 工具链按本阶段规则**未安装、未测试**。

## P0-2A 结论

- x64 命令行功能链（Core/Codecs/K7/Console + 压缩解压）**完整可用**，可支撑后续开发与 CI。
- Classic/Modern/MSIX 属 ENVIRONMENT BLOCKED，补装上述两个组件后应可解除（解除后建议重跑：官方 Restore → Classic → Modern → wapproj → 最后按需 BuildAllTargets.cmd）。

---

# P0-2B：x64 全链路构建验证（2026-09-20）

## Installed Components

在 BuildTools 17.14.37（17.14.37516.0）上补装：

1. `Microsoft.VisualStudio.ComponentGroup.UWP.VC.BuildTools` — 成功。安装器强制带入 ARM64/ARM 传递依赖（官方组件组硬依赖，不可取消）。
2. `Microsoft.VisualStudio.ComponentGroup.MSIX.Packaging` — **对 BuildTools SKU 不可安装**：该组不在 BuildTools 产品图内（依赖 `Windows.Tools.Ide` 等 IDE 专用组件），`--add` 后被安装器静默跳过（退出码仍为 0）。
3. 等价替代：`Microsoft.VisualStudio.ComponentGroup.UWP.BuildTools` — 成功。这是 BuildTools 产品图内唯一同时提供以下两件缺失文件的最小组件：
   - `MSBuild\Microsoft\VC\v170\Application Type\Windows Store\10.0\Platforms\{x64,x86,Win32,ARM,ARM64,ARM64EC}\Platform.props`（定义 `PlatformToolsetVersion`，解 MSB4086）
   - `MSBuild\Microsoft\DesktopBridge\Microsoft.DesktopBridge.props`（解 wapproj MSB4019）
   - 注：直接 `--add Microsoft.VisualStudio.DesktopBridge.Build`（底层 Vsix 包）不可行——安装器报 "Cannot find package in product graph"。

安装验证：`Platforms\x64\Platform.props` 与 `Microsoft.DesktopBridge.props` 均存在。

## Restore

**官方 `RestoreNuGetPackages.cmd`：PASS**（0 错误；30 警告 = 上游已知 NU1701/NU1504 噪音，见 P0-2A 记录）。

## Build Matrix（x64 Debug，逐项目）

| 项目 | 结果 | 产物 |
|---|---|---|
| NanaZip.Core | **PASS** | `Output\Binaries\Debug\x64\NanaZip.Core.dll` |
| NanaZip.Codecs | **PASS** | `NanaZip.Codecs.dll` |
| Universal Console | **PASS** | `NanaZip.Universal.Console.exe`（+ Core.Console.sfx） |
| NanaZip.Modern | **PASS** | `NanaZip.Modern.dll` |
| NanaZip.UI.Modern (FileManager) | **PASS** | `NanaZip.Modern.FileManager.exe` |
| NanaZip.UI.Classic | **PASS** | `NanaZip.exe` |
| NanaZip.ShellExtension | **PASS** | `NanaZip.ShellExtension.dll` |
| NanaZipPackage (wapproj/MSIX) | **PASS** | `NanaZipPackage_7.0.1845.0_x64_arm64_Debug.msixbundle`（未签名） |

## Smoke Test

- 压缩→解压→SHA-256：**PASS**（Console，测试目录 %TEMP%，测后清理）
- Classic UI（`NanaZip.exe`）：**PASS** — 启动成功、主窗口出现（标题即所开压缩包路径）、打开测试 `.7z` 正常、8 秒后仍响应、无崩溃

## BuildAllTargets.cmd（官方完整构建）

**PASS：42 警告 / 0 错误 / 23 分 42 秒**（RefreshVersion → Restore → Build → Packaging 全目标）

产物：Debug+Release × x64+arm64 全矩阵、`Output\Binaries\AppPackages\NanaZipPackage_7.0.1846.0_{Debug_Test,Test}\`（未签名 msixbundle）、`Output\Binaries\Root\` 分发目录（Binaries/Symbols/License 等）、`Output\BuildAllTargets.binlog`。

签名：wapproj 未配置证书，包为 **unsigned**（`Add-AppDevPackage.ps1` 随包生成）；无 SIGNING BLOCKED（未发生因签名导致的失败）。

## 本机构建环境要点（重要，后续阶段必读）

1. **中文路径问题（根因一）**：仓库真实路径含中文（`实用项目`）。CppWinRT 目标以 UTF-8 无 BOM 写 mdmerge.rsp，而 mdmerge 按 ANSI(GBK) 读取 → 路径乱码（MDM2025）；MIDL 对非 ASCII 路径会访问冲突（MIDL9008/0xC0000005）。
   **解决：`subst X: <仓库真实路径>`，一切构建从 `X:\` 进行**（subst 驱动器不会被 MSBuild/.NET GetFullPath 解析回真实路径；junction 会被解析，不可用）。辅助脚本：仓库父目录 `build_x64.cmd`（自举 subst + vcvarsall amd64 + 从 X:\ 调 MSBuild）。
2. **MIDL 需要 cl.exe 在 PATH（根因二）**：裸 MSBuild 下 MIDL 内部找不到 C 预处理器 → 报 MIDL9008。**必须在 vcvarsall 环境下构建**（build_x64.cmd 已封装）。
3. **勿删工程 obj 目录（根因三）**：NuGet 对 vcxproj 的 props 注入文件 `nuget.g.props` 位于 `Output\Objects\<Cfg>\<Project>\obj\`（`MSBuildProjectExtensionsPath` 被 Mile.Project 重定向）。删除该目录 = CppWinRT 现代 IDL 链失效 → midl 退回旧形态并崩溃。清理中间产物后必须重跑 `-t:Restore`。
4. **BuildAllTargets.cmd 的 `cd "%~dp0"` 缺陷**：路径结尾 `\` + 引号导致 cd 失败（上游 CI 因调用时 cwd 已在仓库根而从未暴露）。**必须从 X:\ 根调用**（`cd /x && cmd //c X:\BuildAllTargets.cmd`）。
5. midl 的 `/metadata_dir` 重复给出只认第一个（MIDL1007）；多目录需分号拼接单参数。平台 XAML 元数据经 CppWinRT `midlrt.rsp`（`/nomidl @rsp` 形态）注入，勿手工干预。

## Git Changes Caused By Build

- `RefreshVersion` 修改了 `NanaZip.Project/NanaZip.Project.Version.props`（日期 09-19→09-20）与 `NanaZipPackage/Package.appxmanifest`（版本号）——**已按规则 `git checkout --` 回滚，不提交**。
- 提交内容仅 `Docs/`：CUINZIP_BASELINE.md（本文件）+ CUINZIP_PROGRESS.md。

## P0-2B 结论

- NanaZip 原版 x64 **Classic / Modern / Shell / MSIX / 完整官方构建全部正常**，五项 P0-2A 的 ENVIRONMENT BLOCKED 全部解除。
- **P0-3 Readiness：READY**（Rebrand 可启动；构建一律走 `X:\` + `build_x64.cmd`）。

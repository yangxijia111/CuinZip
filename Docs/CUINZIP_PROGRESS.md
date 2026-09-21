# CuinZip Progress

## Current Phase
P1-1 UI / UX Modernization

## Status
DONE

## Completed
- P0-1 Fork / Git / License Audit
- P0-2A Core build baseline
- Compression / extraction tests passed
- P0-2B 全链路构建验证（2026-09-20）：
  - UWP/MSIX 构建组件补齐（UWP.VC.BuildTools + 等价组件 UWP.BuildTools）
  - 官方 RestoreNuGetPackages.cmd PASS
  - x64 Debug 8/8 项目 PASS（Core/Codecs/Console/Modern/FileManager/Classic/ShellExtension/MSIX）
  - Classic UI Smoke Test PASS（启动/主窗口/打开 .7z/无崩溃）
  - BuildAllTargets.cmd PASS（0 错误 / 23 分 42 秒 / Debug+Release × x64+arm64 + MSIX bundle）
  - RefreshVersion 自动改动已回滚，未污染 Git
- P0-3 Branding / Rebrand（2026-09-21，详见 `Docs/CUINZIP_REBRAND.md`）：
  - 安全标签 `p0-3-pre-rebrand`（基线 `754758cd`）
  - 新身份：Package Identity `Cuin.CuinZip` / `Cuin.CuinZipPreview`、Publisher
    `CN=D4DFD125-…`（本地开发用）、Shell CLSID Release `C8F7BD19-…` / Preview
    `788F8FA7-…`、Inno AppId `{9BBA69D3-…}`
  - 用户可见品牌全量切换 CuinZip（manifest / 窗口标题 / 右键菜单 / resw×97 / 文件属性 /
    控制台横幅 / SFX 标题 / 安装器 / ReadMe），7-Zip ABI GUID 未动
  - 全新创作 CZ 占位图标（740 PNG + 4 ico + 徽标），未修改 CC BY-ND 上游图标
  - `RefreshProjectResources.cs` 重写为 CuinZip 双轨替换表，构建实测无 NanaZip 回滚
  - RestoreNuGetPackages PASS；x64 Debug 8/8 PASS（0 错误）；
    BuildAllTargets PASS（0 错误 / 42 警告 / 30 分 39 秒；本机内存所限经
    `-m:1 -p:CL_MPCount=1` 降并发完成，命令经临时 runner，上游脚本未改）
  - Smoke Test：Classic/Modern 窗口标题 CuinZip、.7z 往返 SHA-256 一致、MSIX 包内
    manifest 为 CuinZip 品牌、分发产物名 `CuinZipPreview_7.0.1846.0_*`，全部 PASS
  - RefreshVersion 自动改动（Version.props / manifest 版本号）已回滚，未污染 Git

## Current Task
无（P1-1 完成，建议进入 P1-2：Modern File Manager 主界面深度重构）

## P1-1 UI / UX Modernization
DONE（2026-09-21，安全标签 `p1-1-pre-ui`，基线 `fea047c4`，计划与审计见
`Docs/CUINZIP_UI_PLAN.md`）：
- Sponsor 入口彻底改造：工具栏按钮改为 `Open Source`（直接打开对话框，不再重启进程 /
  查询 Microsoft Store / 缓存授权状态）；`SponsorPage` 改为 Open Source Projects 页
  （CuinZip GitHub / NanaZip GitHub / 7-Zip 官网 + based-on 说明，保留上游 attribution）；
  删除 `--AcquireSponsorEdition` 参数链路；`K7ModernShowSponsorDialog` 导出 ABI 保留
- 工具栏重组：`Add / Extract / Test / Delete / Info` + `Options / More`；
  Copy / Move / Benchmark / About 收进 More（FM 完整主菜单，含 Tools/Help），
  F5/F6 等快捷键不变
- AboutPage 升级：`Based on NanaZip and 7-Zip.` attribution 行 +
  GitHub / NanaZip.org / 7-Zip.org / Close 四链接
- 验证：Modern 库 / FileManager / Classic / ShellExtension / MSIX（wapproj）构建全部
  0 错误；Restore PASS；.7z 压缩/解压 SHA-256 一致；Classic GUI Light/Dark 截图 PASS、
  打开 .7z PASS（`Docs/Screenshots/P1-1/`）
- 已知问题：本机直接运行 Modern FM（非 MSIX 部署）时 `K7ModernInitialize Failed`
  （unpackaged XAML 环境限制，P0-3 即存在，非 P1-1 回归）；Modern 完整 GUI/Light/Dark
  截图需管理员启用开发者模式后以 MSIX 部署验证

## Notes
- 本机构建必须从 `subst X:` 驱动器进行（中文路径会破坏 mdmerge/MIDL），用仓库父目录 `build_x64.cmd` 封装脚本
- 详见 `Docs/CUINZIP_BASELINE.md` 的 P0-2B 节与环境要点

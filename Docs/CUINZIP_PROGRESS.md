# CuinZip Progress

## Current Phase
P1-3 Compression / Extraction / Progress UX Modernization

## Status
DONE(2026-09-22;Modern MSIX 部署实测仍因 Developer Mode 未开启阻塞,其余全部完成)

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
无（P1-3 完成，建议进入 P1-4：设置 / 文件关联 / Explorer 右键菜单完善）

## P1-3 Compression / Extraction / Progress UX Modernization
DONE（2026-09-22，安全标签 `p1-3-pre-dialogs`，基线 `dfe3ae8c`，
计划与审计见 `Docs/CUINZIP_UI_PLAN.md` P1-3 节）：
- **对话框镜像引擎**：原 7-Zip rc 对话框（CompressDialog/ExtractDialog）以隐藏
  无模式窗口作为数据引擎，注册表/组合框联动/OnOK 校验零重写；新增
  `K7ModernShowCompressDialog` / `K7ModernShowExtractDialog` 导出（def + wrapper
  动态转发，宿主无静态 DLL 依赖）与通用 `K7_DIALOG_MIRROR_ENGINE` 回调 ABI；
  Modern 页为纯视图，每次交互后全量同步（格式切换的加密区/SFX/第二密码框
  显隐自动跟随）；SFX 构建（Z7_SFX）编译剔除镜像接入
- **CompressDialogPage**（纯代码 UI）：Archive / Compression / Encryption /
  Advanced(默认折叠) 四分区;高频优先;全部原有选项保留(格式/级别/方法/字典/
  字大小/固实/线程/内存上限+实时内存数值/分卷/参数/SFX/共享/删除后压缩/
  密码×2+显示切换+加密文件名+加密方法/Options 时间戳-NTFS 经典子对话框);
  字段标签取自原对话框(7-Zip Lang 本地化零迁移)
- **ExtractDialogPage**:Destination / Options / Password 分区;路径+浏览+拆分名+
  路径模式+覆盖模式+密码;主按钮 Extract(强调色);默认"解压到 <archive>\"规则
  原样保留
- **键盘/可访问性**:Enter=OK/Esc=Cancel 四层保障(PreviewKeyDown/KeyDown/
  加速器/控件级);Tab=声明序;AutomationProperties 全字段
- **进度链**:审计确认 NanaZip 上游已 Modern 化(百分比/文件/总量/速度/剩余/
  Pause 真实支持/Cancel 二段确认/错误经 Information 对话框),本阶段未改动
- **本地化**:`CompressDialogPage.resw`(6 键)/`ExtractDialogPage.resw`(7 键),
  en+zh-Hans,其余回退 English;无新增散落硬编码
- **验证**:Restore PASS;Modern/Universal/FM/Shell/Classic/Core/Codecs/Console/
  SFX 全部 0 错误;MSIX bundle PASS;UIA 驱动 Modern 对话框 OK/Extract/Cancel
  三态端到端 PASS(产物+SHA-256 往返);CLI 矩阵 .7z×3 级别/.zip/AES/7z 加密/
  分卷/overwrite/错误路径 9 项 PASS;Light 主题截图(`Docs/Screenshots/P1-3/`)
- **已知问题**:XAML island 需一次真实点击激活键盘路由(DesktopWindowXamlSource
  特性,真实交互已验证);Modern MSIX 部署实测仍被 Developer Mode 阻塞
  (`AllowDevelopmentWithoutDevLicense=0x0`,不修改系统策略),与 P1-1/P1-2 相同;
  测试环境 unpackaged 运行需在 exe 目录放置 `resources.pri`(正式打包内置)

## P1-2 Modern File Manager 深度重构
DONE（2026-09-22，安全标签 `p1-2-pre-modern-ui`，基线 `1d491387`，
计划与审计见 `Docs/CUINZIP_UI_PLAN.md` P1-2 节）：
- **导航**：AddressBar 新增 后退 / 前进 按钮（E72B/E72A，Tooltip 本地化），与
  上一级（E74A）并列；Win32 侧 `CPanel::NavigateBack/Forward` 以路径串历史
  （上限 64）经既有 `BindToPathAndRefresh` 管线导航，不改动文件列表核心；
  新增 Alt+Left / Alt+Right 加速键（命令 1073/1074）
- **地址栏视觉简化**：下拉箭头 Subtle 填充去边框；路径保持可编辑/可复制
- **状态栏层级重构**：无选择 `128 items · 245 MB`，有选择 `3 selected · 16.4 MB`，
  右侧追加压缩包名；总大小/项目数随列表刷新一次缓存，选择变化零重复遍历；
  移除焦点项大小/日期
- **Empty State**：面板空列表（或仅剩 ".."）时显示 `This folder is empty` /
  `This archive is empty`（压缩包内外自动区分，resw 本地化），有内容自动隐藏
- **Search**：DEFERRED → `P2 Search`（7-Zip Panel 无可复用过滤能力，增量过滤需
  大规模侵入列表核心；未加假搜索框，审计结论见 UI_PLAN）
- **Settings Modern 化第一轮**：`K7ModernShowSettingsDialog`（8 分类：
  General/Compression/Extraction/FileAssociations/ContextMenu/Appearance/
  Advanced/About）；Appearance 6 开关经回调直写 `CFmSettings` 即时生效；
  其余分类为经典设置页直达入口（`OptionsDialog` 新增 startPage 参数，
  WM_COMMAND 1075-1078）；工具栏 Options 按钮改开 Modern 设置；原设置功能零删除；
  Settings 页为纯代码构建 UI（规避旧版 XAML 编译管道对新 x:Class 页面的绑定
  注册缺陷，详见 UI_PLAN 实现备注）
- **本地化**：新增 `SettingsPage.resw`（41 键）/`AboutPage.resw`（6 键）；
  `Common.resw` 增补导航/空态/状态栏文案；`MainWindowToolBarPage.resw` 增补
  Open Source 按钮；About 页 attribution/版权/按钮接入 resw；新导出
  `K7ModernGetUiString`（PRI 取文本 + 英文兜底）；English + zh-Hans 覆盖，
  其余语言暂回退 English
- **验证**：Restore PASS；NanaZip.Modern / FileManager / Classic / ShellExtension /
  Console / Codecs / Core / MSIX（wapproj，msixbundle 产物）构建全部 0 错误；
  .7z 压缩→解压 SHA-256 往返 PASS；Classic GUI 启动/打开 .7z 截图 PASS
  （`Docs/Screenshots/P1-2/`）
- **已知问题**：本机 Developer Mode 未开启（`AllowDevelopmentWithoutDevLicense=0x0`），
  按约束不修改系统安全策略 → Modern MSIX 部署（Launch/Light/Dark/Empty State
  实测截图）阻塞，与 P1-1 相同；待管理员开启后
  `Add-AppxPackage -Register` 验证；Modern FM 直接运行（unpackaged）的
  `K7ModernInitialize Failed` 限制仍存在（非本阶段回归）

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

# CuinZip Progress

## Current Phase
P1-4 Settings / File Associations / Explorer Context Menu(系统集成体验)

## Status
IN PROGRESS(2026-09-23,安全标签 `p1-4-pre-integration`,基线 `723d047a`)

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
P1-4 Settings / File Associations / Explorer Context Menu 实施中(实现已完成,
验证与构建进行中)

## P1-4 Settings / File Associations / Explorer Context Menu
IN PROGRESS(2026-09-23,安全标签 `p1-4-pre-integration`,基线 `723d047a`,
计划见 `Docs/CUINZIP_UI_PLAN.md` P1-4 节):
- **Settings 全分类真实化**:Modern 设置窗口 8 个分类全部接入真实配置源
  (经宿主回调读写,Modern DLL 不直接访问注册表,无第二套设置):
  General/Appearance → `CFmSettings`(即时生效);Compression →
  `NCompression::CInfo`(压缩对话框同一处,默认格式/级别);Extraction →
  `NExtract::CInfo`(解压对话框同一处,路径模式/覆盖模式/完成后打开文件夹);
  Context Menu → `CContextMenuInfo`(Shell 扩展实时读取的同一注册表);
  File Associations 状态经 `K7ModernQueryFileAssociation` 只读查询
  Windows 官方 UserChoice / Progid,变更默认应用一律走 Windows 设置
  (ms-settings:defaultapps?registeredAUMID=...,不绕过 Windows 11 用户确认)
- **Context Menu 设置真实驱动 Shell**:13 个菜单项开关 + 二级菜单模式 +
  消除重复文件夹 + 打开时解压,全部直写 `CContextMenuInfo`;写入后下一次
  右键即生效(Shell 扩展每次弹出菜单时重新读取)
- **右键菜单双模式**:二级菜单模式(默认)= 单个 CuinZip 子菜单;平铺模式
  = 6 个高频项(Open with CuinZip / Extract Here / Extract to <archive>\ /
  Add to archive... / Add to .7z / Add to .zip)直接出现在一级菜单。
  两种模式经同一组设置开关 + 选择场景(文件/文件夹/压缩包/多选)由
  `GetState(ECS_HIDDEN)` 实时过滤;6 个平铺动词使用全新 CuinZip 自有 CLSID
  (与上游 NanaZip 不重复,理论可共存),manifest 已注册(*/Directory/Drive)
- **本地化**:`SettingsPage.resw` 扩充至 96 键(en + zh-Hans,其余回退
  English);右键菜单新字符串(Open with CuinZip / Hash 组)进入
  Explorer STRINGTABLE(与既有菜单字符串同一本地化机制)
- **实现备注**:Settings 页继续沿用 P1-2 的纯代码构建 UI 模式;新导出
  `K7ModernQueryFileAssociation` / `K7ModernLaunchDefaultAppsSettings`;
  设置回调 ABI 收敛为 `K7_MODERN_SETTINGS_CALLBACKS` 结构(仅 FM 调用,
  无外部消费者)
- **验证(2026-09-23)**:
  - 构建:BuildAllTargets(Debug+Release × x64+arm64 + MSIX bundle)0 错误;
    MSIX 包内 manifest 验证:7 个动词(1 根 + 6 平铺)× 3 ItemType、
    6 个新 CLSID、fileTypeAssociation / comServer / fileExplorerContextMenus
    全部就位;RefreshVersion 自动改动已回滚
  - Settings 页 UIA 驱动 18/18 PASS(Universal exe 临时钩子承载 XAML 环境,
    钩子已验证后移除):8 分类导航、General/Compression/Extraction/Context
    Menu/File Associations 全部开关与组合框均正确写真实配置源(回调结构值
    逐一断言);关联列表实时显示各扩展名当前默认应用
  - Shell 扩展 COM 驱动测试 43/43 PASS(DllGetClassObject 直接创建各动词 +
    真实 IShellItemArray):二级菜单模式(18 子命令/根项显隐)、平铺模式
    (6 动词显隐/标题/图标)、设置开关实时过滤、多选、Invoke 全链路
    (Add to ZIP 多文件 / Add to 7z / Extract Here / Extract to Folder /
    Open),压缩解压 SHA-256 一致
  - 文件关联 9/9 PASS:每用户 Progid 注册后 `.7z`/`.zip` 双击经关联拉起
    CuinZip;查询函数如实反映 Windows 真实状态(UserChoice 受保护,
    不谎报、不抢占);自有扩展名 Progid 场景正确报告 CuinZip 为默认;
    测试注册已清理,不留失效关联
  - CLI 矩阵 PASS:.7z / .zip / .7z(AES+加密文件名)压缩解压 SHA-256 一致
  - Classic UI 冒烟 PASS(标题 CuinZip);MSIX 部署仍受 Developer Mode
    关闭阻塞(不修改系统策略)


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

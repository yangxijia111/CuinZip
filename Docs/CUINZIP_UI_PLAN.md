# CuinZip UI / UX 计划(P1-1)

日期:2026-09-21,基线 `fea047c4`。
约束:不改压缩核心 / ABI / Package Identity / Shell CLSID / LangUtils 匹配键 / Mile.* /
第三方许可;不开发智能解压、安全扫描、预览、AI 功能。

## 现状审计摘要

- **架构**:Modern UI = WinUI 3(XAML,`NanaZip.Modern.dll`,SunValley/Fluent 样式)提供
  工具栏、地址栏、状态栏、About/Sponsor/进度/复制位置/信息对话框,通过
  `K7Modern*` 导出接口嵌在 7-Zip FM 的 Win32 主窗口上;文件列表本体仍为 7-Zip Win32
  Panel。Classic UI = 纯 Win32(7-Zip FM)。
- **主题**:已基于 Mile.Xaml SunValley(Fluent),Light/Dark 跟随系统(K7UserDarkMode)。
- **工具栏**(`MainWindowToolBarPage`):10 个纯图标按钮(Add/Extract/Test/Copy/Move/
  Delete/Info/Options/Benchmark/About,标签折叠,密度大)+ 右侧 Sponsor 按钮。
- **Sponsor 机制**(上游遗留):工具栏按钮 → 重启进程带 `--AcquireSponsorEdition` →
  SponsorPage 调 Microsoft Store 购买 addon `9N9DNPT6D6Z9` → 授权状态缓存于
  `HKCU\Software\NanaZip\SponsorEdition`。CuinZip 无此商业体系,属误导性入口。
- **About**(`AboutPage`):标题/版本/免费软件文案已品牌化(P0-3);但 XAML 版权行仍为
  `© M2-Team and Contributors. All rights reserved.`,底部按钮为
  NanaZip.org / GitHub(→CuinZip) / Cancel,缺 7-Zip 入口。
- **地址栏 / 状态栏**(`AddressBarTemplate` / `StatusBarTemplate`):WinUI 化,可沿用。
- **Classic**:标题/图标已 CuinZip(P0-3);Win32 架构保持兼容模式。

## P1-1 实施(本阶段)

| # | 项 | 优先级 | 内容 |
| --- | --- | --- | --- |
| 1 | Sponsor 入口改造 | 高 | 工具栏按钮改为 `Open Source`(直接打开对话框,不走重启传参/Store);`SponsorPage` 改为 **Open Source Projects** 页(CuinZip GitHub / NanaZip GitHub / 7-Zip 官网 + based-on 说明);删除 Store 购买、授权检查、`--AcquireSponsorEdition` 参数链路(`NanaZip.UI.cpp` 分支一并删);`K7ModernShowSponsorDialog` 导出 ABI 保留(行为变为显示开源页) |
| 2 | 工具栏重组 | 高 | 第一组高频:`Add / Extract / Test / Delete / Info`;第二组:`Options / More`;`Copy / Move / Benchmark / About` 收进 More(FM 主菜单弹出自带;Copy/Move 的 F5/F6 快捷键不变);More 菜单初始化补 Tools/Help 子菜单,保证 Benchmark/About 等仍可达 |
| 3 | AboutPage 升级 | 高 | 文案区:`CuinZip <版本>` + `Based on NanaZip and 7-Zip.` + `© CuinZip Project, M2-Team and Contributors, Igor Pavlov.`;按钮区:`GitHub(CuinZip) / NanaZip.org / 7-Zip.org / Close` |
| 4 | 风格打磨 | 中 | 新增文案统一英文(本地化在后续阶段补 resw);沿用 SunValley 主题,不自造花哨动画 |

## P1-2+ 记录(本阶段不做)

- Modern File Manager 主界面深度重构(顶部 返回/前进/上一级 + 地址栏 + 搜索;
  底部状态栏信息层级)——即 P1-2 主题
- 设置(Win32 Options 对话框)Modern 化与分类整理(常规/压缩/解压/文件关联/右键菜单/
  外观/高级/关于)
- 空压缩包 / 空目录 Empty State;统一确认框与错误提示文案
- Classic Dark Mode / 工具栏间距(Win32 层,延后;Classic 保持兼容模式)
- 新增文案的多语言 resw 补齐

# CuinZip UI / UX 计划(P1-2 实施记录)

日期:2026-09-22,基线 `1d491387`(安全标签 `p1-2-pre-modern-ui`)。

## 主窗口信息架构(目标布局已达成)

```text
┌────────────────────────────────────────────┐
│ Add Extract Test Delete Info │ Open Source │  ← App 级 ToolBar(XAML 岛,P1-1 布局保持)
├────────────────────────────────────────────┤
│ ← → ↑  地址栏(可编辑/可复制)          ▾ │  ← AddressBar 岛(面板级,每面板)
├────────────────────────────────────────────┤
│         文件 / 压缩包列表(Win32 核心)     │  ← 不改动
├────────────────────────────────────────────┤
│ 项目数·大小 │ 压缩包名                     │  ← StatusBar 岗(信息层级重构)
└────────────────────────────────────────────┘
```

- **导航**:`AddressBar` 新增 Back(E72B)/ Forward(E72A)按钮,与 Up(E74A)并列;
  状态属性 `IsBackButtonEnabled / IsForwardButtonEnabled`,事件经面板侧接线。
  Win32 侧实现 `CPanel::NavigateBack/Forward`:以 `_currentFolderPrefix` 路径串为
  历史(深度上限 64),统一经既有 `BindToPathAndRefresh` 管线导航,文件系统与压缩包
  虚拟路径均可用,**未改动文件列表核心**。记录点在 `LoadFullPathAndShow`(刷新不重复
  入栈);新增 Alt+Left / Alt+Right 加速键(`IDR_ACCELERATOR1`,命令 1073/1074),
  Backspace=上一级 等原有快捷键不变。
- **地址栏视觉简化**:下拉箭头改 Subtle 填充、去边框;路径文本保持可编辑、可选中、
  可复制(TextCommandBarFlyout);Light/Dark 均走主题资源(SubtleBrush 已确认存在
  于 Mile.Xaml SunValley 主题)。
- **状态栏层级**:`Refresh_StatusBar` 重写 —— 无选择 `128 items · 245 MB`;有选择
  `3 selected · 16.4 MB`;右侧追加压缩包文件名。总大小/项目数在 `RefreshListCtrl`
  一次性缓存(`_statusItemsTotalSize/_statusItemCount`),选择变化复用缓存,不重复
  遍历大型压缩包;移除原焦点项大小/日期(信息降噪)。控件接口改为
  `TextPrimary/TextArchive`。

## 搜索(审计结论)

7-Zip FM Panel **无可安全复用的列表过滤能力**:列表由 `_folder` COM 绑定 +
`RefreshListCtrl` 全量重建,增量过滤需侵入 `PanelItems/PanelListNotify/Selection`
核心(全量重建 + 虚拟模式 + 选择映射重写)。按阶段约束**不做**,记为 **`P2 Search`**;
未加入任何假搜索框。

## Empty State

面板新增 STATIC 子控件(`_emptyStateWindow`),仅在列表为空(或仅剩 ".." 父项)时
显示,文案 `This folder is empty` / `This archive is empty`(resw 本地化,压缩包内
外自动区分);有内容时自动隐藏,不遮挡文件列表;配色走系统 COLOR_WINDOW/COLOR_GRAYTEXT,
Light/Dark 均可读。位置与列表区同步(`ChangeWindowSize`)。

## Settings 第一轮 Modern 化(渐进迁移)

新增 `K7ModernShowSettingsDialog`(NanaZip.Modern.dll):
- 8 分类:General / Compression / Extraction / File Associations / Context Menu /
  Appearance / Advanced / About;
- **Appearance 直接 Modern 化**:6 个开关(ShowDots/RealFileIcons/FullRow/Grid/
  SingleClick/AltSelection)经回调(`K7_MODERN_SETTINGS_LOAD/APPLY_CALLBACK`,由
  FM 侧以 `CFmSettings` 读写注册表)即时生效并刷新列表 —— 真实设置源仍是
  `CFmSettings`,无双重注册表实现;
- 其余分类提供**经典设置页直达入口**(WM_COMMAND 1075-1078 →
  `OptionsDialog(hwnd,hInstance,startPage)`,`MyPropertySheet` 新增 startPage 参数),
  原有设置功能零删除;
- File Associations 附带直接打开 `ms-settings:defaultapps`;
- Compression 如实说明当前经由压缩对话框配置(后续 P1-3 扩展);
- About 分类内嵌打开 About 对话框入口;
- 工具栏 Options 按钮(`IDM_OPTIONS`)改开 Modern 设置窗口。
- **实现备注**:Settings 页为纯代码构建 UI(不走 x:Class XAML 绑定)——项目所用
  旧版 XAML 编译管道(V8.2 CompileXaml)对新增 x:Class 页面确定性跳过绑定注册
  (perfXC_SearchIxmpAndBindable 后不产出 XamlTypeInfo include),导致链接失败;
  纯代码构建 + GetUiString(PRI) 文本可完全等效并规避。此坑已记录,后续新页面
  优先沿用本模式或扩展已注册页面。

## 文案与本地化(P1-2)

- 新增 resw:`SettingsPage.resw`(41 条)、`AboutPage.resw`(6 条)(en + zh-Hans);
- `Common.resw` 增补:导航 Tooltip ×3、Empty State ×2、状态栏格式 ×3(en + zh-Hans);
- `MainWindowToolBarPage.resw` 增补 `OpenSourceButton.Content`(P1-1 硬编码迁移);
- AboutPage 的 attribution/版权行/四按钮接入 resw(x:Uid + 英文兜底);
- 新导出 `K7ModernGetUiString(name, fallback)`:Win32 面板侧(空态/状态栏)与
  XAML 模板(AddressBar Tooltip)统一经 PRI 取文本,英文兜底,其余语言暂回退
  English;`UiStrings.h/cpp` 提供带缓存的内部实现。

## 部署与验证(本阶段)

- 本机 Developer Mode 处于关闭状态(`AllowDevelopmentWithoutDevLicense=0x0`),
  按约束**不修改系统安全策略** → Modern MSIX 部署(Launch/Light/Dark 实测)仍被
  阻塞,与 P1-1 相同;已完成:Restore PASS、构建 0 错误、压缩/解压/SHA-256 往返
  PASS、Classic GUI 回归截图 PASS。
- 待管理员开启 Developer Mode 后执行 `Add-AppxPackage -Register` 部署验证
  (见 CUINZIP_PROGRESS.md Known Issues)。

## 永不改(红线)

7-Zip ABI GUID(`23170F69-…`)、Core/Codecs 算法、Shell CLSID、Package Identity、
LangUtils `"NanaZip"` 匹配键、`Mile.*`、第三方许可证、内部二进制契约。

# CuinZip UI / UX 计划(P1-3 实施记录)

日期:2026-09-22,基线 `dfe3ae8c`(安全标签 `p1-3-pre-dialogs`)。
范围:压缩(Add to Archive)/ 解压(Extract)对话框 Modern 化;进度链经审计
已由上游 Modern 化(见下),不重写。

## 现状审计结论(P1-3)

- FM 的 Add/Extract 经 `CompressCall.cpp` 拉起 `NanaZip.Universal.Windows.exe`
  子进程(`a/-ad` 或 `x/-ad` 命令行 + FileMapping 传文件清单),配置对话框
  (`CCompressDialog`/`CExtractDialog`,Win32 rc 模板)与 GUI 进度都在子进程内。
- **进度链已经 Modern 化**(NanaZip 上游):`CProgressDialog::Create` 在
  `K7ModernAvailable()` 时走 `K7ModernShowProgressWindow`(Mile.Xaml 窗口 +
  `ProgressPage`);百分比/当前文件/总量/已处理大小/速度/已用时间/剩余时间
  (仅在总量已知时计算)/压缩比齐备;Pause(底层真实支持,`CProgressSync`
  暂停)/Background(优先级)/Cancel(二段确认 + `Set_Stopped` → E_ABORT)齐备;
  结束态:错误/消息经 `K7ModernShowInformationDialog` 展示,成功沿用 7-Zip
  静默语义。P1-3 未改动该链路。
- 压缩/解压对话框逻辑深耦合 HWND(格式/方法/字典联动、内存估算、注册表
  读写、OnOK 校验,`CompressDialog.cpp` 约 4300 行)。

## 对话框镜像引擎(核心设计)

**原则:Modern 外壳 → 复用原对话框全部业务逻辑,零重写。**

- 原 rc 对话框以**隐藏的无模式窗口**创建(`CreateDialogParamW` +
  `ModernMirrorDialogProc`,复刻 NWindows `DialogProcedure` 的装配分发),
  继续作为数据引擎:`OnInitDialog` 读注册表/填组合框、`CBN_SELCHANGE`/
  `BN_CLICKED` 联动、`OnOK` 全部校验与回写都走原代码;
- `K7ModernShowCompressDialog` / `K7ModernShowExtractDialog`(新导出,
  `NanaZip.Modern.def` + `NanaZip.Modern.Wrapper.cpp` 动态转发,宿主 EXE
  保持对 DLL 无静态导入依赖)显示 Modern 纯代码页面
  (`CompressDialogPage`/`ExtractDialogPage`,规避 P1-2 发现的 XAML 管道坑);
- 页面经通用 `K7_DIALOG_MIRROR_ENGINE` 回调表(读组合框/文本/复选、
  设选中/文本/勾选并触发原通知、模拟按钮点击、`PressOK`)读写引擎;
  每次交互后全量同步控件状态(含启用/可见——格式切换时加密区、加密文件名、
  SFX、第二密码框的显隐全部跟随原逻辑);
- 可编辑组合框文本(UWP 无 TextChanged 事件)经 `PushEditableTexts`
  在任何触发原逻辑的操作前统一写回;引擎操作后 `DrainPostedMessages`
  排空对话框 Post 的后续消息(如 `k_Message_ArcChanged`),保证同步读到终态;
- OK:页面 `PressOK` → 原 `OnOK` 全量校验(密码 ASCII/长度、内存上限、
  路径、分卷确认……);全部通过时原代码置位 `Modern_OK_Completed`
  (在 `CModalDialog::OnOK()` 前插入的标志),页面发
  `K7_DIALOG_MIRROR_RESULT_OK` 并关闭;校验失败原逻辑弹错误提示,
  页面保持打开并重新同步。Cancel/Esc → `RESULT_CANCEL` → `E_ABORT` 语义;
- **回退**:任何 `K7Modern*` 不可用(unpackaged XAML 初始化失败等)时
  `Create()` 走原模态 rc 对话框,经典行为零改动;SFX 构建
  (`NanaZip.Core.Sfx`,定义 `Z7_SFX` 并复用 `ExtractDialog.cpp`)用
  `#ifndef Z7_SFX` 编译剔除镜像接入。

## 页面信息架构

- **CompressDialogPage**(560×640 逻辑):`Archive`(名称+目录显示+浏览+格式+
  更新模式+路径模式+SFX)/ `Compression`(级别/方法/字典/字大小/固实/线程/
  内存上限 + 压缩/解压内存实时数值)/ `Encryption`(密码×2 + 显示密码 +
  加密文件名 + 加密方法)/ `Advanced`(默认折叠:分卷/参数/共享/压缩后删除/
  Options 经典时间戳-NTFS 子对话框 + 选项摘要);按钮 Cancel / OK(强调色)。
- **ExtractDialogPage**(500×480 逻辑):`Destination`(路径+浏览+拆分名+
  路径模式+覆盖模式)/ `Options`(ElimDup/NtSecurity/打开目标文件夹×2)/
  `Password`;按钮 Cancel / **Extract**(强调色)。
- 字段标签直接读自隐藏原对话框(沿用 7-Zip Lang 本地化体系,含清理
  `&`/`(&X)` 加速键);新增文案(分区标题/按钮/文件夹名)进 resw;
- 密码:PasswordBox + 明文 TextBox 叠放切换(镜像 `UpdatePasswordControl`
  语义,显示时隐藏第二输入框);不落日志、不缓存;
- 键盘:Enter=OK / Esc=Cancel,经 PreviewKeyDown + KeyDown + 键盘加速器 +
  控件级 KeyDown 四层保障(可编辑组合框会吞 Enter 的 UWP 特性);
- 浏览/Options 子对话框期间 Modern 窗口自我禁用,保持模态语义;
- Tab 顺序=声明顺序;AutomationProperties 全字段补齐。

## 文案与本地化(P1-3)

- 新增 `CompressDialogPage.resw`(6 键)/ `ExtractDialogPage.resw`(7 键),
  en + zh-Hans,其余语言英文兜底;经 `GetUiString("Page/Key")` 读取;
- 未新增任何散落硬编码 UI 文案。

## 验证(P1-3)

- 构建:Restore PASS;NanaZip.Modern / Universal.Windows / Modern FM /
  ShellExtension / Classic / Core / Codecs / Universal.Console /
  Core.Sfx(Z7_SFX 保护)全部 0 错误;MSIX(msixbundle)构建 PASS;
- 功能(unpackaged + 目录放置 `resources.pri` 使 XAML island 可用):
  Modern 压缩对话框 OK → 压缩包生成、进程 exit 0(UIA 驱动);
  Modern 解压对话框 Extract → 目录结构+文件完整还原;Cancel → exit
  E_ABORT 且无产物;Esc 关闭;
- 往返矩阵(Console,同核心管线):`.7z` store/normal/ultra、`.zip` fast、
  `.zip` AES 加密、`.7z` 密码+加密文件名、`.7z` 分卷(-v100k)、
  overwrite `-y` 重解压、不存在压缩包非零退出 —— 全部 PASS,文件 SHA-256
  一致(`Docs/Screenshots/P1-3/` Light 主题截图);
- 键盘备注:XAML island 需一次真实点击激活键盘路由(DesktopWindowXamlSource
  特性);自动注入按键在 island 未激活时不达,四层键盘保障代码已就位,
  真实交互(先点击窗口)验证 Enter=OK / Esc=Cancel 成立。
- Modern MSIX 部署实测仍被 Developer Mode 关闭阻塞(与 P1-1/P1-2 相同,
  不修改系统策略)。

## P1-4+(记录)

- Settings 的 Compression/Extraction 分类接入镜像引擎能力(当前为经典页
  直达入口 + 说明,配置源仍唯一:NCompression/NExtract 注册表,经
  压缩/解压对话框 OnOK 写入,无第二套配置);
- Advanced 分区内的 Options(时间戳/NTFS)子对话框 Modern 化;
- Classic 对话框(非 Modern 回退路径)的深色/视觉打磨;
- P2 Search。


# CuinZip UI / UX 计划(P1-4 实施记录)

日期:2026-09-23,基线 `723d047a`(安全标签 `p1-4-pre-integration`)。
范围:Settings 全分类真实化 / 文件关联状态管理 / Explorer 右键菜单
(Windows 11 体验)。不修改压缩核心。

## 设计原则

1. **单一配置源**:所有 Modern 设置经宿主(FileManager)回调读写既有真实
   配置源(CFmSettings / CContextMenuInfo / NCompression::CInfo /
   NExtract::CInfo),Modern DLL 不直接访问注册表,不存在第二套设置;
   Shell 扩展与压缩/解压对话框读取的是同一处注册表。
2. **官方机制优先**:文件关联的默认应用变更一律走 Windows 官方路径
   (ms-settings:defaultapps?registeredAUMID=…),只读查询 UserChoice /
   Progid 状态,不绕过 Windows 11 用户确认机制,不做危险注册表抢占。
3. **不删除原有功能**:经典设置页全部保留为直达入口(WM_COMMAND
   1075-1078),复杂高级参数继续跳转经典页。
4. **CLSID 自有**:新平铺动词使用全新生成的 CuinZip GUID,与上游
   NanaZip 不重复,保证 CuinZip 与 NanaZip 理论可共存。

## Settings 信息架构(P1-4 后)

| 分类 | 真实配置源 | 生效时机 |
| --- | --- | --- |
| General | CFmSettings(系统菜单/四类历史) | 历史类实时;系统菜单对新窗口生效 |
| Compression | NCompression::CInfo(默认格式/级别) | 下一次压缩对话框 |
| Extraction | NExtract::CInfo(路径模式/覆盖模式/完成后打开文件夹) | 下一次解压对话框 |
| File Associations | 只读查询(Windows UserChoice) + CContextMenuInfo.ExtractOnOpen | 状态实时;默认应用经 Windows 设置 |
| Context Menu | CContextMenuInfo(13 项开关 + 二级菜单模式 + 消除重复 + 打开时解压) | 下一次右键 |
| Appearance | CFmSettings(6 开关) | 即时 |
| Advanced | 经典页直达 | - |
| About | About 对话框直达 | - |

## 右键菜单双模式(Windows 11)

- **二级菜单模式(默认)**:单个 "CuinZip" 根项(ECF_HASSUBCOMMANDS),
  子命令按选择场景过滤(Open 仅单压缩包;Extract 系仅当选中含压缩包;
  Compress 系任意选择可用;Hash 组)。
- **平铺模式**:6 个高频动词直接出现在一级菜单,各自带图标与本地化标题
  ("Extract to <name>\" / "Add to <name>.7z" 等动态名实时计算);
  Extract/Open 类在不含压缩包的选择下经 GetState 返回 ECS_HIDDEN。
- 两种模式由同一 `CascadedMenu` 设置互换:平铺模式隐藏根项,二级模式
  隐藏全部平铺动词;Explorer 不显示空壳菜单。
- 平铺动词 CLSID(Preview 与 Release 构建共用,均为 CuinZip 自有):
  Open 9EE110B9-… / ExtractHere 3FCAFA2A-… / ExtractTo EEEA627E-… /
  Compress 40E45AB6-… / CompressTo7z AD18A991-… / CompressToZip 95BA5FBD-…;
  二级菜单根项沿用 P0-3 的 788F8FA7-…。
- manifest 在 */Directory/Drive 三个 ItemType 各注册 7 个动词
  (0000 根项 + 0001-0006 平铺),Verb Id 唯一,不出现重复菜单。

## 本地化(P1-4)

- `SettingsPage.resw` 96 键(en + zh-Hans,其余语言回退 English),
  覆盖全部新增分类文案/开关/组合框项/关联状态;
- 右键菜单新字符串(Open with CuinZip、Hash 组五项)进入 Explorer
  STRINGTABLE(English 基线 + Lang 文件覆盖机制,与既有菜单字符串一致);
- 未新增散落硬编码 UI 文案。

## 验证(P1-4,2026-09-23 完成)

- 构建:BuildAllTargets(Debug+Release × x64+arm64 + MSIX bundle)0 错误;
  MSIX 包内 manifest 验证 7 动词 × 3 ItemType + 6 新 CLSID + 关联声明齐全;
- Settings 页 UIA 驱动 18/18 PASS(经 Universal exe 临时钩子承载 XAML
  环境,验证后钩子已移除):八分类导航 + 各设置组写真实配置源逐一断言;
- Shell 扩展 COM 驱动测试 43/43 PASS(`p14_shelltest`,DllGetClassObject
  创建各动词 + 真实 IShellItemArray):双模式显隐、设置开关过滤、多选、
  Invoke 全链路(Add to ZIP 多文件 / Add to 7z / Extract Here /
  Extract to Folder / Open)与 SHA-256 往返;
- 文件关联 9/9 PASS:每用户 Progid 注册后 .7z/.zip 双击拉起 CuinZip;
  查询如实反映 UserChoice 真实状态(不谎报/不抢占);测试注册已清理;
- CLI 矩阵 PASS:.7z / .zip / .7z(AES+加密文件名)SHA-256 一致;
- Classic UI 冒烟 PASS。

## 已知限制

- Win32 FM / Shell 扩展的菜单字符串本地化经 Lang 文件机制,仓库不随包
  发布 Lang 文件(与上游一致),zh-Hans 菜单名需用户自备 Lang 文件;
  Modern 设置 UI 的 zh-Hans 经 resw 完整覆盖。
- Modern MSIX 部署实测仍被 Developer Mode 关闭阻塞(不修改系统策略),
  与 P1-1/P1-2/P1-3 相同。

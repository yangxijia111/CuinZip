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

## 永不改(红线)

7-Zip ABI GUID(`23170F69-…`)、Core/Codecs 算法、Shell CLSID、Package Identity、
LangUtils `"NanaZip"` 匹配键、`Mile.*`、第三方许可证、内部二进制契约。

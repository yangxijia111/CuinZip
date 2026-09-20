# CuinZip 许可证审计笔记（License Notes）

> 阶段：P0-1（Fork 基线建立）
> 日期：2026-09-20
> 基线提交：`1f9d7d93`（upstream/main）
> 本文档为 CuinZip 项目内部的许可证记录，不修改、不替代仓库根目录的原始 `License.md`。

---

## 1. 许可证构成总览

NanaZip 是多许可证混合项目，**不能假设整个仓库只有一种许可证**。按根目录 `License.md` 的官方划分：

| 范围 | 许可证 | 版权人 |
|---|---|---|
| NanaZip 自有源码（除下述第三方与 7-Zip 目录） | MIT | M2-Team and Contributors |
| `Assets/` 中的文件关联图标 | **CC BY-ND 4.0**（禁止演绎） | Shomnipotence（授权给 NanaZip 项目） |
| 7-Zip 及其衍生源码（见第 3 节的 5 个目录） | 7-Zip License（主体为 GNU LGPL，部分含 unRAR 限制，LZFSE 部分为 BSD 3-clause） | Igor Pavlov（1999-2021）等 |
| 第三方库源码（见第 4 节） | 各自原始许可证 | 各自版权人 |

## 2. NanaZip 原项目许可证（MIT）

- 适用于：除第三方库、7-Zip 衍生代码、`Assets/` 图标之外的全部 NanaZip 自有代码。
- 版权声明形式：`Copyright (c) M2-Team and Contributors. All rights reserved.`
- MIT 允许 CuinZip 作为衍生项目修改、再分发（包括商业分发），条件是**保留原版权声明与许可文本**。
- CuinZip 因 MIT 义务必须永久保留 `License.md` 中 MIT 部分的完整文本与 M2-Team 版权声明（即使后续做 Rebrand 改名，也只能"补充"CuinZip 自己的声明，不能"替换"原作者声明）。

## 3. 7-Zip 相关许可证（ LGPL + unRAR 限制 + BSD 3-clause）

### 3.1 覆盖目录（5 处，均在仓库内 vendored，无 submodule）

1. `NanaZip.Core/SevenZip/`
2. `NanaZip.Core/Extensions/ZSCodecs/`
3. `NanaZip.Universal/SevenZip/`
4. `NanaZip.UI.Classic/SevenZip/`
5. `NanaZip.UI.Modern/SevenZip/`

### 3.2 许可证要点

- **主体**：GNU LGPL（部分文件 LGPL + unRAR 限制，LZFSE 解压代码为 BSD 3-clause，源自 Apple Inc. 2015-2016 的 LZFSE 库）。
- **二进制再分发义务**：LGPL 要求二进制形式分发时必须复现相关许可证信息（即随 CuinZip 发布包携带 7-Zip License 文本——保留根目录 `License.md` 即可满足）。
- **unRAR 限制（重点合规红线）**：RAR 解压引擎基于 Alexander Roshal 的 unRAR 源码。该许可**禁止**利用 unRAR 源码重新实现 RAR（WinRAR 兼容）的**压缩**算法；分发修改后的 unRAR 源码或将其嵌入其他软件是允许的，但**必须在文档和源码注释中明确声明**"该代码不得用于开发 RAR（WinRAR）兼容的压缩软件"。
  - 对 CuinZip 的具体约束：CuinZip 永远不得基于仓库内 unRAR 衍生代码实现"创建 RAR 压缩包"功能；现有文档声明必须原样保留。
- **上游同步标记规范**（来自 `CONTRIBUTING.md`）：对继承自 7-Zip 主线的代码的修改必须使用 `// **************** NanaZip Modification Start ****************` / `End` 成对注释标记，以便未来从 7-Zip 主线同步。CuinZip 继续沿用此规范（沿用 NanaZip 字样标记以保持与 upstream 的 diff 可比性；CuinZip 自己的新增修改可用同格式 `CuinZip Modification Start/End` 标记，但**不得删除**既有的 NanaZip 标记）。
- **7-Zip 分支来源**：7-zip.org 官方、`myfreeer/7z-build-nsis`、`mcmilk/7-Zip-zstd`（ZS 系列编解码来源）。

## 4. 第三方组件许可证清单

第三方库分布在 `NanaZip.Codecs/` 下的子目录（另有少量散置代码）。逐库核实结果：

| 库 | 目录 | 许可证 | 已验证证据 |
|---|---|---|---|
| Brotli | `NanaZip.Codecs/Brotli/` | MIT | 源码头："Copyright 2016 Google Inc. … Distributed under MIT license" |
| xxHash | `NanaZip.Codecs/xxHash/` | BSD 2-Clause | `xxhash.h`："Copyright (C) 2012-2023 Yann Collet, BSD 2-Clause License" |
| LZ4 | `NanaZip.Codecs/LZ4/` | BSD 2-Clause | `lz4.c`："Copyright (C) 2011-2023, Yann Collet" |
| LZ5 | `NanaZip.Codecs/LZ5/` | BSD 2-Clause | `lz5*.c` 头部明确 BSD 2-Clause |
| Lizard | `NanaZip.Codecs/Lizard/` | BSD 2-Clause | `lizard_decompress.c` 头部 BSD 2-Clause |
| Zstandard (zstd) | `NanaZip.Codecs/Zstandard/` | BSD 3-Clause 或 GPLv2 双许可 | `zstd.h`："Copyright (c) Meta Platforms… licensed under both the BSD-style license … and the GPLv2"（CuinZip 分发时选择 BSD 3-Clause 分支即可，无需 GPL 化） |
| ZSTDMT | `NanaZip.Codecs/ZSTDMT/` | BSD 系（zstd 同款双许可）+ Tino Reichardt 版权 | 源码头 |
| Fast LZMA2 | `NanaZip.Codecs/FastLZMA2/` | BSD 系 / GPLv2 双许可（zstd 风格声明） | 源码头："Copyright (c) 2018 Conor McCarthy" |
| BLAKE3 | `NanaZip.Codecs/BLAKE3/` | 上游为 CC0-1.0 或 Apache-2.0 双许可 | **注意：仓库内 vendored 文件未携带许可头**，以 BLAKE3 上游项目声明为准 |
| GmSSL（SM3 等国密哈希） | `NanaZip.Codecs/GmSSL/` | Apache License 2.0 | 源码头："Copyright 2014-2023 The GmSSL Project … Apache License, Version 2.0" |
| RHash（AICH/ED2K/GOST/Tiger/Whirlpool 等哈希源） | `NanaZip.Codecs/RHash/` | ISC 风格宽松许可（上游 RHash 为 MIT 系） | `has160.c` 等："Permission to use, copy, modify, and/or distribute … with or without fee" |
| littlefs | `NanaZip.Codecs/LittleFS/` | BSD 3-Clause | 源码头："Copyright (c) 2022, The littlefs authors / (c) 2017, Arm Limited" |
| FreeBSD（UFS 文件系统头） | `NanaZip.Codecs/FreeBSD/` | FreeBSD 许可（BSD 2-Clause 系） | 上游 FreeBSD 项目 |
| C++/WinRT | NuGet 包 | MIT | 以 NuGet 包内声明为准 |
| Mile 系列库（Mile.Detours / Mile.Json / Mile.Xaml / Mile.Project.Configurations / Mile.Windows.Helpers / Mile.Windows.Internal / Mile.Windows.UniCrt / Mile.Mobility） | NuGet 包 + 少量 vendored 代码（如 `NanaZip.Codecs/Mile.Helpers.Portable.Base.Unstaged.cpp`） | 各自原许可（多为宽松许可） | 以各包/仓库内 LICENSE 为准 |

另：`NanaZip.Codecs/` 内大量 `NanaZip.Codecs.Hash.*.cpp` / `NanaZip.Codecs.Archive.*.cpp` 为 **NanaZip 自有代码（MIT）** 对上述第三方算法的 7-Zip 接口封装，注意区分。

## 5. 必须永久保留的版权声明与文件

1. 根目录 `License.md` —— 整体保留，不得删除、不得裁剪（内含 MIT、7-Zip License、CC BY-ND 4.0 三份全文及第三方清单）。
2. 5 个 `SevenZip` 目录与 `ZSCodecs` 内 7-Zip 源码的文件头版权声明（Igor Pavlov 等）。
3. 各第三方库源文件头部的版权与许可证声明（Brotli/xxHash/LZ4/zstd/GmSSL/RHash/littlefs 等）。
4. `Assets/` 图标的作者署名（Shomnipotence，见第 6 节）。
5. NanaZip 自有代码中的 M2-Team 版权声明（MIT 义务）。
6. `CONTRIBUTING.md` 中引用的开源哲学段落属于 `License.md` 的一部分，同样保留。

## 6. 不能直接修改或以修改版形式再发布的资产（重点）

- **`Assets/` 下的文件关联图标与 NanaZip 品牌图标（CC BY-ND 4.0）**：
  - ND = NoDerivatives。CuinZip **不得修改这些图标再分发**（例如"把 NanaZip 图标改个颜色/字母当 CuinZip 图标"是**不被允许**的）。
  - 允许的做法：a) 原样保留并按要求署名（但品牌上不合适）；b) **另请设计师/自行创作全新的 CuinZip 图标**替换之（推荐，且 Rebrand 阶段必须如此）。
  - 原样再分发时必须附带署名（Shomnipotence）、许可声明与链接。
  - 涉及范围：`Assets/NanaZip*.png/ico`、`Assets/OriginalAssets*/**`、`Assets/PackageAssets/**`、`Assets/PreviewPackageAssets/**`（后两者为母版生成物，继承同一许可）。
- **工具栏位图**（`NanaZip.UI.Classic/Assets/Toolbar/` 等）：属 7-Zip/上游资产，修改不受 CC BY-ND 约束但需遵守其原许可并保留来源。
- **unRAR 衍生代码**：可分发但不得用于开发 RAR 兼容压缩器（见 3.2）。
- **`Mile.` 前缀的文件与文件夹**：`CONTRIBUTING.md` 明确禁止未经 Kenji Mouri 许可修改（因跨项目共享）。CuinZip 同样遵守——遇到需要改的场景，先在本地包装层（K7Base/K7User/自有代码）适配，不动 Mile 文件本体。

## 7. CuinZip 新增代码的许可证建议

- **建议：MIT**。理由：
  1. 与上游 NanaZip 自有代码许可一致，未来从 upstream 合并代码时零许可证摩擦；
  2. 与仓库内全部第三方许可（MIT/BSD/Apache/LGPL 作为库使用）兼容；
  3. 若采用 copyleft（如 GPL）会给"继续同步 7-Zip/NanaZip 上游（LGPL/MTI）"制造不必要的合规负担。
- 实施方式（留待 Rebrand 阶段执行，本阶段不动）：
  - 根目录 `License.md` 原样保留；
  - 新增 `Docs/CUINZIP_LICENSE.md`：声明 CuinZip 新增/修改部分为 MIT，版权 `Copyright (c) 2026 CuinZip Contributors`，并声明衍生自 M2Team/NanaZip（保留其 MIT 声明）、包含 7-Zip（Igor Pavlov）LGPL 代码与各第三方组件；
  - 若 CuinZip 的图标/美术由本项目创作，采用 CC BY 4.0 或更宽松许可（避免重蹈 ND 限制）。
- 商标说明：NanaZip 名称与 Logo 属名称/商标范畴而非代码许可；衍生项目改名（CuinZip）并替换自有图标是正当做法，但需在 About/文档中保留"基于 NanaZip（M2Team）与 7-Zip（Igor Pavlov）"的溯源声明。

## 8. 合规检查清单（供每次发布前自查）

- [ ] 根目录 `License.md` 完整存在且未被修改
- [ ] 发布包（MSIX/ZIP/安装器）内随附许可证文本
- [ ] `Assets/` 内无对 CC BY-ND 图标的修改版（Rebrand 后应为全新创作的 CuinZip 图标）
- [ ] 未删除任何源文件头版权声明
- [ ] 未新增基于 unRAR 代码的 RAR 压缩功能
- [ ] 未修改 `Mile.` 前缀文件
- [ ] About/文档含上游溯源声明（NanaZip、7-Zip、7-Zip-zstd、myfreeer 分支）

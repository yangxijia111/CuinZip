# ![CuinZip](Assets/CuinZip.png) CuinZip

CuinZip is an open source file archiver intended for the modern Windows
experience. It is a long-term derivative project of
[NanaZip](https://github.com/M2Team/NanaZip), which itself is forked from the
source code of the well-known open source file archiver
[7-Zip](https://www.7-zip.org).

**CuinZip is based on NanaZip and 7-Zip.** All credit for the original
architecture, compression implementations, and modern Windows integration
belongs to the NanaZip team (M2-Team, led by Kenji Mouri) and Igor Pavlov.

## Project Status

CuinZip is in the P0 bootstrap phase: establishing a verified build baseline
and product identity on top of the upstream NanaZip source tree, before
starting its own UI/UX evolution in P1.

- Rebrand completed (P0-3): independent MSIX Package Identity, Shell Extension
  CLSID, installer identity, and original placeholder iconography.
- See [Docs/CUINZIP_PROGRESS.md](Docs/CUINZIP_PROGRESS.md) for the phase plan
  and [Docs/CUINZIP_REBRAND.md](Docs/CUINZIP_REBRAND.md) for branding details.

## Features

CuinZip inherits the full feature set of its upstream baseline, including:

- All features from 7-Zip 26.03, [7-Zip ZS] and [7-Zip NSIS].
- Modern Windows 10/11 experience: MSIX packaging, dark mode, Mica effect,
  File Explorer context menu, Per-Monitor DPI awareness, and i18n support.
- Additional hash algorithms and codecs (Brotli, LZ4, Lizard, Zstandard, and
  more), plus additional security mitigations.
- The 7-Zip execution aliases (`7z.exe`, `7zFM.exe`, `7zG.exe`) and the `K7`
  alias family are preserved for compatibility.

Refer to the upstream
[NanaZip documentation](https://github.com/M2Team/NanaZip) for the complete
feature list of the inherited baseline.

## Building from Source

Requirements and the verified procedure are documented in
[Docs/CUINZIP_BASELINE.md](Docs/CUINZIP_BASELINE.md). In short: use Visual
Studio 2022 Build Tools with the UWP/MSIX components, map the repository to a
drive root with `subst` (non-ASCII paths break MIDL/mdmerge), run
`RestoreNuGetPackages.cmd`, then `BuildAllTargets.cmd`.

## License

CuinZip is distributed under the terms in [License.md](License.md). The
upstream NanaZip and 7-Zip licenses and copyright notices are preserved
unchanged. The CuinZip placeholder icons are original works created for this
project and do not derive from the NanaZip icons (which are licensed
CC BY-ND 4.0 and are not modified or redistributed by CuinZip).

[7-Zip ZS]: https://github.com/mcmilk/7-Zip-zstd
[7-Zip NSIS]: https://github.com/myfreeer/7z-build-nsis

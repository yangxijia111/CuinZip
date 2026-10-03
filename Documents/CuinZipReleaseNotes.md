# CuinZip Preview Release Notes

**CuinZip 0.1.0, version 0.1.0-preview.2 (0.1.0.1)** — October 3, 2026

Preview 2 is a usability-focused update on top of the first public preview.
Everything below was verified on the release build.

> **Unsigned Preview Build.** This build is not digitally signed. Windows
> SmartScreen may warn when running the installer or the portable executable.
> Please verify the downloaded files against the SHA-256 checksums published
> with the release. No signing certificate or private key is distributed.

## What's New in Preview 2

- **New Home page**: CuinZip now opens on a dedicated Home instead of a bare
  file view, with four quick-action cards (Open Archive, Create Archive,
  Extract, and Test) as the starting point for every task.
- **Recent Archives**: archives you open are remembered on the Home page, so
  you can get back to your work in one click.
- **Drag & Drop**: drop archives onto the Home page to open them directly, or
  drop regular files and folders to jump straight into the Create Archive
  dialog with everything preloaded. Unsupported drops are ignored safely.
- **More intuitive toolbar**: the toolbar is now Home / Add / Extract / Test /
  Delete / More, plus Settings.
- **Context-aware actions**: Extract and Test — meaningless inside a plain
  folder — are hidden there and only appear when a toolbar action would
  actually apply, reducing clutter without losing power.
- **Simplified compression & extraction UI**: the dialogs lead with a clean
  Basic view; every original 7-Zip option is still available under
  "More options", and values are preserved when switching between the two.
- **Settings usability**: the Settings page keeps its title, restores
  Basic / More options without losing values, and shows a correct Chinese
  interface alongside English.
- **Home ↔ File Manager navigation**: a closed loop — browse in, browse out,
  with state preserved.

For the full list of inherited capabilities, see the Preview 1 notes below.

## Known Issues (Preview 2)

- The build is unsigned; SmartScreen warnings are expected.
- The MSIX bundle is unsigned as well, so sideloading it requires Developer
  Mode. Without it, use the portable ZIP or the per-user installer. The
  Explorer context menu is only available through the MSIX package.
- **Portable / installer mode**: the Modern file manager and dialogs run
  unpackaged via `NanaZip.Modern.FileManager.exe`; integration features
  (context menu, file associations, tiles) require the MSIX package.
- This is a preview: settings layouts, menu structures, and internal APIs may
  change in future builds.

---

## v0.1.0-preview.1 (0.1.0.0) — September 29, 2026

**CuinZip 0.1.0, version 0.1.0-preview.1 (0.1.0.0)**

This is the first public preview of CuinZip, a modern Windows file archiver
based on [NanaZip](https://github.com/M2Team/NanaZip) and
[7-Zip](https://www.7-zip.org).

> **Unsigned Preview Build.** This build is not digitally signed. Windows
> SmartScreen may warn when running the installer or the portable executable.
> Please verify the downloaded files against the SHA-256 checksums published
> with the release. No signing certificate or private key is distributed.

## What's New

- **Modern UI**: redesigned toolbar (Add / Extract / Test / Delete / Info +
  Options / More), address bar with back/forward navigation, status bar with
  selection summary, and empty states.
- **Modern compression and extraction dialogs** built on a mirror engine over
  the classic 7-Zip dialogs: every original option is preserved (formats,
  levels, dictionaries, CPU threads, memory limit, split volumes, SFX,
  passwords, encryption of file names, timestamps, path modes, overwrite
  modes).
- **Windows 11 Explorer context menu**: a cascaded CuinZip submenu by default,
  or six flat verbs (Open with CuinZip / Extract Here / Extract to
  <archive>\ / Add to archive… / Add to .7z / Add to .zip). All menu items are
  configurable in Settings and apply on the next right-click. Delivered
  through the MSIX package.
- **Settings**: eight categories (General, Compression, Extraction, File
  Associations, Context Menu, Appearance, Advanced, About), all wired to the
  real configuration sources used by the dialogs and the shell extension.
- **File associations**: CuinZip reports the current default application for
  each archive extension and opens the official Windows Default Apps settings
  page when you want to change a default. It never overrides your choice.
- **Original CuinZip icon set**: application, SFX, and archive file icons.
- **Supported formats**: everything from the NanaZip / 7-Zip baseline,
  including 7z, zip, gzip, bzip2, tar, xz, zstd, and the additional codecs
  shipped by NanaZip (Brotli, LZ4, Lizard, LZ5, and more).

## Known Issues

- The build is unsigned; SmartScreen warnings are expected.
- The MSIX bundle is unsigned as well, so sideloading it requires Developer
  Mode. Without it, use the portable ZIP or the per-user installer. The
  Explorer context menu is only available through the MSIX package.
- **Portable / installer mode**: the Modern file manager and dialogs run
  unpackaged via `NanaZip.Modern.FileManager.exe`; integration features
  (context menu, file associations, tiles) require the MSIX package.
- This is a preview: settings layouts, menu structures, and internal APIs may
  change in future builds.

## Attribution

CuinZip is a derivative of NanaZip © M2-Team and Contributors, which is a
fork of 7-Zip © Igor Pavlov. All upstream licenses and copyright notices are
preserved. The CuinZip icons are original works of this project and do not
derive from the NanaZip icons (CC BY-ND 4.0).

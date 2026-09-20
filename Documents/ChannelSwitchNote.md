# Channel Switch Note for NanaZip development

> [!NOTE]
> CuinZip (2026-09): This is an upstream NanaZip document kept for reference.
> CuinZip no longer uses the NanaZip identities below. The CuinZip Shell
> Extension CLSIDs are Release `C8F7BD19-04D0-4086-820C-AD2F35AB89B0` and
> Preview `788F8FA7-178F-40BE-BAF5-5D5D1335C0F9`, with package identities
> `Cuin.CuinZip` / `Cuin.CuinZipPreview`. See Docs/CUINZIP_REBRAND.md.

P.S. Only for Kenji Mouri

## Preview

- DisplayName="NanaZip Preview"
- Name="40174MouriNaruto.NanaZipPreview"
- <DisplayName>NanaZip Preview</DisplayName>
- 469D94E9-6AF4-4395-B396-99B1308F8CE5
- return ::SHStrDupW(L"NanaZip Preview", ppszName);

## Stable

- DisplayName="NanaZip"
- Name="40174MouriNaruto.NanaZip"
- <DisplayName>NanaZip</DisplayName>
- CAE3F1D4-7765-4D98-A060-52CD14D56EAB
- return ::SHStrDupW(L"NanaZip", ppszName);

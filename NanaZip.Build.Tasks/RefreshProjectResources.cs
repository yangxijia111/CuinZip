using Microsoft.Build.Framework;
using Microsoft.Build.Utilities;
using Mile.DotNet.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace NanaZip.Build.Tasks
{
    public class RefreshProjectResources : Task
    {
        [Required]
        public string? RootPath { get; set; }

        [Required]
        public bool BuildPreviewRelease { get; set; }

        static void ReplaceFileContentViaStringList(
            string FilePath,
            List<string> FromList,
            List<string> ToList)
        {
            if (FromList.Count != ToList.Count)
            {
                throw new ArgumentOutOfRangeException();
            }

            string Content = File.ReadAllText(FilePath, Encoding.UTF8);

            for (int Index = 0; Index < FromList.Count; ++Index)
            {
                Content = Content.Replace(FromList[Index], ToList[Index]);
            }

            if (Path.GetExtension(FilePath) == ".rc")
            {
                File.WriteAllText(FilePath, Content, Encoding.Unicode);
            }
            else
            {
                Text.SaveTextToFileAsUtf8WithBom(FilePath, Content);
            }
        }

        // CuinZip (Rebrand 2026-09):列表在 CuinZip 正式版与 CuinZip Preview 两个品牌状态间切换。
        // 与上游的差异:Identity/CLSID/资产名均为 CuinZip 自有值,不再包含任何 NanaZip 品牌串,
        // 因此构建不会再把品牌回滚成 NanaZip。
        // CLSID Release=C8F7BD19-04D0-4086-820C-AD2F35AB89B0,Preview=788F8FA7-178F-40BE-BAF5-5D5D1335C0F9。
        static List<string> ReleaseStringList = new List<string>
        {
            "DisplayName=\"CuinZip\"",
            "Name=\"Cuin.CuinZip\"",
            "<DisplayName>CuinZip</DisplayName>",
            "C8F7BD19-04D0-4086-820C-AD2F35AB89B0",
            "return ::SHStrDupW(L\"CuinZip\", ppszName);",
            "<Content Include=\"..\\Assets\\CuinZipPackageAssets\\**\\*\">",
            "Assets/CuinZip.ico",
            "Assets/CuinZipSfx.ico",
        };

        static List<string> PreviewStringList = new List<string>
        {
            "DisplayName=\"CuinZip Preview\"",
            "Name=\"Cuin.CuinZipPreview\"",
            "<DisplayName>CuinZip Preview</DisplayName>",
            "788F8FA7-178F-40BE-BAF5-5D5D1335C0F9",
            "return ::SHStrDupW(L\"CuinZip Preview\", ppszName);",
            "<Content Include=\"..\\Assets\\CuinZipPreviewPackageAssets\\**\\*\">",
            "Assets/CuinZipPreview.ico",
            "Assets/CuinZipPreviewSfx.ico",
        };

        static List<string> FileList = new List<string>
        {
            @"{0}\NanaZip.Core\SevenZip\CPP\7zip\Bundles\SFXCon\resource.rc",
            @"{0}\NanaZip.Core\SevenZip\CPP\7zip\Bundles\SFXSetup\resource.rc",
            @"{0}\NanaZip.Core\SevenZip\CPP\7zip\Bundles\SFXWin\resource.rc",
            @"{0}\NanaZip.Universal\SevenZip\CPP\7zip\UI\GUI\resource.rc",
            @"{0}\NanaZip.UI.Modern\SevenZip\CPP\7zip\UI\FileManager\resource.rc",
            @"{0}\NanaZip.UI.Modern\NanaZip.ShellExtension.cpp",
            @"{0}\NanaZipPackage\Package.appxmanifest",
            @"{0}\NanaZipPackage\NanaZipPackage.wapproj",
        };

        public override bool Execute()
        {
            foreach (var FilePath in FileList)
            {
                ReplaceFileContentViaStringList(
                    string.Format(FilePath, RootPath),
                    BuildPreviewRelease ? ReleaseStringList : PreviewStringList,
                    BuildPreviewRelease ? PreviewStringList : ReleaseStringList);
            }

            return true;
        }
    }
}

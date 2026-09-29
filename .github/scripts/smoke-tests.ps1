# CuinZip CI smoke tests
# 验证 x64 Release 构建的基本功能未回归:
#   CLI 压缩/解压(.7z/.zip/AES)、分卷、SHA-256、品牌标识(manifest/文件属性)
# 用法: smoke-tests.ps1 -BinDir <x64 产物目录>
param(
    [Parameter(Mandatory = $true)]
    [string]$BinDir
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

function Invoke-Checked {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments 2>&1 | Write-Host
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed ($LASTEXITCODE): $Exe $Arguments"
    }
}

if (-not (Test-Path -LiteralPath $BinDir)) {
    throw "Binaries directory not found: $BinDir"
}

$Console = Join-Path $BinDir 'NanaZip.Universal.Console.exe'
foreach ($f in @($Console, (Join-Path $BinDir 'NanaZip.Core.dll'), (Join-Path $BinDir 'NanaZip.Codecs.dll'), (Join-Path $BinDir 'NanaZip.Universal.Windows.exe'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "Missing build output: $f" }
}

$Work = Join-Path $env:TEMP ("cuinzip-smoke-" + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $Work | Out-Null
try {
    # 测试数据
    $Payload = "CuinZip smoke test payload $(Get-Date -Format o)"
    Set-Content -LiteralPath (Join-Path $Work 'hello.txt') -Value $Payload -Encoding utf8

    # .7z(AES-256 + 加密文件名)
    Invoke-Checked $Console @('a', '-t7z', '-pSecret123', '-mhe', "$Work\test.7z", "$Work\hello.txt")
    if (-not (Test-Path -LiteralPath "$Work\test.7z")) { throw '7z archive was not created' }

    # .zip(AES-256)
    Invoke-Checked $Console @('a', '-tzip', '-pSecret123', '-mem=AES256', "$Work\test.zip", "$Work\hello.txt")
    if (-not (Test-Path -LiteralPath "$Work\test.zip")) { throw 'zip archive was not created' }

    # 分卷
    Invoke-Checked $Console @('a', '-t7z', '-v10k', "$Work\vol.7z", "$Work\hello.txt")
    if (-not (Test-Path -LiteralPath "$Work\vol.7z.001")) { throw 'split volume was not created' }

    # 解压 .7z 并校验内容
    Invoke-Checked $Console @('x', "-o$Work\out7z", '-pSecret123', "$Work\test.7z")
    $Extracted = Get-Content -LiteralPath (Join-Path $Work 'out7z\hello.txt') -Raw
    if ($Extracted.Trim() -ne $Payload.Trim()) { throw '7z round-trip content mismatch' }

    # 解压 .zip 并校验内容
    Invoke-Checked $Console @('x', "-o$Work\outzip", '-pSecret123', "$Work\test.zip")
    $ExtractedZip = Get-Content -LiteralPath (Join-Path $Work 'outzip\hello.txt') -Raw
    if ($ExtractedZip.Trim() -ne $Payload.Trim()) { throw 'zip round-trip content mismatch' }

    # SHA-256(控制台哈希 vs .NET 计算)
    $Expected = (Get-FileHash -LiteralPath (Join-Path $Work 'hello.txt') -Algorithm SHA256).Hash
    $HashOutput = & $Console @('h', '-scrcSHA256', "$Work\hello.txt") 2>&1 | Out-String
    if ($HashOutput -notmatch [regex]::Escape($Expected)) {
        throw "SHA-256 mismatch. Expected $Expected, got: $HashOutput"
    }

    # 品牌标识:MSIX manifest 身份(便携/安装目录无此文件时跳过,
    # CI 的包暂存目录一定存在)
    $ManifestPath = Join-Path $BinDir 'AppxManifest.xml'
    if (Test-Path -LiteralPath $ManifestPath) {
        $Manifest = Get-Content -LiteralPath $ManifestPath -Raw
        if ($Manifest -notmatch 'Cuin\.CuinZipPreview') { throw 'AppxManifest identity is not CuinZip' }
        if ($Manifest -notmatch 'fileExplorerContextMenus') { throw 'AppxManifest is missing Explorer context menu declarations' }
    }

    # 品牌标识:文件属性
    foreach ($exe in @('NanaZip.Universal.Windows.exe', 'NanaZip.Modern.FileManager.exe')) {
        $Info = (Get-Item -LiteralPath (Join-Path $BinDir $exe)).VersionInfo
        if ($Info.ProductName -ne 'CuinZip') { throw "$exe ProductName is '$($Info.ProductName)', expected 'CuinZip'" }
        if ($Info.FileDescription -notmatch 'CuinZip') { throw "$exe FileDescription is not CuinZip branded" }
    }

    Write-Host 'CuinZip smoke tests: PASS'
}
finally {
    Remove-Item -Recurse -Force -LiteralPath $Work -ErrorAction SilentlyContinue
}

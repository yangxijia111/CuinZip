param([string]$LauncherPath)
$ErrorActionPreference = 'Stop'
$taskDirectory = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\Output\Tests\LauncherContract'))
$env:TEMP = Join-Path $taskDirectory 'temp'
$env:TMP = $env:TEMP
if (-not $LauncherPath) { $LauncherPath = Join-Path $taskDirectory '..\..\Binaries\Release\x64\CuinZip.exe' }
$LauncherPath = (Resolve-Path -LiteralPath $LauncherPath).Path
$taskFixture = Join-Path $taskDirectory 'fixture with spaces 中文'
$taskRuntime = Join-Path $taskFixture 'x64'
$taskCaller = Join-Path $taskDirectory 'caller with spaces 中文'
foreach ($taskPath in @($taskFixture, $taskRuntime, $taskCaller)) {
    New-Item -ItemType Directory -Force -Path $taskPath | Out-Null
}
Copy-Item -LiteralPath $LauncherPath -Destination (Join-Path $taskFixture 'CuinZip.exe')
Copy-Item -LiteralPath $LauncherPath -Destination (Join-Path $taskFixture 'NanaZip.Modern.FileManager.exe')
Copy-Item -LiteralPath (Join-Path $taskDirectory 'child.exe') -Destination (Join-Path $taskRuntime 'NanaZip.Modern.FileManager.exe')
$taskRequired = @('NanaZip.Modern.dll','NanaZip.Core.dll','NanaZip.Codecs.dll','K7Base.dll','K7User.dll','NanaZip.Universal.Windows.exe','resources.pri','Mile.Xaml.Styles.SunValley.xbf')
foreach ($taskName in $taskRequired) { [IO.File]::WriteAllBytes((Join-Path $taskRuntime $taskName), [byte[]]@()) }
Add-Type @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
public static class LauncherContractWindows {
    public delegate bool Callback(IntPtr hwnd, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumWindows(Callback cb, IntPtr data);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr parent, Callback cb, IntPtr data);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint pid);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr hwnd, StringBuilder s, int count);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr hwnd, StringBuilder s, int count);
    [DllImport("user32.dll")] static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    public static string ReadAndClose(uint pid) {
        string result = null;
        EnumWindows((hwnd, unused) => {
            uint owner; GetWindowThreadProcessId(hwnd, out owner);
            if (owner != pid) return true;
            var cls = new StringBuilder(256); GetClassName(hwnd, cls, 256);
            if (cls.ToString() != "#32770") return true;
            var parts = new List<string>();
            EnumChildWindows(hwnd, (child, x) => {
                var text = new StringBuilder(8192); GetWindowText(child, text, 8192);
                if (text.Length > 0) parts.Add(text.ToString());
                return true;
            }, IntPtr.Zero);
            result = String.Join("\n", parts);
            PostMessage(hwnd, 0x0010, IntPtr.Zero, IntPtr.Zero);
            return false;
        }, IntPtr.Zero);
        return result;
    }
}
'@
function Start-TestLauncher([string]$Arguments, [string]$ExecutableName = 'CuinZip.exe') {
    $taskInfo = New-Object Diagnostics.ProcessStartInfo
    $taskInfo.FileName = Join-Path $taskFixture $ExecutableName
    $taskInfo.Arguments = $Arguments
    $taskInfo.WorkingDirectory = $taskCaller
    $taskInfo.UseShellExecute = $false
    $taskInfo.CreateNoWindow = $true
    return [Diagnostics.Process]::Start($taskInfo)
}
function Test-Forwarding([string]$Arguments, [string]$ExecutableName = 'CuinZip.exe') {
    $taskCapture = Join-Path $taskRuntime 'capture.txt'
    [IO.File]::WriteAllText($taskCapture, '')
    $taskProcess = Start-TestLauncher $Arguments $ExecutableName
    if (-not $taskProcess.WaitForExit(5000)) { $taskProcess.Kill(); throw 'Launcher did not exit' }
    if ($taskProcess.ExitCode -ne 0) { throw "Launcher exit: $($taskProcess.ExitCode)" }
    $taskDeadline = [DateTime]::UtcNow.AddSeconds(5)
    do {
        Start-Sleep -Milliseconds 100
        try { $taskText = [IO.File]::ReadAllText($taskCapture) } catch { $taskText = '' }
    } while (-not $taskText -and [DateTime]::UtcNow -lt $taskDeadline)
    if ($taskText -cne "args=$Arguments`r`ncwd=$taskCaller") { throw "Forwarding mismatch: $taskText" }
    return @{arguments=$Arguments; result='PASS'; cwd=$taskCaller}
}
function Test-ErrorDialog([string]$Expected) {
    $taskProcess = Start-TestLauncher ''
    $taskDeadline = [DateTime]::UtcNow.AddSeconds(5)
    $taskMessage = $null
    do {
        Start-Sleep -Milliseconds 100
        $taskMessage = [LauncherContractWindows]::ReadAndClose([uint32]$taskProcess.Id)
    } while (-not $taskMessage -and -not $taskProcess.HasExited -and [DateTime]::UtcNow -lt $taskDeadline)
    if (-not $taskProcess.WaitForExit(5000)) { $taskProcess.Kill(); throw 'Error dialog did not close' }
    if ($taskProcess.ExitCode -ne 1 -or -not $taskMessage -or $taskMessage -notmatch $Expected) {
        throw "Unexpected error response: exit=$($taskProcess.ExitCode), text=$taskMessage"
    }
    return @{result='PASS'; message=$taskMessage}
}
$taskResults = [ordered]@{}
$taskResults.no_arguments = Test-Forwarding ''
$taskResults.multiple_quoted_arguments = Test-Forwarding '"relative archive 中文.zip" "name with space.txt" -y'
$taskResults.literal_special_characters = Test-Forwarding '"archive $name & (copy).7z" "C:\folder with spaces\file.zip"'
$taskResults.legacy_installer_entry = Test-Forwarding '"relative archive 中文.zip"' 'NanaZip.Modern.FileManager.exe'
$taskMissing = Join-Path $taskRuntime 'resources.pri'
Rename-Item -LiteralPath $taskMissing -NewName 'resources.pri.fixture-backup'
try { $taskResults.missing_file = Test-ErrorDialog 'x64\\resources\.pri' }
finally { Rename-Item -LiteralPath (Join-Path $taskRuntime 'resources.pri.fixture-backup') -NewName 'resources.pri' }
$taskChild = Join-Path $taskRuntime 'NanaZip.Modern.FileManager.exe'
[IO.File]::WriteAllBytes($taskChild, [byte[]]@())
try { $taskResults.invalid_executable = Test-ErrorDialog 'Windows.*193' }
finally { Copy-Item -LiteralPath (Join-Path $taskDirectory 'child.exe') -Destination $taskChild }
$taskResults | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskDirectory 'result.json') -Encoding UTF8
$taskResults | ConvertTo-Json -Depth 5

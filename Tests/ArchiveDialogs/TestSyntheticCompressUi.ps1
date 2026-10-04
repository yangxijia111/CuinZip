param([string]$RuntimeDir = (Join-Path $PSScriptRoot '..\..\Output\Tests\ArchiveDialogs\UiRuntime'))
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
$RuntimeDir = (Resolve-Path -LiteralPath $RuntimeDir).Path
$info = [Diagnostics.ProcessStartInfo]::new()
$info.FileName = Join-Path $RuntimeDir 'NanaZip.Modern.FileManager.exe'
$info.WorkingDirectory = $RuntimeDir
$info.UseShellExecute = $false
$info.CreateNoWindow = $true
$info.WindowStyle = [Diagnostics.ProcessWindowStyle]::Hidden
$info.RedirectStandardOutput = $true
$info.RedirectStandardError = $true
$info.EnvironmentVariables['TEMP'] = $RuntimeDir
$info.EnvironmentVariables['TMP'] = $RuntimeDir
$process = [Diagnostics.Process]::new()
$process.StartInfo = $info
$scope = [Windows.Automation.TreeScope]::Descendants
function Find-Control($Root, [string]$Name, $Type) {
    $conditions = @([Windows.Automation.PropertyCondition]::new([Windows.Automation.AutomationElement]::NameProperty, $Name))
    if ($Type) { $conditions += [Windows.Automation.PropertyCondition]::new([Windows.Automation.AutomationElement]::ControlTypeProperty, $Type) }
    $condition = if ($conditions.Count -eq 1) { $conditions[0] } else { [Windows.Automation.AndCondition]::new([Windows.Automation.Condition[]]$conditions) }
    $control = $Root.FindFirst($scope, $condition)
    if (-not $control) { throw "Control not found: $Name" }
    return $control
}
function Click-Control($Root, [string]$Name) {
    $control = Find-Control $Root $Name ([Windows.Automation.ControlType]::Button)
    $control.GetCurrentPattern([Windows.Automation.InvokePattern]::Pattern).Invoke()
    Start-Sleep -Milliseconds 120
}
function Set-ComboEditor($Root, [string]$Name, [string]$Value) {
    $combo = Find-Control $Root $Name ([Windows.Automation.ControlType]::ComboBox)
    $editor = $combo.FindFirst($scope, [Windows.Automation.PropertyCondition]::new(
        [Windows.Automation.AutomationElement]::ControlTypeProperty, [Windows.Automation.ControlType]::Edit))
    if (-not $editor) { throw "Editable combo has no live editor: $Name" }
    $editor.GetCurrentPattern([Windows.Automation.ValuePattern]::Pattern).SetValue($Value)
}
try {
    $null = $process.Start()
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    $deadline = [DateTime]::UtcNow.AddSeconds(12)
    do {
        Start-Sleep -Milliseconds 150
        $process.Refresh()
    } while ($process.MainWindowHandle -eq [IntPtr]::Zero -and -not $process.HasExited -and [DateTime]::UtcNow -lt $deadline)
    if ($process.MainWindowHandle -eq [IntPtr]::Zero) {
        if (-not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
        Write-Output ($stdout.Result + $stderr.Result)
        throw "Synthetic dialog did not appear (exit $($process.ExitCode))"
    }
    $root = [Windows.Automation.AutomationElement]::FromHandle($process.MainWindowHandle)
    Click-Control $root 'More options'
    $parameters = Find-Control $root 'Parameters' ([Windows.Automation.ControlType]::Edit)
    $parameters.GetCurrentPattern([Windows.Automation.ValuePattern]::Pattern).SetValue('new-parameters')
    Set-ComboEditor $root 'Split volume size' ''
    Click-Control $root 'More options'
    Click-Control $root 'More options'
    $parameters = Find-Control $root 'Parameters' ([Windows.Automation.ControlType]::Edit)
    if ($parameters.GetCurrentPattern([Windows.Automation.ValuePattern]::Pattern).Current.Value -ne 'new-parameters') {
        throw 'Parameters were lost when More options was toggled'
    }
    Write-Output 'PASS: More options preserves custom parameters'
    Set-ComboEditor $root 'Archive name' 'typed archive.7z'
    Click-Control $root 'Create'
    if (-not $process.WaitForExit(10000)) { throw 'Accepted dialog did not close' }
    $output = $stdout.Result + $stderr.Result
    Write-Output $output
    if ($process.ExitCode -ne 0 -or $output -match 'FAIL|INIT_FAIL' -or ([regex]::Matches($output, 'PASS:')).Count -ne 4) {
        throw 'Synthetic form assertions failed'
    }
} finally {
    if (-not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
    $process.Dispose()
}

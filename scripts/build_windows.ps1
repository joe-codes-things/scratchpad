$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $projectRoot "build\Windows\x64"
$artifactsDir = Join-Path $projectRoot "exports\production-builds\Windows"
$installerScript = Join-Path $projectRoot "installer\windows\ScratchPad.iss"
$innoCompiler = Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 6\ISCC.exe"
$cmake = (Get-Command cmake.exe -ErrorAction Stop).Source

$versionMatch = Select-String -Path (Join-Path $projectRoot "CMakeLists.txt") `
    -Pattern '^project\(ScratchpadPlugin VERSION ([0-9]+\.[0-9]+\.[0-9]+)'
if (-not $versionMatch) {
    throw "Could not read the project version from CMakeLists.txt."
}
$version = $versionMatch.Matches[0].Groups[1].Value

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "Visual Studio vswhere.exe was not found."
}
$visualStudio = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64 `
    -property installationPath
if (-not $visualStudio) {
    throw "Visual Studio x64 and ARM64 C++ build tools are missing."
}
if (-not (Test-Path $innoCompiler)) {
    throw "Inno Setup 6 compiler was not found: $innoCompiler"
}

function Invoke-CheckedCommand {
    param(
        [Parameter(Mandatory = $true)][string]$Executable,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )

    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $Executable $Arguments"
    }
}

New-Item -ItemType Directory -Path $buildDir, $artifactsDir -Force | Out-Null

$buildCache = Join-Path $buildDir "CMakeCache.txt"
if (Test-Path $buildCache) {
    $cachedSource = Select-String -Path $buildCache `
        -Pattern '^CMAKE_HOME_DIRECTORY:INTERNAL=(.+)$'
    if ($cachedSource -and $cachedSource.Matches[0].Groups[1].Value -ne $projectRoot) {
        Remove-Item -LiteralPath $buildDir -Recurse -Force
        New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    }
}

$env:CMAKE_GENERATOR_INSTANCE = $visualStudio
$env:CMAKE_GENERATOR = "Visual Studio 17 2022"
$env:VS170COMNTOOLS = Join-Path $visualStudio "Common7\Tools"
$helperBuildDir = Join-Path $buildDir "JUCE\tools"
$helperCache = Join-Path $helperBuildDir "CMakeCache.txt"
if (Test-Path $helperCache) {
    $helperInstance = Select-String -Path $helperCache `
        -Pattern '^CMAKE_GENERATOR_INSTANCE:INTERNAL=(.+)$'
    if ($helperInstance -and $helperInstance.Matches[0].Groups[1].Value -notlike "*$visualStudio*") {
        Remove-Item -LiteralPath $helperBuildDir -Recurse -Force
    }
}
Invoke-CheckedCommand $cmake @(
    "-S", $projectRoot,
    "-B", $buildDir,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_GENERATOR_INSTANCE=$visualStudio"
)
Invoke-CheckedCommand $cmake @(
    "--build", $buildDir,
    "--config", "Release",
    "--target", "ScratchpadPlugin_VST3",
    "--parallel"
)

$pluginSource = Join-Path $buildDir "ScratchpadPlugin_artefacts\Release\VST3\ScratchPad.vst3"
if (-not (Test-Path $pluginSource -PathType Container)) {
    throw "Windows VST3 build output was not found: $pluginSource"
}

$pluginDestination = Join-Path $artifactsDir "nla_scratchpad.vst3"
if (Test-Path $pluginDestination) {
    Remove-Item -LiteralPath $pluginDestination -Recurse -Force
}
Copy-Item -LiteralPath $pluginSource -Destination $pluginDestination -Recurse

$installerArguments = @(
    "/DAppVersion=$version",
    "/DPluginPath=$pluginDestination",
    "/DOutputPath=$artifactsDir",
    $installerScript
)
Invoke-CheckedCommand $innoCompiler $installerArguments

Write-Output "Windows VST3 built: $pluginDestination"
Write-Output "Windows installer built: $(Join-Path $artifactsDir "ScratchPad_v${version}_Windows.exe")"

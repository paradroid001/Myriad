param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Arguments
)

$ErrorActionPreference = 'Stop'

$scriptRoot = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($scriptRoot)) {
    $scriptRoot = Split-Path -LiteralPath $MyInvocation.MyCommand.Path -Parent
}

$projectRoot = (Resolve-Path -LiteralPath $scriptRoot).Path
$buildDir = Join-Path $projectRoot 'build\MinGW 10 x86_64 windows\Debug'
$editorExe = Join-Path $buildDir 'Editor\MyriadEditor.exe'

$mappedDrive = $null
if ($projectRoot.StartsWith('\\')) {
    $driveName = 'MyriadRun'
    if (Get-PSDrive -Name $driveName -ErrorAction SilentlyContinue) {
        Remove-PSDrive -Name $driveName -ErrorAction SilentlyContinue
    }

    $mappedDrive = New-PSDrive -Name $driveName -PSProvider FileSystem -Root $projectRoot -Scope Script
    $projectRoot = "$($mappedDrive.Name):\\"
    $buildDir = Join-Path $projectRoot 'build\MinGW 10 x86_64 windows\Debug'
    $editorExe = Join-Path $buildDir 'Editor\MyriadEditor.exe'
}

if (-not (Test-Path -LiteralPath $editorExe)) {
    Write-Error "MyriadEditor.exe was not found at '$editorExe'. Build the editor with the MinGW toolchain first."
    exit 1
}

$originalPath = $env:PATH
try {
    $env:PATH = "$([IO.Path]::Combine($buildDir, 'Editor'));$([IO.Path]::Combine($buildDir, 'Engine'));$([IO.Path]::Combine($buildDir, 'Examples\TestECS'));$originalPath"
    Push-Location -LiteralPath $projectRoot
    try {
        & $editorExe @Arguments
        exit $LASTEXITCODE
    }
    finally {
        Pop-Location
    }
}
finally {
    $env:PATH = $originalPath
    if ($null -ne $mappedDrive) {
        Remove-PSDrive -Name $mappedDrive.Name -ErrorAction SilentlyContinue
    }
}

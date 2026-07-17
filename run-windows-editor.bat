@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "LAUNCHER=%SCRIPT_DIR%run-windows-editor.ps1"

if not exist "%LAUNCHER%" (
    echo PowerShell launcher was not found at "%LAUNCHER%".
    exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -File "%LAUNCHER%" %*
exit /b %ERRORLEVEL%

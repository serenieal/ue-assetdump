@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM WorkSession.bat
REM v0.3
REM Purpose:
REM   1) Run SSOT dump once before work  (Start)
REM   2) Launch Unreal Editor and wait until it exits
REM   3) Run SSOT dump once after work   (End)
REM   4) Rename ssot_*.json -> start_ssot_*.json / end_ssot_*.json for easy distinction

REM [VAR] UnrealEditor-Cmd.exe path (for SSOTDump commandlet)
set "UnrealCmdPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM [VAR] UnrealEditor.exe path (GUI editor)
set "UnrealGuiPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"

REM [VAR] .uproject path
set "UprojectPath=D:\Work\CarFight_git\UE\CarFight_Re.uproject"

REM [VAR] SSOT output root directory
set "SsotRootDir=D:\Work\CarFight_git\UE\Saved\SSOT"

REM [VAR] Dump target (assets / input / wheel / all)
set "DumpTarget=all"

REM [VAR] Autodetect flag (for input root)
set "AutoDetectFlag=-autodetect"

REM ---------------------------------
REM 0) Basic sanity checks
REM ---------------------------------
if not exist "%UnrealCmdPath%" (
  echo [ERROR] UnrealCmdPath not found: "%UnrealCmdPath%"
  pause
  exit /b 1
)

if not exist "%UnrealGuiPath%" (
  echo [ERROR] UnrealGuiPath not found: "%UnrealGuiPath%"
  pause
  exit /b 1
)

if not exist "%UprojectPath%" (
  echo [ERROR] UprojectPath not found: "%UprojectPath%"
  pause
  exit /b 1
)

REM ---------------------------------
REM 1) Start dump
REM ---------------------------------
set "StartOutDir=%SsotRootDir%\Start"
if not exist "%StartOutDir%" mkdir "%StartOutDir%"

echo [SSOT] Start dump...
"%UnrealCmdPath%" "%UprojectPath%" -run=SSOTDump -out="%StartOutDir%" -target=%DumpTarget% %AutoDetectFlag% -unattended -nop4 -NullRHI -notime
set "StartDumpCode=%errorlevel%"
echo [SSOT] Start dump exit code: %StartDumpCode%

REM Rename: ssot_*.json -> start_ssot_*.json
for %%F in ("%StartOutDir%\ssot_*.json") do (
  ren "%%~fF" "start_%%~nxF"
)

REM ---------------------------------
REM 2) Launch editor (wait until exit)
REM ---------------------------------
echo [UE] Launch editor...
start "" /wait "%UnrealGuiPath%" "%UprojectPath%"

REM ---------------------------------
REM 3) End dump
REM ---------------------------------
set "EndOutDir=%SsotRootDir%\End"
if not exist "%EndOutDir%" mkdir "%EndOutDir%"

echo [SSOT] End dump...
"%UnrealCmdPath%" "%UprojectPath%" -run=SSOTDump -out="%EndOutDir%" -target=%DumpTarget% %AutoDetectFlag% -unattended -nop4 -NullRHI -notime
set "EndDumpCode=%errorlevel%"
echo [SSOT] End dump exit code: %EndDumpCode%

REM Rename: ssot_*.json -> end_ssot_*.json
for %%F in ("%EndOutDir%\ssot_*.json") do (
  ren "%%~fF" "end_%%~nxF"
)

REM ---------------------------------
REM 4) Done
REM ---------------------------------
echo.
echo [DONE] StartDump=%StartDumpCode%  EndDump=%EndDumpCode%
echo Output folders:
echo   %StartOutDir%
echo   %EndOutDir%
echo.
pause
exit /b %EndDumpCode%

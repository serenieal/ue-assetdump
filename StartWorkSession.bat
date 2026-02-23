@echo off
setlocal EnableExtensions EnableDelayedExpansion
REM WorkSession.bat
REM v0.4
REM Purpose:
REM   1) Start SSOT dump once
REM   2) Launch Unreal Editor (wait until exit)
REM   3) End SSOT dump once
REM   4) Rename ssot_*.json -> start_ssot_*.json / end_ssot_*.json
REM   5) Prevent rename errors by cleaning output + auto-generate ssot_cfg.lst

REM [VAR] UnrealEditor-Cmd.exe path (SSOTDump)
set "UnrealCmdPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM [VAR] UnrealEditor.exe path (GUI)
set "UnrealGuiPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"

REM [VAR] uproject path
set "UprojectPath=D:\Work\CarFight_git\UE\CarFight_Re.uproject"

REM [VAR] SSOT output root directory
set "SsotRootDir=D:\Work\CarFight_git\UE\Saved\SSOT"

REM [VAR] Dump target (assets / input / wheel / all)
set "DumpTarget=all"

REM [VAR] Autodetect flag (avoid hardcoding input root)
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
call :PrepOutDir "%StartOutDir%" "start_"

echo [SSOT] Start dump...
"%UnrealCmdPath%" "%UprojectPath%" -run=SSOTDump -out="%StartOutDir%" -target=%DumpTarget% %AutoDetectFlag% -unattended -nop4 -NullRHI -notime
set "StartDumpCode=%errorlevel%"
echo [SSOT] Start dump exit code: %StartDumpCode%

call :RenameSsotJson "%StartOutDir%" "start_"

REM ---------------------------------
REM 2) Launch editor (wait until exit)
REM ---------------------------------
echo [UE] Launch editor...
start "" /wait "%UnrealGuiPath%" "%UprojectPath%"

REM ---------------------------------
REM 3) End dump
REM ---------------------------------
set "EndOutDir=%SsotRootDir%\End"
call :PrepOutDir "%EndOutDir%" "end_"

echo [SSOT] End dump...
"%UnrealCmdPath%" "%UprojectPath%" -run=SSOTDump -out="%EndOutDir%" -target=%DumpTarget% %AutoDetectFlag% -unattended -nop4 -NullRHI -notime
set "EndDumpCode=%errorlevel%"
echo [SSOT] End dump exit code: %EndDumpCode%

call :RenameSsotJson "%EndOutDir%" "end_"

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

REM ===========================================================
REM Function: PrepOutDir
REM  - Create output folder
REM  - Clean previous outputs (prevents duplicate rename errors)
REM  - Auto-generate ssot_cfg.lst (prevents exit code 1 when target=all)
REM ===========================================================
:PrepOutDir
set "OutDir=%~1"
set "Prefix=%~2"

if not exist "%OutDir%" mkdir "%OutDir%"

del /q "%OutDir%\%Prefix%ssot_*.json" 2>nul
del /q "%OutDir%\ssot_*.json" 2>nul
del /q "%OutDir%\ssot_cfg.lst" 2>nul

REM Minimal whitelist so DumpConfig won't fail
(
  echo # ssot_cfg.lst (auto-generated)
  echo DefaultEngine.ini^|/Script/EngineSettings.GameMapsSettings^|GameDefaultMap
  echo DefaultEngine.ini^|/Script/EngineSettings.GameMapsSettings^|EditorStartupMap
) > "%OutDir%\ssot_cfg.lst"

exit /b 0

REM ===========================================================
REM Function: RenameSsotJson
REM  - If ssot_*.json exists, rename to prefix+filename
REM  - If none exists, do nothing (prevents "file not found" noise)
REM ===========================================================
:RenameSsotJson
set "OutDir=%~1"
set "Prefix=%~2"

if exist "%OutDir%\ssot_*.json" (
  for %%F in ("%OutDir%\ssot_*.json") do (
    if exist "%%~fF" (
      ren "%%~fF" "%Prefix%%%~nxF"
    )
  )
)

exit /b 0

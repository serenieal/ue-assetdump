@echo off
REM WorkSession.bat
REM v0.1
REM 목적: 작업 시작 시 SSOT 덤프 1회 + 에디터 실행 + 종료 시 SSOT 덤프 1회

REM [변수] UnrealEditor-Cmd.exe 경로 (SSOTDump 실행용)
set "UnrealCmdPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM [변수] UnrealEditor.exe 경로 (에디터 GUI 실행용)
set "UnrealGuiPath=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"

REM [변수] uproject 경로
set "UprojectPath=D:\Work\CarFight_git\UE\CarFight_Re.uproject"

REM [변수] SSOT 출력 루트 폴더 (Saved 아래 추천)
set "SsotRootDir=D:\Work\CarFight_git\UE\Saved\SSOT"

REM [변수] 덤프 타겟 (assets / input / wheel / all)
set "DumpTarget=all"

REM [변수] 자동탐지 사용 (input_root 하드코딩 방지)
set "AutoDetectFlag=-autodetect"

REM -------------------------
REM 1) 작업 시작 덤프
REM -------------------------
echo [SSOT] Start dump...
"%UnrealCmdPath%" "%UprojectPath%" ^
  -run=SSOTDump ^
  -out="%SsotRootDir%\Start" ^
  -target=%DumpTarget% ^
  %AutoDetectFlag% ^
  -unattended -nop4 -NullRHI -notime

REM [변수] 시작 덤프 결과 코드 저장
set "StartDumpCode=%errorlevel%"
echo [SSOT] Start dump exit code: %StartDumpCode%

REM -------------------------
REM 2) 에디터 실행 (닫힐 때까지 대기)
REM -------------------------
echo [UE] Launch editor...
start "" /wait "%UnrealGuiPath%" "%UprojectPath%"

REM -------------------------
REM 3) 작업 종료 덤프
REM -------------------------
echo [SSOT] End dump...
"%UnrealCmdPath%" "%UprojectPath%" ^
  -run=SSOTDump ^
  -out="%SsotRootDir%\End" ^
  -target=%DumpTarget% ^
  %AutoDetectFlag% ^
  -unattended -nop4 -NullRHI -notime

REM [변수] 종료 덤프 결과 코드 저장
set "EndDumpCode=%errorlevel%"
echo [SSOT] End dump exit code: %EndDumpCode%

REM -------------------------
REM 4) 마무리 출력
REM -------------------------
echo.
echo [DONE] StartDump=%StartDumpCode%  EndDump=%EndDumpCode%
echo Output:
echo   %SsotRootDir%\Start
echo   %SsotRootDir%\End
echo.

pause
exit /b %EndDumpCode%
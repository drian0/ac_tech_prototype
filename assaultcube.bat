@ECHO OFF

set TESS_BIN=bin

IF EXIST bin64\assaultcube.exe (
    IF /I "%PROCESSOR_ARCHITECTURE%" == "amd64" (
        set TESS_BIN=bin64
    )
    IF /I "%PROCESSOR_ARCHITEW6432%" == "amd64" (
        set TESS_BIN=bin64
    )
)

start %TESS_BIN%\assaultcube.exe "-u$HOME\My Games\AssaultCube\ACTP" -glog.txt %*

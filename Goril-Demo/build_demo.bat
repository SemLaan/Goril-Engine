@echo off

setlocal enabledelayedexpansion enableextensions
set FileLIST=
for /R %~DP0 %%f in (*.c) do set FileLIST=!FileLIST! %%f
set FileLIST=%FileLIST:~1%

set defines=-DGR_DEBUG -D__win__ -DDEBUG -D_DEBUG
set includepaths=-I%~DP0/../Goril/src -I%~DP0/src
set linkpaths=-L%~DP0/../bin/Debug
set links=-lGoril
set compilerflags=-Wall -std=c17 -Wno-unused-function -g -march=native

if not exist "%~DP0/../bin/Debug" mkdir "%~DP0/../bin/Debug"

gcc %FileLIST% %compilerflags% -o %~DP0/../bin/Debug/Goril-Demo.exe %defines% %includepaths% %linkpaths% %links% -fmax-errors=0
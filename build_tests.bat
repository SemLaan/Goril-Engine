@echo off

setlocal enabledelayedexpansion enableextensions
set FileLIST=
for /R %~DP0/Goril %%f in (*.c) do set FileLIST=!FileLIST! %%f
set FileLIST=%FileLIST:~1%
for /R %~DP0/Tests %%f in (*.c) do set FileLIST=!FileLIST! %%f

set defines=-DGR_DEBUG -D__win__ -DDEBUG -D_DEBUG
set includepaths=-I%~DP0/Goril/src/ -I%~DP0/Tests/src/ -I%VULKAN_SDK%/Include
set linkpaths=-L%VULKAN_SDK%/Lib
set links=-lvulkan-1
set compilerflags=-Wall -std=c17 -Wno-unused-function -g -march=native -msse3

if not exist "%~DP0/bin/Debug" mkdir "%~DP0/bin/Debug"

echo compiling tests and engine...
gcc %FileLIST% %compilerflags% -o %~DP0/bin/Debug/Tests.exe %defines% %includepaths% %linkpaths% %links% -fmax-errors=0

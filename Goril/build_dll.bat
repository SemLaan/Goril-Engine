@echo off

setlocal enabledelayedexpansion enableextensions
set FileLIST=
for /R %~DP0 %%f in (*.c) do set FileLIST=!FileLIST! %%f
set FileLIST=%FileLIST:~1%

set defines=-DGR_DEBUG -D__win__ -DGR_DLL -DDEBUG -D_DEBUG
set includepaths=-I%~DP0src/ -I%VULKAN_SDK%/Include
set linkpaths=-L%VULKAN_SDK%/Lib
set links=-lvulkan-1
set compilerflags=-Wall -std=c17 -shared -g -march=native

if not exist "%~DP0/../bin/Debug" mkdir "%~DP0/../bin/Debug"

gcc %FileLIST% %compilerflags% -o %~DP0/../bin/Debug/Goril.dll %defines% %includepaths% %linkpaths% %links% -fmax-errors=0
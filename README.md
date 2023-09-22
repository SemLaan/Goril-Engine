# Goril-Engine
Simple C game engine for hobby projects

## Building
The vulkan sdk needs to be installed.\
To build the project run build_game_static.bat, to build the tests run build_tests.bat.\
You do need to have GCC installed and added to the path for the bat files to work. \
If you're using visual studio there's also a .vscode folder so there's tasks set up for building.\
You can also use another editor but you'll have to set it up yourself.\
The build scripts included are also only for debug mode.

### If you want to use a different build system, here's what you need to know:
Note that I have only compiled this code with GCC no idea if it works with other compilers.

Building the game:\
C standard is c17\
Build all .c files in Goril and Goril-Demo, include Goril/src and Goril-Demo/src and %VULKAN_SDK%/Include\
Link with vulkan-1.lib\
Global defines: GR_DEBUG \_\_win\_\_ GR_NODLL DEBUG _DEBUG\
If you want to compile in release mode remove all the debug defines and define GR_DIST\
Also run compile_shaders.bat or compile the shaders in whichever way you like (make sure they get put in the same folder as the exe).

Building the tests:\
C standard is c17\
Build all .c files in Goril and Tests, include Goril/src and Tests/src and %VULKAN_SDK%/Include\
Link with vulkan-1.lib\
Global defines: GR_DEBUG \_\_win\_\_ GR_NODLL DEBUG _DEBUG\
If you want to compile in release mode remove all the debug defines and define GR_DIST


-- Setting workspace params
workspace "Goril"
	location "./"
	configurations { "Debug", "Release", "Dist" }
	platforms { "Windows" }
	language "C++"
	cppdialect "C++20"
	startproject "Goril-Demo"

	-- Setting platforms
	filter { "platforms:Windows" }
		defines "__win__"
		system "Windows"
    	architecture "x86_64"
	filter {}

	-- Configuration specific settings for all projects
	filter "configurations:Debug"
		defines { "GR_DEBUG" , "DEBUG", "_DEBUG"}
		symbols "On"
	filter {}
		
	filter "configurations:Release"
		defines "GR_RELEASE"
		optimize "On"
	filter {}
		
	filter "configurations:Dist"
		defines "GR_DIST"
		optimize "On"
	filter {}

	-- Copying the dll to the Demo and tests folder
	postbuildcommands 
	{
		"{COPY} %{wks.location}/build/bin/%{cfg.buildcfg}/Goril/Goril.dll %{wks.location}/build/bin/%{cfg.buildcfg}/Goril-Demo",
		"{COPY} %{wks.location}/build/bin/%{cfg.buildcfg}/Goril/Goril.dll %{wks.location}/build/bin/%{cfg.buildcfg}/Tests",
		"for /r %%i in (*.frag, *.vert) do %VULKAN_SDK%/Bin/glslc %%i -o src/renderer/shaders/%%~ni.spv"
	}

	-- Engine dll
	project "Goril"
		kind "SharedLib"
		location "%{wks.location}/Goril/"

		targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
		objdir ("%{wks.location}/build/bin-intermediate/%{cfg.architecture}-%{cfg.buildcfg}/%{prj.name}")

		files
		{
			"%{prj.name}/linking/include/**.h",
			"%{prj.name}/linking/libs/**",
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{prj.name}/linking/include",
			"%{prj.name}/src",
			"%VULKAN_SDK%/Include",
		}

		libdirs 
		{ 
			"%{prj.name}/linking/libs" 
		}

		links 
		{
			"%VULKAN_SDK%/Lib/vulkan-1.lib"
		}

		defines 
		{
			"GR_DLL"
		}


	-- Engine test game
	project "Goril-Demo"
		kind "ConsoleApp"
		location "%{wks.location}/Goril-Demo/"

		targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
		objdir ("%{wks.location}/build/bin-intermediate/%{cfg.architecture}-%{cfg.buildcfg}/%{prj.name}")

		files
		{
			"%{prj.name}/linking/include/**.h",
			"%{prj.name}/linking/libs/**",
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%VULKAN_SDK%/Include",
			"%{wks.location}/Goril/src",
		}
		
		libdirs 
		{ 
		}
		
		links 
		{ 
			"Goril",
		}
		
		defines {}
		
		
	-- Unit testing
	project "Tests"
		kind "ConsoleApp"
		location "%{wks.location}/Tests/"

		targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
		objdir ("%{wks.location}/build/bin-intermediate/%{cfg.architecture}-%{cfg.buildcfg}/%{prj.name}")

		files
		{
			"%{prj.name}/linking/include/**.h",
			"%{prj.name}/linking/libs/**",
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
		}

		includedirs
		{
			"%VULKAN_SDK%/Include",
			"%{wks.location}/Goril/src",
			"%{prj.name}/src",
		}
		
		libdirs 
		{ 
		}
		
		links 
		{ 
			"Goril",
		}
		
		defines {}
		
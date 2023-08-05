-- Setting workspace params
workspace "Goril"
	location "./"
	configurations { "Debug", "Release", "Dist" }
	platforms { "Windows" }
	language "C++"
	cppdialect "C++20"

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
		"{COPY} %{wks.location}/build/bin/%{cfg.buildcfg}/Goril/Goril.dll %{wks.location}/build/bin/%{cfg.buildcfg}/Goril-Demo"
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
			"%{prj.name}/linking/include/**glad.c",
			"%{prj.name}/linking/libs/**",
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{prj.name}/linking/include",
			"%{prj.name}/src",
		}

		libdirs 
		{ 
			"%{prj.name}/linking/libs" 
		}

		links 
		{ 
			"glfw3.lib",
			"spdlog.lib",
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
			"%{wks.location}/Goril/linking/include",
			"%{wks.location}/Goril/src",
		}
		
		libdirs 
		{ 
			"%{wks.location}/Goril/linking/libs" 
		}
		
		links 
		{ 
			"Goril",
			"glfw3.lib",
			"spdlog.lib",
		}
		
		defines {}
		
		
	-- Unit testing
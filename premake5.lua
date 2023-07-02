workspace "Goril"
	architecture "x86_64"
	location "./"
	configurations { "Debug", "Release", "Dist" }


	project "Goril"
		kind "ConsoleApp"
		language "C++"
		location "%{wks.location}/Goril/"

		targetdir ("%{wks.location}/build/bin/%{cfg.buildcfg}/%{prj.name}")
		objdir ("%{wks.location}/build/bin-intermediate/%{cfg.architecture}-%{cfg.buildcfg}/%{prj.name}")

		cppdialect "C++20"

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{prj.name}/vendor/headers;",
			"%{prj.name}/src/engine;",
			"%{prj.name}/src/platform;"
		}


		defines {}

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

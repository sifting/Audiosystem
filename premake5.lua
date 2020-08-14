workspace "as"
	configurations {"Debug", "Release"}
	location "build"
	targetdir "."
	debugdir "."
	filter "language:C"
		toolset "gcc"
		buildoptions {"-std=c11 -pedantic -Wall"}
	filter "system:windows"
		links {"mingw32"}
	filter "configurations:Debug"
		defines {"DEBUG"}
		symbols "On"
	filter "configurations:Test"
		defines {"DEBUG", "TEST"}
		symbols "On"
	filter "configurations:Release"
		defines {"NDEBUG"}
		vectorextensions "Default"
		optimize "Speed"
	project "as"
		language "C"
		kind "StaticLib"
		includedirs {"as/public"}
		sysincludedirs {"headers"}
		postbuildcommands {"cd .. && python copy.py as"}
		files {
			"as/**.h",
			"as/**.c",
		}
		filter {}

solution "slice"
	configurations { "Debug", "Release" }
	platforms { "x64", "x86" } --, "Clang" }

	project "slice"
	kind "ConsoleApp"
	language "C++"

	files { "../include/**", "src/**" }

	includedirs { "../include" }

	filter { "configurations:Debug*" }
		defines { "_DEBUG" }
		optimize "Off"
		symbols "On"

	filter { "configurations:Release*" }
		defines { "NDEBUG" }
		optimize "Full"
		symbols "On"
		flags { "NoFramePointer", "NoBufferSecurityCheck" }

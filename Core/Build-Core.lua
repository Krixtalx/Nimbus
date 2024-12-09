project "Nimbus Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "On"
   vectorextensions "AVX2"

   pchheader "CorePch.h"
   pchsource ("Source/CorePch.cpp")

   files { "Source/**.h", "Source/**.hpp", "Source/**.cpp"}

   includedirs
   {
      "Source",
      "%{wks.location}/Dependencies",
   }

   defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "NimbusDebug" }
       runtime "Debug"
       symbols "FastLink"

   filter "configurations:Release"
       defines { "NimbusRelease" }
       buildoptions {"/incremental"}
       runtime "Release"
       optimize "On"
       symbols "FastLink"

   filter "configurations:Dist"
       defines { "NimbusDist" }
       runtime "Release"
       optimize "Full"
       symbols "Off"
       enableunitybuild "On"
       flags {"LinkTimeOptimization"}
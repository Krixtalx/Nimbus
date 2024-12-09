project "Nimbus App"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "On"
   vectorextensions "AVX2"

   pchheader "AppPch.h"
   pchsource ("Source/AppPch.cpp")

   files { "Source/**.h", "Source/**.hpp", "Source/**.cpp", "Source/**.rc" }

   includedirs
   {
      "Source",
	  -- Include Core
	  "../Core/Source",
   }

   links
   {
        "Nimbus Core"
   }

   targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "FastLink"

   filter "configurations:Release"
       defines { "RELEASE" }
       buildoptions {"/incremental"}
       runtime "Release"
       optimize "On"
       symbols "FastLink"

   filter "configurations:Dist"
       kind("WindowedApp")
       defines { "DIST" }
       runtime "Release"
       optimize "Full"
       enableunitybuild "On"
       symbols "Off"
       entrypoint "mainCRTStartup"
       flags {"LinkTimeOptimization"}

       

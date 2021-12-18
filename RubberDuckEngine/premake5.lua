
workspace "RubberDuckEngine"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"

project "RubberDuckEngine"
    location "RubberDuckEngine"
    kind "ConsoleApp"
    language "C++"

    targetdir("bin/" .. outputdir .. "/%{prj.name}")
    objdir("bin-int/" .. outputdir .. "/%{prj.name}")

    editAndContinue "Off"
    
    pchheader "Pch.h"
	pchsource("%{prj.name}/Source/Pch.cpp")

	editAndContinue "Off"
    
    files
    {
        "%{prj.name}/Source/**.cpp",
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.hpp"
    }

    excludes 
    {
    }

    defines
    {
    }
    
    ignoredefaultlibraries 
    { 
    }

    removefiles
    {
    }

    -- Directories to be included (libraries, plug-ins, etc)
    includedirs
    {
      "%{prj.name}/Source/",
      "%{prj.name}/Dep/glfw/include/",
      "%{prj.name}/Dep/VulkanSDK/1.2.198.1/Include/",
      "%{prj.name}/Dep/glm/",
      "%{prj.name}/Dep/spdlog/include"
    }

    libdirs
    {
      "%{prj.name}/Dep/glfw/lib-vc2022/",
      "%{prj.name}/Dep/VulkanSDK/1.2.198.1/Lib/",
    }

    links
    {
        "glfw3.lib",
        "vulkan-1.lib"
    }

    cppdialect "C++17"
    systemversion "latest"

    flags
    {
        "MultiProcessorCompile"
    }

    filter "configurations:Debug"
        defines "RD_DEBUG"
        symbols "On"

        libdirs
        {
            "%{prj.name}/Dep/spdlog/lib/debug",
        }

        links
        {
            "spdlogd.lib"
        }
        
        buildoptions { "/bigobj"}

    filter "configurations:Release"
        defines "RD_RELEASE"
        optimize "Speed"

        flags
        {
            "LinkTimeOptimization"
        }

        libdirs
        {
            "%{prj.name}/Dep/spdlog/lib/debug",
        }

        links
        {
            "spdlog.lib"
        }
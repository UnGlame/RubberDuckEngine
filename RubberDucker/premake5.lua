
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
    
    pchheader "precompiled/pch.hpp"
	pchsource("%{prj.name}/source/precompiled/pch.cpp")

	editAndContinue "Off"
    
    files
    {
        "%{prj.name}/source/**.cpp",
        "%{prj.name}/source/**.h",
        "%{prj.name}/source/**.hpp",
        "%{prj.name}/assets/shaders/*"
    }

    excludes 
    {
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "WIN32_LEAN_AND_MEAN",
        "NOMINMAX",
        "GLM_FORCE_RADIANS",
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
        "%{prj.name}/source/",
        "%{prj.name}/dep/entt/include",
        "%{prj.name}/dep/glfw/include/",
        "%{prj.name}/dep/glm/",
        "%{prj.name}/dep/spdlog/include",
        "%{prj.name}/dep/stbi/include",
        "%{prj.name}/dep/vulkan/include/"
    }

    libdirs
    {
        "%{prj.name}/dep/vulkan/lib/",
        "%{prj.name}/dep/glfw/lib-vc2022/",
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
        defines "RDE_DEBUG"
        symbols "On"

        libdirs
        {
            "%{prj.name}/dep/spdlog/lib/debug",
        }

        links
        {
            "spdlogd.lib"
        }
        
        buildoptions { "/bigobj"}

    filter "configurations:Release"
        defines "RDE_RELEASE"
        optimize "Speed"

        flags
        {
            "LinkTimeOptimization"
        }

        libdirs
        {
            "%{prj.name}/dep/spdlog/lib/release",
        }

        links
        {
            "spdlog.lib"
        }
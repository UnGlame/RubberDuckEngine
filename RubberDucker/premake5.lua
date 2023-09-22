
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
        "%{prj.name}/dep/imgui/source/**.cpp",
        "%{prj.name}/assets/shaders/*"
    }

    excludes 
    {
    }

    defines
    {
        "RDE_ENABLE_VALIDATION_LAYERS",
        "_CRT_SECURE_NO_WARNINGS",
        "WIN32_LEAN_AND_MEAN",
        "NOMINMAX",
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_FORCE_INLINE",
        "GLM_FORCE_INTRINSICS"
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
        "%{prj.name}/dep/imgui/source/include",
        "%{prj.name}/dep/glfw/include/",
        "%{prj.name}/dep/glm/",
        "%{prj.name}/dep/mono/include/",
        "%{prj.name}/dep/rttr/include",
        "%{prj.name}/dep/spdlog/include",
        "%{prj.name}/dep/stbi/include",
        "%{prj.name}/dep/tinyobjloader/include",
        "%{prj.name}/dep/vulkan/include/",
        "%{prj.name}/dep/vma/include/"
    }

    libdirs
    {
        "%{prj.name}/dep/vulkan/lib/",
        "%{prj.name}/dep/glfw/lib-vc2022/"
    }

    links
    {
        "glfw3.lib",
        "vulkan-1.lib",
    }

    postbuildcommands
    {
        --"{COPY} dep/mono %{cfg.targetdir}"
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
            "%{prj.name}/dep/mono/lib/debug",
            "%{prj.name}/dep/rttr/lib/debug",
            "%{prj.name}/dep/spdlog/lib/debug"
        }

        links
        {
            "mono-2.0-sgen.lib",
            "rttr_core_d",
            "spdlogd.lib",
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
            "%{prj.name}/dep/mono/lib/release",
            "%{prj.name}/dep/rttr/lib/release",
            "%{prj.name}/dep/spdlog/lib/release",
        }

        links
        {
            "mono-2.0-sgen.lib",
            "rttr_core",
            "spdlog.lib",
        }

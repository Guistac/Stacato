--=================================================================================================================

workspace "EthernetIPdev"
	architecture "x64"

	configurations{
		"Debug",
		"Release"
	}

	platforms { 
		"x64"
	}

	startproject "EthernetIPdev"

--=================================================================================================================

project "EthernetIPdev"
	location "" --directory of the project
	kind "consoleApp" --executable file
	language "C++"
	staticruntime "On"
	cppdialect "C++17"
	
	pchheader "pch.h"
	pchsource "%{prj.location}/src/Core/pch.cpp"

	targetdir "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}"
	objdir "%{wks.location}/bin/obj"

	debugdir "%{wks.location}/"

	files{
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp"
	}
	
	includedirs{
		"%{wks.location}/src",
		"%{wks.location}/src/Core",
		"%{wks.location}/dependencies/oscpp/include",
		"%{wks.location}/dependencies/asio/asio/include",
		"%{wks.location}/dependencies/glm/",
		"%{wks.location}/dependencies/glad/include",
		"%{wks.location}/dependencies/glfw/include",
		"%{wks.location}/dependencies/dearimgui/",
		"%{wks.location}/dependencies/implot/",
		"%{wks.location}/dependencies/tinyxml2/"
	}

	links{
		"glad",
		"glfw",
		"dearimgui",
		"implot",
		"tinyxml2"
	}
	
	defines{
		"ASIO_STANDALONE",
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "system:windows"
		systemversion "latest"
		defines{
			"_WIN32_WINNT=0x0601"
		}
		
	filter "configurations:Debug"
		defines {}
		symbols "On"

		
	filter "configurations:Release"
		defines {}
		optimize "On"



--=================================================================================================================

project "glad"
	location "dependencies/glad"
	kind "StaticLib"
	language "C"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
	objdir ("%{wks.location}/bin/obj");

	files {
		"%{prj.location}/include/glad/glad.h",
		"%{prj.location}/include/KHR/khrplatform.h",
		"%{prj.location}/src/glad.c"
	}

		
	includedirs{
		"%{prj.location}/include"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"


--=================================================================================================================


project "glfw"
	location "dependencies/glfw"
	kind "StaticLib"
	language "C"
	staticruntime "On"

	targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
	objdir ("%{wks.location}/bin/obj");

	files{
		"%{prj.location}/include/GLFW/glfw3.h",
		"%{prj.location}/include/GLFW/glfw3native.h",
		"%{prj.location}/src/glfw_config.h",
		"%{prj.location}/src/context.c",
		"%{prj.location}/src/init.c",
		"%{prj.location}/src/input.c",
		"%{prj.location}/src/monitor.c",
		"%{prj.location}/src/vulkan.c",
		"%{prj.location}/src/window.c"
	}

	filter "system:windows"
		systemversion "latest"

		files{
			"%{prj.location}/src/win32_init.c",
			"%{prj.location}/src/win32_joystick.c",
			"%{prj.location}/src/win32_monitor.c",
			"%{prj.location}/src/win32_time.c",
			"%{prj.location}/src/win32_thread.c",
			"%{prj.location}/src/win32_window.c",
			"%{prj.location}/src/wgl_context.c",
			"%{prj.location}/src/egl_context.c",
			"%{prj.location}/src/osmesa_context.c"
		}

		defines { 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"


--=================================================================================================================

project "dearimgui"
location "dependencies/dearimgui"
kind "StaticLib"
language "C++"
staticruntime "On"

targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
objdir ("%{wks.location}/bin/obj");

files{
	"%{prj.location}/imconfig.h",
	"%{prj.location}/imgui.h",
	"%{prj.location}/imgui.cpp",
	"%{prj.location}/imgui_draw.cpp",
	"%{prj.location}/imgui_internal.h",
	"%{prj.location}/imgui_widgets.cpp",
	"%{prj.location}/imgui_tables.cpp",
	"%{prj.location}/imgui_rectpack.h",
	"%{prj.location}/imgui_textedit.h",
	"%{prj.location}/imgui_truetype.h",
	"%{prj.location}/imgui_demo.cpp"
}

includedirs{
	"%{wks.location}/dependencies/glm"
}

filter "system:windows"
	systemversion "latest"

filter "configurations:Debug"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	runtime "Release"
	optimize "on"

--=================================================================================================================

project "implot"
location "dependencies/implot"
kind "StaticLib"
language "C++"
staticruntime "On"

targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
objdir ("%{wks.location}/bin/obj");

files{
	"%{prj.location}/implot.cpp",
	"%{prj.location}/implot.h",
	"%{prj.location}/implot_demo.cpp",
	"%{prj.location}/implot_internal.h",
	"%{prj.location}/implot_items.cpp",
}

includedirs{
	"%{wks.location}/dependencies/dearimgui/",
	"%{wks.location}/dependencies/glm"
}

filter "system:windows"
	systemversion "latest"

filter "configurations:Debug"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	runtime "Release"
	optimize "on"


	--=================================================================================================================

project "tinyxml2"
location "dependencies/tinyxml2"
kind "StaticLib"
language "C++"
staticruntime "On"

targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
objdir ("%{wks.location}/bin/obj");

files{
	"%{prj.location}/tinyxml2.h",
	"%{prj.location}/tinyxml2.cpp"
}

includedirs{
	"%{wks.location}/tinyxml2.h"
}

filter "system:windows"
	systemversion "latest"

filter "configurations:Debug"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	runtime "Release"
	optimize "on"
--=================================================================================================================

workspace "FieldbusDev"
	architecture "x64"

	configurations{
		"Debug",
		"Release"
	}
 
    filter "system:windows"
        defines "WIN32"
    
    filter "system:macosx"
        defines "MACOSX"

	startproject "FieldbusDev"

--=================================================================================================================

project "FieldbusDev"
	location "" --directory of the project
	kind "consoleApp" --executable file
	language "C++"
	cppdialect "C++17"

	targetdir "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}"
	objdir "%{wks.location}/bin/obj"

	debugdir "%{wks.location}/"

	files{
		"%{prj.location}/src/**.h",
		"%{prj.location}/src/**.cpp"
	}
	
	sysincludedirs{
		"%{wks.location}/src",
		"%{wks.location}/src/Core",
		"%{wks.location}/dependencies/glm/",
		"%{wks.location}/dependencies/glfw/include",
		"%{wks.location}/dependencies/dearimgui/",
		"%{wks.location}/dependencies/implot/",
		"%{wks.location}/dependencies/tinyxml2/",
		"%{wks.location}/dependencies/soem/soem",
		"%{wks.location}/dependencies/soem/osal",
		"%{wks.location}/dependencies/asio/asio/include/"
	}

	links{
		"SOEM",
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
			"WIN32",
			"_WINDOWS",
			"_WIN32_WINNT=0x0601"
		}
		sysincludedirs{
            "%{wks.location}/dependencies/glad/include",
			"%{wks.location}/dependencies/soem/oshw/win32/wpcap/Include",
			"%{wks.location}/dependencies/soem/osal/win32",
			"%{wks.location}/dependencies/soem/oshw/win32"
		}
        links{
            "glad",
            "Winmm.lib",
            "%{wks.location}/dependencies/soem/oshw/win32/wpcap/Lib/x64/wpcap.lib",
            "Ws2_32.lib",
        }
    
    filter "system:macosx"
        defines{}
        sysincludedirs{
            "%{wks.location}/dependencies/soem/osal/macosx",
            "%{wks.location}/dependencies/soem/oshw/macosx"
        }
        links{
            "Cocoa.framework",
            "IOKit.framework"
        }
		
	filter "configurations:Debug"
		defines {}
		symbols "On"

		
	filter "configurations:Release"
		defines {}
		optimize "On"

--=================================================================================================================

project "SOEM"
    location "dependencies/soem"
    kind "StaticLib"
    language "C"

    targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
    objdir ("%{wks.location}/bin/obj");

    files{
        "%{prj.location}/soem/*.h",
        "%{prj.location}/soem/*.c",
        "%{prj.location}/osal/osal.h"
    }

    sysincludedirs{
        "%{prj.location}/soem",
        "%{prj.location}/osal",
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        systemversion "latest"
        files{
            "%{prj.location}/oshw/win32/*.h",
            "%{prj.location}/oshw/win32/*.c",
            "%{prj.location}/osal/win32/*.h",
            "%{prj.location}/osal/win32/*.c"
        }
        sysincludedirs{
            "%{prj.location}/oshw/win32/wpcap/Include",
            "%{prj.location}/osal/win32",
            "%{prj.location}/oshw/win32"
        }
        defines{
            "WIN32"
        }
     
    filter "system:macosx"
        files{
            "%{prj.location}/oshw/macosx/*.h",
            "%{prj.location}/oshw/macosx/*.c",
            "%{prj.location}/osal/macosx/*.h",
            "%{prj.location}/osal/macosx/*.c"
        }
        sysincludedirs{
            "%{prj.location}/osal/macosx",
            "%{prj.location}/oshw/macosx"
        }
        defines{}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

--=================================================================================================================

--glad should only be used on windos, macos uses metal

project "glad"
    location "dependencies/glad"
    kind "StaticLib"
    language "C"
    systemversion "latest"

    targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
    objdir ("%{wks.location}/bin/obj");

    files {
        "%{prj.location}/include/glad/glad.h",
        "%{prj.location}/include/KHR/khrplatform.h",
        "%{prj.location}/src/glad.c"
    }
        
    sysincludedirs{
        "%{prj.location}/include"
    }

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
  
    filter "system:macosx"
        files{
            "%{prj.location}/src/cocoa_platform.h",
            "%{prj.location}/src/cocoa_init.m",
            "%{prj.location}/src/cocoa_joystick.h",
            "%{prj.location}/src/cocoa_joystick.m",
            "%{prj.location}/src/cocoa_monitor.m",
            "%{prj.location}/src/cocoa_window.m",
            "%{prj.location}/src/cocoa_time.c",
            "%{prj.location}/src/osmesa_context.h",
            "%{prj.location}/src/osmesa_context.c",
            "%{prj.location}/src/posix_thread.h",
            "%{prj.location}/src/posix_thread.c",
            "%{prj.location}/src/nsgl_context.h",
            "%{prj.location}/src/nsgl_context.m",
            "%{prj.location}/src/egl_context.h",
            "%{prj.location}/src/egl_context.c"
        }
        sysincludedirs{}
        defines{
            "_GLFW_COCOA"
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
        "%{prj.location}/imstb_rectpack.h",
        "%{prj.location}/imstb_textedit.h",
        "%{prj.location}/imstb_truetype.h",
        "%{prj.location}/imgui_demo.cpp"
    }

    sysincludedirs{
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

    targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
    objdir ("%{wks.location}/bin/obj");

    files{
        "%{prj.location}/implot.cpp",
        "%{prj.location}/implot.h",
        "%{prj.location}/implot_demo.cpp",
        "%{prj.location}/implot_internal.h",
        "%{prj.location}/implot_items.cpp",
    }

    sysincludedirs{
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

    targetdir ("%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.platform}/dependencies");
    objdir ("%{wks.location}/bin/obj");

    files{
        "%{prj.location}/tinyxml2.h",
        "%{prj.location}/tinyxml2.cpp"
    }

    sysincludedirs{
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

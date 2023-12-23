root_directory = './../../..'
workspace_root_directory = '%{wks.location}'

engine_directory = workspace_root_directory .. '/Engine'
source_directory = engine_directory .. '/Source'
runtime_source_directory = source_directory .. '/Runtime'
third_party_directory = source_directory .. '/ThirdParty'

build_output_directory = workspace_root_directory .. 'Build/Bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}'
intermediate_output_directory = workspace_root_directory .. 'Build/Intermediate/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}'

-- Workspace variables
workspace_directory = root_directory
workspace_macro_prefix = 'SB_'

function sb_reset_filter()
	filter {}
end

function sb_project(name)
  sb_reset_filter()
  project(name)
    sb_default_compiler_flags()
    filter { 'configurations:Debug' }
			symbols 'On'
			defines {
				'_DEBUG=1',
				workspace_macro_prefix .. 'DEBUG=1',
        workspace_macro_prefix .. 'RELEASE=0',
				workspace_macro_prefix .. 'DISTRIBUTION=0',
			}
		filter { 'configurations:Release' }
			optimize 'On'
			defines {
				'NDEBUG=1',
        workspace_macro_prefix .. 'DEBUG=0',
				workspace_macro_prefix .. 'RELEASE=1',
				workspace_macro_prefix .. 'DISTRIBUTION=0',
			}
		filter { 'configurations:Distribution' }
			optimize 'On'
			defines {
				'NDEBUG=1',
        workspace_macro_prefix .. 'DEBUG=0',
				workspace_macro_prefix .. 'RELEASE=0',
				workspace_macro_prefix .. 'DISTRIBUTION=1',
			}
		filter { 'system:windows' }
      defines {
        workspace_macro_prefix .. 'OS_WINDOWS=1',
        'WIN32_LEAN_AND_MEAN',
        'NOMINMAX',
      }
      linkoptions { '-IGNORE:4099' }
			disablewarnings { '4251' }
    filter { 'kind:StaticLib' }
			defines {
				workspace_macro_prefix .. 'LIBRARY=1',
				workspace_macro_prefix .. 'LIBRARY_STATIC=1',
				workspace_macro_prefix .. 'LIBRARY_SHARED=0',
        workspace_macro_prefix .. 'LIBRARY_EXPORT=1',
        workspace_macro_prefix .. 'EXECUTABLE=0',
        workspace_macro_prefix .. 'EXECUTABLE_CONSOLE=0',
        workspace_macro_prefix .. 'EXECUTABLE_WINDOWED=0',
			}
		filter { 'kind:SharedLib' }
			defines {
				workspace_macro_prefix .. 'LIBRARY=1',
				workspace_macro_prefix .. 'LIBRARY_STATIC=0',
				workspace_macro_prefix .. 'LIBRARY_SHARED=1',
        workspace_macro_prefix .. 'LIBRARY_EXPORT=1',
        workspace_macro_prefix .. 'EXECUTABLE=0',
        workspace_macro_prefix .. 'EXECUTABLE_CONSOLE=0',
        workspace_macro_prefix .. 'EXECUTABLE_WINDOWED=0',
			}
		filter { 'kind:ConsoleApp' }
			defines {
				workspace_macro_prefix .. 'LIBRARY=0',
				workspace_macro_prefix .. 'LIBRARY_STATIC=0',
				workspace_macro_prefix .. 'LIBRARY_SHARED=0',
        workspace_macro_prefix .. 'LIBRARY_EXPORT=0',
        workspace_macro_prefix .. 'EXECUTABLE=1',
        workspace_macro_prefix .. 'EXECUTABLE_CONSOLE=1',
        workspace_macro_prefix .. 'EXECUTABLE_WINDOWED=0',
			}
		filter { 'kind:WindowedApp' }
			defines {
				workspace_macro_prefix .. 'LIBRARY=0',
				workspace_macro_prefix .. 'LIBRARY_STATIC=0',
				workspace_macro_prefix .. 'LIBRARY_SHARED=0',
        workspace_macro_prefix .. 'LIBRARY_EXPORT=0',
        workspace_macro_prefix .. 'EXECUTABLE=1',
        workspace_macro_prefix .. 'EXECUTABLE_CONSOLE=0',
        workspace_macro_prefix .. 'EXECUTABLE_WINDOWED=1',
			}
  sb_reset_filter()
end

function sb_default_compiler_flags()
	targetdir(build_output_directory .. '/%{prj.name}')
	objdir(intermediate_output_directory .. '/%{prj.name}')
	characterset 'MBCS'
	floatingpoint 'Fast' -- Change to 'Fast' for performance or 'Strict' in case of floating point issues
	staticruntime 'Off'
	flags {
		'MultiProcessorCompile'
	}
end

function sb_default_files()
	files {
		'%{prj.location}/**.c',
		'%{prj.location}/**.cpp',
		'%{prj.location}/**.h',
		'%{prj.location}/**.hpp',
	}
end

function sb_default_include_directories()
	includedirs {
		'%{prj.location}/Public',
		'%{prj.location}/Private',
	}
end

function sb_language_cpp()
	language 'C++'
	cppdialect 'C++20'
end

function sb_precompiled_header()
  files {
    '%{prj.location}/pch.cpp',
    '%{prj.location}/pch.h',
  }
  pchheader('pch.h')
  pchsource '%{prj.location}/pch.cpp'
end

function sb_link_project(name, include_directory)
	if not include_directory then
		include_directory = runtime_source_directory .. '/' .. name .. '/Public'
	end
	includedirs {
		include_directory,
	}
	links {
		name
	}
end

function sb_project_location(name)
	location(runtime_source_directory .. '/' .. name)
end

function sb_third_party_project_location(name)
	location(third_party_directory .. '/' .. name)
end

function sb_copy_output_to_directory(directory)
	postbuildcommands {
		('{COPY} %{cfg.buildtarget.relpath} "' .. directory .. '"')
	}
end

function sb_copy_dll_from_project(project_name, directory)
  postbuildcommands {
    ('{COPY} ' .. build_output_directory .. '/' .. project_name .. '/*.dll "' .. directory .. '"')
  }
  filter { 'configurations:Debug' }
    postbuildcommands {
      ('{COPY} ' .. build_output_directory .. '/' .. project_name .. '/*.pdb "' .. directory .. '"')
    }
  sb_reset_filter()
end

function sb_copy_dll_to_directory(directory)
	postbuildcommands {
		('{COPY} ' .. build_output_directory .. '/%{prj.name}/*.dll "' .. directory .. '"')
	}
  filter { 'configurations:Debug' }
    postbuildcommands {
      ('{COPY} ' .. build_output_directory .. '/%{prj.name}/*.pdb "' .. directory .. '"')
    }
  sb_reset_filter()
end

function sb_executable(name)
  sb_reset_filter()
	sb_project(name)
    sb_default_files()
		sb_default_include_directories()
		sb_project_location(name)
		sb_language_cpp()
    filter { 'configurations:Debug' }
      kind 'ConsoleApp'
    filter { 'configurations:Release' }
      kind 'ConsoleApp'
    filter { 'configurations:Distribution' }
      kind 'WindowedApp'
  sb_reset_filter()
end

function sb_shared_library(name)
  sb_reset_filter()
  sb_project(name)
    sb_default_files()
    sb_default_include_directories()
		kind 'SharedLib'
    sb_project_location(name)
    sb_language_cpp()
  sb_reset_filter()
end

function sb_static_library(name)
  sb_reset_filter()
  sb_project(name)
    sb_default_files()
    sb_default_include_directories()
    kind 'StaticLib'
    sb_project_location(name)
    sb_language_cpp()
  sb_reset_filter()
end
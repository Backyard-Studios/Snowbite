sb_project('spdlog')
	kind 'StaticLib'
	sb_language_cpp()
	sb_third_party_project_location('spdlog')

	files {
		'%{prj.location}/include/**.h',
		'%{prj.location}/src/**.cpp',
	}

	includedirs {
		'%{prj.location}/include',
	}

	defines {
		'SPDLOG_COMPILED_LIB'
	}
sb_project('imgui')
	kind 'StaticLib'
	sb_language_cpp()
	sb_third_party_project_location('imgui')

	files {
		'%{prj.location}/imgui.h',
    '%{prj.location}/imgui.cpp',
    '%{prj.location}/imgui_demo.cpp',
    '%{prj.location}/imgui_draw.cpp',
    '%{prj.location}/imgui_widgets.cpp',
    '%{prj.location}/imgui_tables.cpp',
    '%{prj.location}/imgui_internal.h',
    '%{prj.location}/imstb_rectpack.h',
    '%{prj.location}/imstb_textedit.h',
    '%{prj.location}/imstb_truetype.h',
    '%{prj.location}/backends/imgui_impl_dx12.h',
    '%{prj.location}/backends/imgui_impl_dx12.cpp',
    '%{prj.location}/backends/imgui_impl_win32.h',
    '%{prj.location}/backends/imgui_impl_win32.cpp',
	}

	includedirs {
		'%{prj.location}/',
	}
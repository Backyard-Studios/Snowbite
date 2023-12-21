sb_project('D3D12MemoryAllocator')
	kind 'StaticLib'
	sb_language_cpp()
	sb_third_party_project_location('D3D12MemoryAllocator')

	files {
		'%{prj.location}/include/D3D12MemAlloc.h',
		'%{prj.location}/src/D3D12MemAlloc.cpp',
	}

	includedirs {
		'%{prj.location}/include',
	}
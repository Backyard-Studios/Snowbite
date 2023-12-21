sb_runtime_executable('Sandbox')
  sb_link_project('Engine')
  sb_copy_dll_from_project('Engine', build_output_directory .. '/%{prj.name}')
  sb_link_project('imgui', third_party_directory .. '/imgui')
  sb_link_project('spdlog', third_party_directory .. '/spdlog/include')
  sb_link_project('D3D12MemoryAllocator', third_party_directory .. '/D3D12MemoryAllocator/include')

  files {
    '%{prj.location}/Assets/**.*',
  }

  filter { 'files:**.hlsl' }
		buildaction 'None'
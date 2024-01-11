sb_executable('Engine')
  sb_precompiled_header()

  sb_link_project('spdlog', third_party_directory .. '/spdlog/include')

  includedirs {
    third_party_directory .. '/AgilitySDK/include',
  }

  links {
    'dxgi.lib',
    'd3d12.lib',
    'd3dcompiler.lib',
    'dxguid.lib',
  }

  postbuildcommands {
    ('{COPY} "' .. third_party_directory .. '/AgilitySDK/bin/x64/*.dll" "' .. build_output_directory .. '/Editor/D3D12"'),
  }
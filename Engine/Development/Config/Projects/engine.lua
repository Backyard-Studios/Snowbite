sb_executable('Engine')
  sb_precompiled_header()

  includedirs {
    third_party_directory .. '/AgilitySDK/include',
  }

  links {
    'dxgi.lib',
    'd3dcompiler.lib',
    'dxguid.lib',
  }

  postbuildcommands {
    ('{COPY} "' .. third_party_directory .. '/AgilitySDK/bin/x64/*.dll" "' .. build_output_directory .. '/%{prj.name}/D3D12"')
  }
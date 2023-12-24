sb_executable('Engine')
  sb_precompiled_header()

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
    ('{COPY} "' .. third_party_directory .. '/AgilitySDK/bin/x64/*.dll" "' .. build_output_directory .. '/%{prj.name}/D3D12"')
  }

  filter 'configurations:Debug'
    defines {
      'SB_D3D12_ENABLE_DEBUG_LAYER=1'
    }

  filter 'configurations:Release'
    defines {
      'SB_D3D12_ENABLE_DEBUG_LAYER=0'
    }

  filter 'configurations:Distribution'
    defines {
      'SB_D3D12_ENABLE_DEBUG_LAYER=0'
    }
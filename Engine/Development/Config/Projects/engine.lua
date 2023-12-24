sb_executable('Engine')
  sb_precompiled_header()

  links {
    'dxgi.lib',
    'd3d12.lib',
    'd3dcompiler.lib',
    'dxguid.lib',
  }
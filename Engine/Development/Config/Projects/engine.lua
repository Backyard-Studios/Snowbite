sb_shared_library('Engine')
  sb_copy_dll_to_directory(build_output_directory .. '/Sandbox')
  sb_precompiled_header()

  sb_link_project('spdlog', third_party_directory .. '/spdlog/include')
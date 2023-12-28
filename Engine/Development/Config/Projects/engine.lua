sb_shared_library('Engine')
  sb_precompiled_header()

  sb_copy_dll_to_directory(build_output_directory .. '/Editor')
sb_runtime_executable('Sandbox')
  sb_link_project('Engine')
  sb_copy_dll_from_project('Engine', build_output_directory .. '/%{prj.name}')
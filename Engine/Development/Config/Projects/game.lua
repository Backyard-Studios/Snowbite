sb_project('GameModule')
  sb_default_files()
  sb_default_include_directories()
  sb_project_location('GameModule')
  sb_language_cpp()
  kind 'SharedLib'

  sb_copy_dll_to_directory(build_output_directory .. '/Engine')
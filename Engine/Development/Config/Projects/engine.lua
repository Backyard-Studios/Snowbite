sb_executable('Engine')
  sb_precompiled_header()

  sb_link_project('spdlog', third_party_directory .. '/spdlog/include')

  removefiles { '%{prj.location}/**/PAL/Windows/**' }

  filter { 'system:windows' }
    files {
      '%{prj.location}/**/PAL/Windows/**.cpp',
      '%{prj.location}/**/PAL/Windows/**.h',
    }
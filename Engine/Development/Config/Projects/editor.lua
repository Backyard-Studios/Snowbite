sb_executable('Editor')
  sb_link_shared_library('Engine')
  sb_link_project('spdlog', third_party_directory .. '/spdlog/include')
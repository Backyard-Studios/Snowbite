include 'helpers.lua'
include 'workspace.lua'

newoption {
  trigger = "build-type",
  value = "TYPE",
  description = "Choose which build type to use",
  allowed = {
     { "dist",    "Configures the project to be in distribution mode. The game module will be built as static libraries and will be linked into the engine executable" },
     { "editor",  "Configures the project to be in editor mode. The game module will be built as a shared library and will be loaded at runtime" },
  },
  default = "editor"
}

group 'Engine'
  include 'Projects/engine.lua'

group 'GameModules'
  include 'Projects/game.lua'

group 'ThirdParty'
  include 'Projects/spdlog.lua'
include 'helpers.lua'
include 'workspace.lua'

group 'Engine'
  include 'Projects/engine.lua'

group 'Applications'
  include 'Projects/sandbox.lua'

group 'GameModules'

group 'ThirdParty'
  include 'Projects/spdlog.lua'
  include 'Projects/imgui.lua'
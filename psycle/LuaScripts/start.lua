-- psycle start script

-- viewport options 
-- psycle.CHILDVIEWPORT : display at start in mainwindow
-- psycle.FRAMEVIEWPORT : display at start in window

-- userinterface options 
-- psycle.SDI : single plugin architecture, life time ending with program close
-- psycle.MDI : multiple plugin architecture, life time ending with window x close

-- todo not implemented yet (atm psycle.LAZY as default)
-- creation
-- psycle.CREATELAZY : plugin created at first link call
-- psycle.CREATEPROGRAMSTART: plugin created at program start


-- @plugineditor.lua

local link = {
  label = "New Plugin Editor",
  plugin = "plugineditor.lua",
  viewport = psycle.FRAMEVIEWPORT,
  userinterface = psycle.MDI
}
psycle.addmenu("view", link)  


-- @pianoroll.lua
link = {
  label = "Pianoroll",
  plugin = "pianoroll.lua",
  viewport = psycle.CHILDVIEWPORT,
  userinterface = psycle.SDI
}
psycle.addmenu("view", link)

-- @scopes.lua
link = {
  label = "Scopes",
  plugin = "scopes.lua",
  viewport = psycle.TOOLBARVIEWPORT,
  userinterface = psycle.SDI,
  creation = psycle.CREATEATSTART
}
psycle.addmenu("view", link)




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

-- local extension = require("psycle.extension")

--local link = {
--  label = "keys",
--  plugin = "type.lua",
--  viewport = psycle.FRAMEVIEWPORT,
--  userinterface = psycle.MDI
--}
--psycle.replacemenu("help", 2, link)

local link = {
  label = "New Plugin Editor",
  plugin = "plugineditor.lua",
  viewport = psycle.CHILDVIEWPORT,
  userinterface = psycle.MDI
}
psycle.addmenu("view", link)  



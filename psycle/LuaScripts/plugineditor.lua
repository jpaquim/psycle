-- scintilla based editor for psycle lua plugins

function psycle.install()
  return 
[[
link = {
  label = "Plugin Editor",
  plugin = "plugineditor.lua",
  viewport = psycle.CHILDVIEWPORT,
  userinterface = psycle.MDI
}
psycle.addmenu("view", link)  
]]  
end

function psycle.info()
  return { 
    vendor  = "psycle",
    name    = "Plugineditor",
    mode    = require("psycle.machine").HOST,
    version = 0,
    api     = 0
  }
end

function psycle.start()  
  psycle.setmachine(require("plugineditor.machine"))
end


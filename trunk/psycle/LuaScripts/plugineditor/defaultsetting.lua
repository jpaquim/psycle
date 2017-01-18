local orderedtable = require("psycle.orderedtable")
local fontinfo = require("psycle.ui.fontinfo")
local stock = require("psycle.stock")
local property = require("property")

local settings = orderedtable.new()  

settings.meta = {
  name = "plugineditor",
  version = 0.2
}

settings.tab = {}
settings.tab.properties = orderedtable.new()
settings.tab.properties.tabwidth = property:new(32, "Tabwidth")

settings.tab.label = "Tab"

settings.textpage = {}
settings.textpage.properties = orderedtable.new()
settings.textpage.properties.mapcapslocktoctrl = property:new(true, "Map Capslock to Control")
settings.textpage.properties.haslinenumbers = property:new(false, "Show Linenumbers")
settings.textpage.label = "textpage"

settings.lualexer = {}
settings.lualexer.properties = orderedtable.new()
settings.lualexer.properties.keywords = property:new("and break do else elseif end false for function if in local nil not or repeat return then true until while", "Keywords")

settings.lualexer.label = "Lua Lexer"

return settings

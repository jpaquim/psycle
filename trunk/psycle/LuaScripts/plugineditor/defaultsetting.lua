local orderedtable = require("psycle.orderedtable")
local property = require("property")

local settings = orderedtable.new()  

settings.meta = {
  name = "plugineditor",
  version = 0.1
}

settings.general = {}
settings.general.label = "General"
settings.general.properties = orderedtable.new()
settings.general.properties.haslinenumbers = property:new(false, "Show Linenumbers")
settings.general.properties.linenumberforegroundcolor = property:new(0x939393, "Linenumber Foreground Color", "color")
settings.general.properties.selbackgroundcolor = property:new(0x232323, "Selection Background Color", "color")
settings.general.properties.marginbackgroundcolor = property:new(0x232323, "Margin Background Color", "color")
settings.general.properties.backgroundcolor = property:new(0x292929, "Background Color", "color")
settings.general.properties.foregroundcolor = property:new(0xcacaca, "Foreground Color", "color")
settings.general.properties.linenumberbackgroundcolor = property:new(0x232323, "Linenumber Background Color", "color")
settings.general.properties.textcolor = property:new(0xffff00, "Textcolor", "color")

settings.general.children = orderedtable.new()
settings.general.children.ui = {}
settings.general.children.ui.label = "Ui Components"
settings.general.children.ui.properties = orderedtable.new()
settings.general.children.ui.properties.editforegroundcolor = property:new(0xB0D8B1, "Edit Foreground Color", "color")
settings.general.children.ui.properties.editbackgroundcolor = property:new(0x323232, "Edit Background Color", "color")


settings.tab = {}
settings.tab.label = "Tab"
settings.tab.properties = orderedtable.new()
settings.tab.properties.tabwidth = property:new(32, "Tabwidth")

settings.lualexer = {}
settings.lualexer.label = "Lua Lexer"
settings.lualexer.properties = orderedtable.new()
settings.lualexer.properties.keywords = property:new("and break do else elseif end false for function if in local nil not or repeat return then true until while", "Keywords")
settings.lualexer.properties.commentcolor = property:new(0xB0D8B1, "Comment Color", "color")
settings.lualexer.properties.commentlinecolor = property:new(0xB0D8B1, "Comment Line Color", "color")
settings.lualexer.properties.commentdoccolor = property:new(0xB0D8B1, "Comment Doc Color", "color")
settings.lualexer.properties.foldingcolor = property:new(0x939393, "Folding Color", "color")
settings.lualexer.properties.operatorcolor = property:new(0xA6B5E1, "Operator Color", "color")
settings.lualexer.properties.wordcolor = property:new(0xA6B5E1, "Word Color", "color")
settings.lualexer.properties.stringcolor = property:new(0xA0A0A0, "String Color", "color")
settings.lualexer.properties.identifiercolor = property:new(0xCACACA, "Identifier Color", "color")
settings.lualexer.properties.numbercolor = property:new(0xffa54b, "Number Color", "color")



settings.fileexplorer = {}
settings.fileexplorer.properties = orderedtable.new()
settings.fileexplorer.properties.backgroundcolor = property:new(0x292929, "Background Color", "color")
settings.fileexplorer.properties.foregroundcolor = property:new(0xffffff, "Textcolor", "color")



 
return settings
 

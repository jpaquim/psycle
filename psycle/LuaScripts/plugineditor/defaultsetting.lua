local orderedtable = require("psycle.orderedtable")
local fontinfo = require("psycle.ui.fontinfo")
local stock = require("psycle.stock")
local property = require("property")

local settings = orderedtable.new()  

settings.meta = {
  name = "plugineditor",
  version = 0.2
}

settings.general = {}
settings.general.children = orderedtable.new()
settings.general.children.ui = {}
settings.general.children.ui.children = orderedtable.new()
settings.general.children.ui.children.advancededit = {}
settings.general.children.ui.children.advancededit.properties = orderedtable.new()
settings.general.children.ui.children.advancededit.properties.color = property:new(0xffb0d8b1, "Text Color", "color", stock.color.PVFONT)
settings.general.children.ui.children.advancededit.properties.backgroundcolor = property:new(0xff323232, "Background Color", "color", stock.color.PVROW)
settings.general.children.ui.children.advancededit.properties.hovercolor = property:new(0xffb0d8b1, "Hover Text Color", "color", stock.color.PVFONT)
settings.general.children.ui.children.advancededit.properties.hoverbackgroundcolor = property:new(0xff323232, "Hover Background Color", "color", stock.color.PVROW4BEAT)
settings.general.children.ui.children.advancededit.properties.activecolor = property:new(0xffb0d8b1, "Active Text Color", "color", stock.color.PVFONT)
settings.general.children.ui.children.advancededit.properties.activebackgroundcolor = property:new(0xff323232, "Active Background Color", "color", stock.color.PVROW4BEAT)

settings.general.children.ui.children.advancededit.label = "advancededit"
settings.general.children.ui.label = "Ui Components"

settings.general.properties = orderedtable.new()
settings.general.properties.haslinenumbers = property:new(false, "Show Linenumbers")
settings.general.properties.linenumberforegroundcolor = property:new(0xff939393, "Linenumber Foreground Color", "color", stock.color.PVFONT)
settings.general.properties.selbackgroundcolor = property:new(0xff232323, "Selection Background Color", "color")
settings.general.properties.foldingbackgroundcolor = property:new(0xff333333, "Folding Background Color", "color", stock.color.PVROW4BEAT)
settings.general.properties.marginbackgroundcolor = property:new(0xff333333, "Margin Background Color", "color", stock.color.PVBACKGROUND)
settings.general.properties.backgroundcolor = property:new(0x0, "Background Color", "color", stock.color.PVBACKGROUND)
settings.general.properties.foregroundcolor = property:new(0xffcacaca, "Foreground Color", "color", stock.color.PVFONT)
settings.general.properties.linenumberbackgroundcolor = property:new(0xff232323, "Linenumber Background Color", "color", stock.color.PVROWBEAT)
settings.general.properties.textcolor = property:new(0xffffff00, "Textcolor", "color", stock.color.PVFONT)
settings.general.properties.caretlinebackgroundcolor = property:new(0xffffff00, "Caret Linebackgroundcolor", "color", stock.color.PVROW)
settings.general.properties.caretcolor = property:new(0xffffff00, "Caret Color", "color", stock.color.PVFONT)
settings.general.properties.font = property:new(fontinfo:new("courier", 12, 1), "Font", "fontinfo")

settings.general.label = "General"

settings.tab = {}
settings.tab.properties = orderedtable.new()
settings.tab.properties.tabwidth = property:new(32, "Tabwidth")

settings.tab.label = "Tab"

settings.statusbar = {}
settings.statusbar.properties = orderedtable.new()
settings.statusbar.properties.color = property:new(0xffb0d8b1, "Text Color", "color", stock.color.PVFONT)
settings.statusbar.properties.statuscolor = property:new(0xffb0d8b1, "Status Color", "color", stock.color.PVSELECTION)
settings.statusbar.properties.backgroundcolor = property:new(0xff323232, "Background Color", "color", stock.color.PVROWBEAT)

settings.statusbar.label = "statusbar"

settings.output = {}
settings.output.properties = orderedtable.new()
settings.output.properties.color = property:new(0xffb0d8b1, "Text Color", "color", stock.color.PVFONT)
settings.output.properties.backgroundcolor = property:new(0xff323232, "Background Color", "color", stock.color.PVROWBEAT)

settings.output.label = "output"

settings.textpage = {}
settings.textpage.properties = orderedtable.new()
settings.textpage.properties.mapcapslocktoctrl = property:new(true, "Map Capslock to Control")
settings.textpage.label = "textpage"

settings.lualexer = {}
settings.lualexer.properties = orderedtable.new()
settings.lualexer.properties.keywords = property:new("and break do else elseif end false for function if in local nil not or repeat return then true until while", "Keywords")
settings.lualexer.properties.commentcolor = property:new(0xffb0d8b1, "Comment Color", "color")
settings.lualexer.properties.commentlinecolor = property:new(0xffb0d8b1, "Comment Line Color", "color")
settings.lualexer.properties.commentdoccolor = property:new(0xffb0d8b1, "Comment Doc Color", "color")
settings.lualexer.properties.foldingmarkerforecolor = property:new(0xff939393, "Folding Marker Forecolor", "color", stock.color.PVFONT)
settings.lualexer.properties.foldingmarkerbackcolor = property:new(0xff939393, "Folding Marker Backcolor", "color", stock.color.PVFONT)
settings.lualexer.properties.operatorcolor = property:new(0xffa6b5e1, "Operator Color", "color")
settings.lualexer.properties.wordcolor = property:new(0xffa6b5e1, "Word Color", "color")
settings.lualexer.properties.stringcolor = property:new(0xffa0a0a0, "String Color", "color")
settings.lualexer.properties.identifiercolor = property:new(0xffcacaca, "Identifier Color", "color")
settings.lualexer.properties.numbercolor = property:new(0xffffa54b, "Number Color", "color")

settings.lualexer.label = "Lua Lexer"

return settings

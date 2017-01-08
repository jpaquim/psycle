local orderedtable = require("psycle.orderedtable")
local fontinfo = require("psycle.ui.fontinfo")
local stock = require("psycle.stock")
local property = require("property")

local machinecreate = orderedtable.new()  

machinecreate.meta = {
  name = "machinecreate",
  version = 0.1
}

machinecreate.fx = {}
machinecreate.fx.label = "fx"
machinecreate.fx.isplugin = true
machinecreate.fx.properties = orderedtable.new()
machinecreate.fx.properties.pluginname = property:new("", "pluginname")
machinecreate.fx.properties.vendor = property:new("", "vendor")
machinecreate.fx.properties.machmode = property:new("machine.FX", "machmode"):preventedit()
machinecreate.fx.outputs = {
  {template = "plugin.lu$", path = "machine.lua"}
}

machinecreate.generator = {}
machinecreate.generator.label = "generator"
machinecreate.generator.isplugin = true
machinecreate.generator.properties = orderedtable.new()
machinecreate.generator.properties.pluginname = property:new("", "pluginname")
machinecreate.generator.properties.vendor = property:new("", "vendor")
machinecreate.generator.properties.machmode = property:new("machine.GENERATOR", "machmode"):preventedit()
machinecreate.generator.outputs = {
  {template = "plugin.lu$", path = "machine.lua"}
}

machinecreate.hostextension = {}
machinecreate.hostextension.label = "hostextension"
machinecreate.hostextension.isplugin = true
machinecreate.hostextension.general = orderedtable.new()
machinecreate.hostextension.general.machinename = property:new("", "machinename")
machinecreate.hostextension.general.vendor = property:new("", "vendor")
machinecreate.hostextension.general.machmode = property:new("machine.HOST", "machmode"):preventedit()
machinecreate.hostextension.outputs = {
  {template = "hostextension.lu$", path = "machine.lua"},  
  {template = "canvas.lu$", path = "maincanvas.lua"},
  {template = "defaultsetting.lu$", path = "defaultsetting.lua"},
  {template = "global.lu$", path = "global.lua"}
}
machinecreate.hostextension.outputs[2].properties = orderedtable.new()
machinecreate.hostextension.outputs[2].properties.classname = property:new("maincanvas", "classname")
local maincanvasrequirecode =
[[
local titlebaricons = require("psycle.ui.canvas.titlebaricons")
local systems = require("psycle.ui.systems")
local toolicon = require("psycle.ui.canvas.toolicon")
]]
machinecreate.hostextension.outputs[2].properties.requirecode = property:new(maincanvasrequirecode):preventedit()
local maincanvasinitcode = [[
  self:invalidatedirect()
  self:addornament(ornamentfactory:createfill(0xFFCACACA))
  self.toolbar = group:new(self):setautosize(false, true):setalign(item.ALTOP)
  self.titlebaricons = titlebaricons:new(self.toolbar):setalign(item.ALRIGHT)  
  local systems = systems:new()  
  local setting = psycle.proxy.settingsmanager:setting()
  systems:setstyleclass(toolicon.windowtype, setting.general.children.ui.children.toolicon.properties) 
  self:addornament(ornamentfactory:createfill(setting.general.properties.backgroundcolor:value()))
  systems:new():updatewindows()
]]
machinecreate.hostextension.outputs[2].properties.initcode = property:new(maincanvasinitcode):preventedit()

machinecreate.class = {}
machinecreate.class.label = "class"
machinecreate.class.isplugin = false
machinecreate.class.properties = orderedtable.new()
machinecreate.class.properties.classname = property:new("", "classname")
machinecreate.class.outputs = {
  {template = "class.lu$", path = "$(classname).lua"}
}

machinecreate.window = {}
machinecreate.window.label = "window"
machinecreate.window.isplugin = false
machinecreate.window.properties = orderedtable.new()
machinecreate.window.properties.classname = property:new("", "classname")
machinecreate.window.outputs = {
  {template = "window.lu$", path = "$(classname).lua"}
}

machinecreate.group = {}
machinecreate.group.label = "group"
machinecreate.group.isplugin = false
machinecreate.group.properties = orderedtable.new()
machinecreate.group.properties.classname = property:new("", "classname")
machinecreate.group.outputs = {
  {template = "group.lu$", path = "$(classname).lua"}
}

machinecreate.canvas = {}
machinecreate.canvas.label = "canvas"
machinecreate.canvas.isplugin = false
machinecreate.canvas.properties = orderedtable.new()
machinecreate.canvas.properties.classname = property:new("", "classname")
machinecreate.canvas.outputs = {
  {template = "canvas.lu$", path = "$(classname).lua"}
}

return machinecreate
 

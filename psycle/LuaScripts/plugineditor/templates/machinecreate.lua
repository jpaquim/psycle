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
 

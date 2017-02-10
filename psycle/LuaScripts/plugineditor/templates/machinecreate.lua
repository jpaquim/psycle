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
machinecreate.fx.general = orderedtable.new()
machinecreate.fx.general.pluginname = property:new("", "pluginname")
machinecreate.fx.general.vendor = property:new("", "vendor")
machinecreate.fx.general.machmode = property:new("machine.FX", "machmode"):preventedit()
machinecreate.fx.outputs = {
  {template = "plugin.lu$", path = "machine.lua"}
}

machinecreate.generator = {}
machinecreate.generator.label = "generator"
machinecreate.generator.isplugin = true
machinecreate.generator.general = orderedtable.new()
machinecreate.generator.general.machinename = property:new("", "machinename")
machinecreate.generator.general.vendor = property:new("", "vendor")
machinecreate.generator.general.machmode = property:new("machine.GENERATOR", "machmode"):preventedit()
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
machinecreate.hostextension.maincanvas = {}
machinecreate.hostextension.maincanvas.properties = orderedtable.new()
machinecreate.hostextension.maincanvas.properties.classname = property:new("maincanvas", "classname")
machinecreate.hostextension.maincanvas.properties.requirecode = property:new([[
local titlebaricons = require("psycle.ui.canvas.titlebaricons")
local toolicon = require("psycle.ui.canvas.toolicon")
]]):preventedit()
machinecreate.hostextension.maincanvas.properties.initcode = property:new([[
  self:link(require("psycle.ui.standarduisheet"))
  self:invalidatedirect()
  self.toolbar = group:new(self)
                      :setautosize(false, true)
                      :setalign(item.ALTOP)
  self.titlebaricons = titlebaricons:new(self.toolbar)
                                    :setalign(item.ALRIGHT)  
]]):preventedit()
machinecreate.hostextension.outputs = {
  {template = "hostextension.lu$", path = "machine.lua"},  
  {template = "canvas.lu$", path = "maincanvas.lua", properties = machinecreate.hostextension.maincanvas.properties},
  {template = "defaultsetting.lu$", path = "defaultsetting.lua"},
  {template = "global.lu$", path = "global.lua"}
}

machinecreate.class = {}
machinecreate.class.label = "class"
machinecreate.class.isplugin = false
machinecreate.class.properties = orderedtable.new()
machinecreate.class.properties.classname = property:new("", "classname")
machinecreate.class.outputs = {
  {label = "general", template = "class.lu$", path = "$(classname).lua", properties = machinecreate.class.properties}
}

machinecreate.window = {}
machinecreate.window.label = "window"
machinecreate.window.isplugin = false
machinecreate.window.properties = orderedtable.new()
machinecreate.window.properties.classname = property:new("", "classname")
machinecreate.window.outputs = {
  {label = "general", template = "window.lu$", path = "$(classname).lua",  properties = machinecreate.window.properties}
}

machinecreate.group = {}
machinecreate.group.label = "group"
machinecreate.group.isplugin = false
machinecreate.group.properties = orderedtable.new()
machinecreate.group.properties.classname = property:new("", "classname")
machinecreate.group.outputs = {
  {label = "general", template = "group.lu$", path = "$(classname).lua", properties = machinecreate.group.properties}
}

machinecreate.canvas = {}
machinecreate.canvas.label = "canvas"
machinecreate.canvas.isplugin = false
machinecreate.canvas.properties = orderedtable.new()
machinecreate.canvas.properties.classname = property:new("", "classname")
machinecreate.canvas.outputs = {
  {label = "general", template = "canvas.lu$", path = "$(classname).lua", properties = machinecreate.canvas.properties}
}

return machinecreate
 

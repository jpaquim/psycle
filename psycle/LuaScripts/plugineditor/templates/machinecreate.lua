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
machinecreate.fx.properties = orderedtable.new()
machinecreate.fx.properties.pluginname = property:new("", "pluginname")
machinecreate.fx.properties.vendor = property:new("", "vendor")
machinecreate.fx.properties.machmode = property:new("machine.FX", "machmode"):preventedit()
machinecreate.fx.outputs = {
  {template = "plugin.lu$", path = "machine.lua"}
}

machinecreate.generator = {}
machinecreate.generator.label = "generator"
machinecreate.generator.properties = orderedtable.new()
machinecreate.generator.properties.pluginname = property:new("", "pluginname")
machinecreate.generator.properties.vendor = property:new("", "vendor")
machinecreate.generator.properties.machmode = property:new("machine.GENERATOR", "machmode"):preventedit()
machinecreate.generator.outputs = {
  {template = "plugin.lu$", path = "machine.lua"}
}

return machinecreate
 

-- psycle pluginselector (c) 2016 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local cfg = require("psycle.config"):new("PatternVisual")
local machine = require("psycle.machine")
local catcher = require("psycle.plugincatcher")
local signal = require("psycle.signal")
local node = require("psycle.node")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local listview = require("psycle.ui.listview")
local button = require("psycle.ui.button")
local edit = require("psycle.ui.edit")
local text = require("psycle.ui.text")
local filehelper = require("psycle.file")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local closebutton = require("closebutton")
local templateparser = require("templateparser")
local settings = require("settings")
local recursivenodeiterator = require("psycle.recursivenodeiterator")
local propertiesview = require("propertiesview")
local serpent = require("psycle.serpent")

local pluginselector = group:new()

function pluginselector:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function pluginselector:init()      
   self.doopen = signal:new()
   self.docreate = signal:new()
   self.tg = tabgroup:new(self)
                     :setalign(item.ALCLIENT)
                     :disableclosebutton()   
   self:initplugincreate()   
   self:initpluginlist()
   self:initcreategroup(self.pg)
   local closebutton = closebutton.new(self.tg.tabbar)
                                  :setalign(item.ALRIGHT)
   local that = self                                    
   function closebutton:onclick()
     self:resethover()
     that:hide():parent():updatealign()     
   end
end

function pluginselector:searchandfocusmachine(text)  
  local node = self:search(text)
  if node then
    self.openprevented = true 
    self.pluginlist:selectnode(node)
  self.openprevented = nil
  else        
    local selectednodes = self.pluginlist:selectednodes()
    for _, node in ipairs(selectednodes) do
    self.pluginlist:deselectnode(node)
  end
  end
end

function pluginselector:search(text)      
  local result = nil
  if text ~= "" then
    for element in recursivenodeiterator(self.rootnode).next do
    local i, j = string.find(element:text(), text, 1, true)  
    if i and i == 1 then
      result = element
    break;
    end  
    end
  end
  return result
end

function pluginselector:initplugincreate()
  self.pg = group:new()
                 :setalign(item.ALCLIENT)
                 :setautosize(false, false)  
  self.plugincreatelist = listview:new(self.pg)
                                  :setalign(item.ALLEFT)
                                  :setposition(rect:new(point:new(), dimension:new(200, 0)))
                                  :setautosize(false, false)
                                  :setbackgroundcolor(0xFF3E3E3E)
                                  :setcolor(0xFFFFFFFF)  
  self.tg:addpage(self.pg, "Create")
  self.plugincreatelistrootnode = node:new()    
  local f = loadfile(cfg:luapath().."\\plugineditor\\templates\\machinecreate.lua")
  if f then
    self.setting = f()    
  end  
  for name, settinggroup in self.setting:opairs() do
    if name ~= "meta" then  
      local label = ""
      if settinggroup.label then    
        label = settinggroup.label    
      else
        label = name
      end 
      local node = node:new(self.plugincreatelistrootnode)
                       :settext(label)    
      node.properties = settinggroup.properties
      node.outputs = settinggroup.outputs
    end
  end    
  self.plugincreatelist:setrootnode(self.plugincreatelistrootnode)  
  local that = self
  function self.plugincreatelist:onchange(node)  
    local oldpropertyview = nil
    if that.activenode then
      oldpropertyview = that.activenode.propertyview
    end
    that.activenode = node  
    that:showproperties(node)    
    if oldpropertyview then      
      oldpropertyview:hide()
    end
    that:parent():flagnotaligned():updatealign()  
  end  
  that:parent():flagnotaligned():updatealign()  
  return self.plugincreatelist    
end

function pluginselector:initcreategroup(parent)
  self.creategroup_ = group:new(parent)
                           :setautosize(false, false)                                                   
                           :setalign(item.ALCLIENT)
                           :setmargin(boxspace:new(5))
  self:initcreatebutton(self.creategroup_)  
end

function pluginselector:initcreatebutton(parent)
  local group = group:new(parent)
                     :setalign(item.ALBOTTOM)
                     :setautosize(false, true)
                     :setmargin(boxspace:new(0, 0, 10, 0))
  local createbutton = button:new(group)
                              :settext("Create")
                              :setalign(item.ALLEFT)
                              :setmargin(boxspace:new(0, 10, 0, 0))
                              :setposition(rect:new(point:new(), dimension:new(90, 20)))  
  local that = self             
  function createbutton:onclick()
    if that.activenode and that.activenode.propertyview then 
      that.activenode.propertyview:parseproperties()    
      that:oncreateplugin(that.activenode.properties,
                          that.activenode.outputs,
                          that.activenode.properties.pluginname:value())
    end
  end
  return group
end


function pluginselector:showproperties(node)  
  if node.propertyview then
    node.propertyview:show()    
  else   
    node.propertyview = 
        propertiesview:new(self.creategroup_, "create", node.properties)
                      :setautosize(false, false)                           
                      :setalign(item.ALCLIENT)
                      :addornament(ornamentfactory:createfill(0x2F2F2F))
  end
  self.creategroup_:show()
  self:parent():flagnotaligned():updatealign()
end

function pluginselector:oncreateplugin(properties, outputs, machinename)
  local env = {}
  for name, property in properties:opairs() do    
    env[name] = property:value()
  end  
  filehelper.mkdir(cfg:luapath().."\\"..machinename)  
  templateparser.work(cfg:luapath().."\\plugineditor\\templates\\pluginregister.lu$",
                      cfg:luapath().."\\"..machinename..".lua",
                      env)
  for _, output in pairs(outputs) do    
    local templatepath = cfg:luapath().."\\plugineditor\\templates\\"..output.template
    templateparser.work(templatepath,
                        cfg:luapath().."\\"..machinename.."\\"..output.path,
                        env)    
  end 
  self.docreate:emit(outputs, machinename)  
end  

function pluginselector:initpluginlist()
  self.pluginlist = listview:new()
                            :setautosize(false, false)
                            :setposition(rect:new(point:new(0, 0), dimension:new(0, 200)))
                            :setalign(item.ALTOP)
                            :setbackgroundcolor(0x3E3E3E)
                            :setcolor(0xFFFFFF)
                            :setmargin(boxspace:new(0, 5, 0, 0))
  self.tg:addpage(self.pluginlist, "All")
  local that = self  
  function self.pluginlist:onchange(node)
    if not that.openprevented then
      that:open(node)    
  end
  end  
  self:updatepluginlist()
end

function pluginselector:open(node)
  local dir = self:machinepath(node.info)        
  self:hide()         
  self.doopen:emit(dir, node.info:name(), node.info)  
end

function pluginselector:machinepath(info)  
  local file = io.open(info:dllname(), "r")  
  local str = ""
  for line in file:lines() do
    local pos = string.find(line, "psycle.setmachine")
  if pos then
      str = string.match(line, "require(%b())")   
      if str then
        str = str:sub(3, -3)      
        str = str:gsub("%.", "\\")      
        break
      end
    end 
  end
  file:close()    
  return str
end

function pluginselector:updatepluginlist()
  local catcher = catcher:new()
  local infos = catcher:infos()    
  self.rootnode = node:new()
  local lua_count = 0
  for i=1, #infos do
    if infos[i]:type() == machine.MACH_LUA then      
      local node = node:new():settext(infos[i]:name())      
      node.info = infos[i]      
      self.rootnode:add(node)      
    end
  end 
  self.pluginlist:setrootnode(self.rootnode) 
  self.pluginlist:updatelist()
end

return pluginselector
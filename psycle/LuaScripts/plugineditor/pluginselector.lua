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
local tree = require("psycle.ui.treeview")


local detailslistview = listview:new()

function detailslistview:new(parent, setting)  
  local c = listview:new(parent)
  setmetatable(c, self)
  self.__index = self
  c:init(setting)
  return c
end

function detailslistview:init(setting)
   self.setting = setting
   self:initlayout()
   self:initnodes()
end

function detailslistview:initlayout()
  self:setautosize(false, false)
  self:setalign(item.ALLEFT)
  self:setposition(rect:new(point:new(), dimension:new(200, 0)))
  self:setbackgroundcolor(0xFF3E3E3E)
  self:setcolor(0xFFFFFFFF)
end

function detailslistview:onchange(node)
  self:parent():onchange(node)  
end
 
 function detailslistview:initnodes()
  self.rootnode = node:new()    
  if self.setting.general ~= nil then    
    local node = node:new(self.rootnode):settext("general")
    node.props = self.setting.general    
  end
  for _, output in pairs(self.setting.outputs) do
    local label = output.label
    if label == nil then
      label = output.path
    end
    local node = node:new(self.rootnode):settext(label)
    node.props = output.properties    
  end
  self:setrootnode(self.rootnode)  
end

function detailslistview:parseproperties()    
  for node in recursivenodeiterator(self.rootnode).next do
    if node.propertyview ~= nil then       
       node.propertyview:parseproperties()
    end    
  end
end

local createdetails = group:new(parent)

function createdetails:new(parent, setting)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init(setting)
  return c
end

function createdetails:init(setting)
   self:initlayout()
   self.listview = detailslistview:new(self, setting)
   self.setting = setting
end

function createdetails:initlayout()
  self:setalign(item.ALCLIENT)
  self:setautosize(false, false) 
end

function createdetails:parseproperties()
  self.listview:parseproperties()
end

function createdetails:isplugin()
  return self.setting.isplugin;
end

function createdetails:onchange(node)
  if self.propertyview ~= nil then
    self.propertyview:hide()
  end
  if not node.propertyview then      
    node.propertyview = propertiesview:new(self, "create", node.props)
                                      :setautosize(false, false)                           
                                      :setalign(item.ALCLIENT)
                                      :addornament(ornamentfactory:createfill(0xFF444444))
  else
    node.propertyview:show();
  end
  self.propertyview = node.propertyview;
  self:flagnotaligned():updatealign()
end

local createlistview = listview:new()

function createlistview:new(parent)  
  local c = listview:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function createlistview:init()
   self:initlayout()
   self:initnodes()
end

function createlistview:initlayout()
  self:setautosize(false, false)
  self:setalign(item.ALLEFT)
  self:setposition(rect:new(point:new(), dimension:new(200, 0)))
  self:setbackgroundcolor(0xFF3E3E3E)
  self:setcolor(0xFFFFFFFF) 
end

function createlistview:onchange(node)
  self:parent():onchange(node)  
 end
 
 function createlistview:initnodes()
  self.rootnode = node:new()    
  local f = loadfile(cfg:luapath() .. "\\plugineditor\\templates\\machinecreate.lua")
  if f then
    self.setting = f()    
  end    
  for name, setting in self.setting:opairs() do
    if name ~= "meta" then  
      local label = ""
      if setting.label then    
        label = setting.label    
      else
        label = name
      end       
      local node = node:new(self.rootnode):settext(label)    
      node.setting = setting;
    end
  end    
  self:setrootnode(self.rootnode)  
end

local creategroup = group:new()

function creategroup:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function creategroup:init()
   self:initlayout()  
   self.createlist = createlistview:new(self); 
   self:initbuttons()
end

function creategroup:initlayout()
  self:setalign(item.ALCLIENT)
  self:setautosize(false, false)  
end

function creategroup:initbuttons()
  local group = group:new(self)
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
     local node = that.createlist:selected()
     if node ~= nil and node.details then
        node.details:parseproperties()               
        if node.details:isplugin() then         
           that:oncreateplugin(node.setting.general,
                               node.setting.outputs,
                               node.setting.general.machinename:value())
        else
           that:oncreatemodule(node.setting.properties,
                               node.setting.outputs)
        end      
     end    
  end                             
end

function creategroup:oncreateplugin(general, outputs, name)   
  filehelper.mkdir(cfg:luapath().."\\" .. name)  
  local env = { machinename = name, vendor = "psycle", machmode="machine.HOST"}
  templateparser.work(cfg:luapath().."\\plugineditor\\templates\\pluginregister.lu$",
                      cfg:luapath().."\\" .. name .. ".lua",
                      env)                      
  for _, output in pairs(outputs) do
    local env = {}
    if output.properties then
      for name, property in output.properties:opairs() do    
        env[name] = property:value()
      end  
    end
    if general then
      for name, property in general:opairs() do    
        env[name] = property:value()
      end  
    end
    local templatepath = cfg:luapath().."\\plugineditor\\templates\\" .. output.template
    templateparser.work(templatepath,
                        cfg:luapath().."\\" .. name .. "\\"..output.path,
                        env)
 end
 local catcher = catcher:new()
 catcher:rescannew()
 local info = catcher:info(name);
 info.machinepath = name
 self.main:open(info)
 self.main:updatepluginlist()
end

function creategroup:oncreatemodule(properties, outputs)
  local env = {}  
  for name, property in properties:opairs() do    
    env[name] = property:value()
  end     
  for _, output in pairs(outputs) do      
    local p = "(.-)$(%b())()"
    local path = output.path
    local text, propertyname = path:match(p)
    if propertyname then
      propertyname = propertyname:sub(2, -2)              
      path = path:gsub(p, env[propertyname])      
    end
    output.realpath = self.main.fileexplorer:path() .. "\\" .. path    
    local templatepath = cfg:luapath().."\\plugineditor\\templates\\"..output.template    
    templateparser.work(templatepath,  output.realpath, env)                         
  end 
  self.main.docreatemodule:emit(outputs)  
end

function creategroup:onchange(node)
  if self.details ~= nil then
    self.details:hide()
  end
  if not node.details then   
    node.details = createdetails:new(self, node.setting)   
  else
    node.details:show();
  end
  self.details = node.details;
  self:flagnotaligned():updatealign()
end

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
  self.docreatemodule = signal:new()
  self.tg = tabgroup:new(self)
                    :setalign(item.ALCLIENT)
                    :disableclosebutton()   
  self:initplugincreate()
  local closebutton = closebutton.new():setalign(item.ALRIGHT)
  self.tg.tabbar:insert(self.tg.tabbar.icon1, closebutton)
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
  self.plugincreate = creategroup:new()
  self.plugincreate.main = self  
  self.tg:addpage(self.plugincreate, "Create")
  self:initpluginlist()
  return self 
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
      that:open(node.info)    
  end
  end  
  self:updatepluginlist()
end

function pluginselector:open(info)
  info.machinepath = self:machinepath(info)
  self:hide()  
  self.doopen:emit(info)  
end

function pluginselector:machinepath(info)  
  local catcher = catcher:new()
  local result = catcher:machinepath(info:dllname())
  if result == "" then
    result = info:name():lower()
  end
  return result
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
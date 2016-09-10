-- psycle pluginselector (c) 2015 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local listview = require("psycle.ui.canvas.listview")
local node = require("psycle.node")
local filehelper = require("psycle.file")
local signal = require("psycle.signal")
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local settings = require("settings")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local closebutton = require("closebutton")
local text = require("psycle.ui.canvas.text")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local catcher = require("psycle.plugincatcher")
local machine = require("psycle.machine")

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
   self.tg = tabgroup:new(self):setalign(item.ALCLIENT)
   local g = group:new():setautosize(false, false)
   self:initpluginlist(g)
   self:initplugincreate()
   self.tg:addpage(g, "All")
   local closebutton = closebutton.new(self.tg.tabbar):setalign(item.ALRIGHT)
   local that = self
   function closebutton.closebtn:onmousedown()
     that:hide():parent():updatealign()     
  end 
end


function pluginselector:initplugincreate()
  local g = group:new():setautosize(false, false)
  self.tg:addpage(g, "Create")   
  self.plugincreatelist = listview:new(g)
                                  :setalign(item.ALCLIENT)
								  :setautosize(false, false)
                                  :setbackgroundcolor(0x3E3E3E)
                                  :settextcolor(0xFFFFFF)
  self.plugincreatelistrootnode = node:new()  
  local nodefx = node:new():settext("FX")
  self.plugincreatelistrootnode:add(nodefx)
  local nodegen = node:new():settext("Generator")          
  self.plugincreatelistrootnode:add(nodegen)
  self.plugincreatelist:setrootnode(self.plugincreatelistrootnode)
end

function pluginselector:initpluginlist(parent)
  self.pluginlist = listview:new(parent)
                            :setautosize(false, false)
                            :setpos(0, 0, 0, 200)
                            :setalign(item.ALTOP)
                            :setbackgroundcolor(0x3E3E3E)
                            :settextcolor(0xFFFFFF)
  local that = self
  local that = self
  function self.pluginlist:onchange(node)
    local dir = that:machinepath(node.info)        
    that:hide()        
    that.doopen:emit(dir, node.info:name(), node.info)  
  end  
  self:updatepluginlist()  
end

function pluginselector:machinepath(info)  
  local file = io.open(info:dllname(), "r")  
  local str = ""
  for line in file:lines() do
    str = string.match(line, "require(%b())")
    if str then
      str = str:sub(3, -3)      
      str = str:gsub("%.", "\\")      
      break
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
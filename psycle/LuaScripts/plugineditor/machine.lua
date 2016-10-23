-- psycle plugineditor (c) 2015 by psycledelics
-- File: machine.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()

machine = require("psycle.machine"):new()
  
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local frame = require("psycle.ui.frame")
local framealigner = require("psycle.ui.framealigner")
local maincanvas = require("maincanvas")
local sysmetrics = require("psycle.ui.systemmetrics")
local menubar = require("psycle.ui.menubar")
local mainmenu = require("mainmenu")
local serpent = require("psycle.serpent")
local plugincatcher = require("psycle.plugincatcher")
local node = require("psycle.node")
local menudesigner = require("menudesigner")
local canvas = require("psycle.ui.canvas")
local item = require("psycle.ui.item")
local cfg = require("psycle.config"):new("PatternVisual")
local project = require("project")
local templateparser = require("templateparser")
local settingsmanager = require("settingsmanager")
  
function machine:info()
  return { 
    vendor  = "psycle",
    name    = "Plugineditor",
    mode    = machine.HOST,
    version = 0,
    api     = 0
  }
end

-- help text displayed by the host
function machine:help()
  return "no help"
end

function machine:init(samplerate)  
   self.settingsmanager = settingsmanager:new("plugineditor")
   self.project = project:new()   
   self.maincanvas = maincanvas:new(self)   
   self:setcanvas(self.maincanvas)
   self:initmenu()
   self:settitle("empty")  
end

function machine:editmachineindex()
  local result = -1
  if self.project and self.project:plugininfo() then          
    result = self.project:pluginindex()
  end
  return result;
end

function machine:onexecute(msg, macidx, plugininfo, trace)    
  if msg ~= nil then
	self.project:setplugininfo(plugininfo):setpluginindex(macidx)
	self.maincanvas:fillinstancecombobox():setpluginindex(macidx)
	self.maincanvas:setoutputtext(msg)
	self.maincanvas:setcallstack(trace)
	for i=1, #trace do
    if self.maincanvas:openinfo(trace[i]) then
		break
	  end
	end      
	self.maincanvas:setplugindir(plugininfo)
	if plugininfo then
      self:settitle(plugininfo:name())
	end
  end
end

function machine:onactivated(viewport)  
  self.maincanvas:fillinstancecombobox()
  if self.project and self.project:pluginindex() ~= -1 then
    self.maincanvas:setpluginindex(self.project:pluginindex())
  end
end

function machine:ondeactivated()  
end

function machine:initmenu()   
   self.menubar = menubar:new()
   psycle.setmenubar(self.menubar)
   self.root = node:new()   
   self.menubar:setrootnode(self.root)   
   self.menus = mainmenu.menus()   
   self.root:add(self.menus.node);
   
   local that = self
   function self.menubar:onclick(node)       
      if node == that.menus.menudesigner then         
        that.menudesigner = menudesigner:new()
        if that.project and that.project:plugininfo() then          
          that.menudesigner.pluginname = that.project:plugininfo():name()
          local success, menu1 = pcall(require, that.project:plugininfo():name():lower()..".mainmenu1")
          if success then
            that.menudesigner:setmenu(menu1.menu().node1)
          else            
            that.menudesigner.rootnode:at(1):settext(that.project:plugininfo():name())
          end
        end        
        that.frame = frame:new()
                          :settitle("Menu Designer")
                          :setposition(rect:new(point:new(0, 0), dimension:new(400, 400)))
                          :setviewport(that.menudesigner)        
        function that.frame:onshow()          
          that.menudesigner.tree:editnode(that.menudesigner.tree:selected())
        end
        function that.frame:onclose()           
          that.frame = nil
        end
        that.frame:show(framealigner:new())        
      elseif node == that.menus.newmodule then
        local modulename = "newmodule"
        local env = {}
        env.modulename = modulename
        templateparser.work(cfg:luapath().."\\plugineditor\\templates\\module.lu$",
                            cfg:luapath().."\\"..modulename..".lua",
                            env)  
        that.maincanvas:openfromfile(cfg:luapath().."\\"..modulename..".lua")    
        that.maincanvas:updatealign():enablefls():invalidate()
      elseif node == that.menus.newgroup then
        local modulename = "newgroup"
        local env = {}
        env.modulename = modulename
        templateparser.work(cfg:luapath().."\\plugineditor\\templates\\group.lu$",
                            cfg:luapath().."\\"..modulename..".lua",
                            env)  
        that.maincanvas:openfromfile(cfg:luapath().."\\"..modulename..".lua")    
        that.maincanvas:updatealign():enablefls():invalidate()
      end
   end              
   self.menubar:update()
end

function machine:ontimer()
  self.maincanvas:onidle()
end

return machine
-- psycle plugineditor (c) 2015 by psycledelics
-- File: machine.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()

machine = require("psycle.machine"):new()

-- local codegen = require("codegen")
local maincanvas = require("maincanvas")
local frame = require("psycle.ui.canvas.frame")
local centertoscreen = require("psycle.ui.canvas.centertoscreen")
local sysmetrics = require("psycle.ui.systemmetrics")
local menubar = require("psycle.ui.menubar")
local mainmenu = require("mainmenu")
local serpent = require("psycle.serpent")
local plugincatcher = require("psycle.plugincatcher")
local node = require("psycle.node")
local menudesigner = require("menudesigner")
local canvas = require("psycle.ui.canvas")
local item = require("psycle.ui.canvas.item")
local cfg = require("psycle.config"):new("PatternVisual")
local project = require("project")

-- plugin info
function machine:info()
  return { 
    vendor  = "psycle",
    name    = "Plugineditor",
    mode    = machine.HOSTUI,
    version = 0,
    api     = 0
  }
end

-- help text displayed by the host
function machine:help()
  return "no help"
end

function machine:init(samplerate)  
   self.project = project:new()   
   self.maincanvas = maincanvas:new()   
   self.maincanvas.togglecanvas:connect(machine.togglecanvas, self)   
   self:setcanvas(self.maincanvas)
   self:initmenu();   
end

function machine:createframe()  
  self.frame = frame:new()
                    :setview(self.maincanvas)  
                    :settitle("Psycle Plugineditor")
  local that = self
  function self.frame:onclose(ev) 		       
    that:exit()
  end
end

function machine:onexecute(msg, macidx, plugininfo, trace)  
  if msg == nil then  
    self:openinmainframe()  
  else  
    self.project:setplugininfo(plugininfo):setpluginindex(macidx)
    self.maincanvas:fillinstancecombobox():setpluginindex(macidx)
    self.maincanvas:setoutputtext(msg)
    self.maincanvas:setcallstack(trace)
    for i=1, #trace do
      if self.maincanvas:openinfo(trace[i]) then
        break
      end
    end  
    if self.frame == nil then      
      self.maincanvas:setplugindir(plugininfo)
      self:openinframe()
    end  
  end    
end

function machine:togglecanvas()
  if self.frame == nil then   
    self:openinframe()
  else
    self:openinmainframe()
  end
end

function machine:openinframe() 
  self.maincanvas:setwindowiconin()
  self:setcanvas(nil)
  self:createframe()   
  self.frame:show(centertoscreen:new():sizetoscreen(0.9, 0.9))
end

function machine:openinmainframe() 
  self.frame = nil
  self.maincanvas:setwindowiconout()
  self:setcanvas(self.maincanvas)
end

function machine:onactivated()  
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
                          :setpos(0, 0, 400, 400)
                          :setview(that.menudesigner)        
        function that.frame:onshow()          
          that.menudesigner.tree:editnode(that.menudesigner.tree:selected())
        end
        function that.frame:onclose()           
          that.frame = nil
        end
        that.frame:show(centertoscreen:new())        
      end
   end              
   self.menubar:update()
end

return machine
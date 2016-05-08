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
local sysmetrics = require("psycle.ui.systemmetrics")
local menubar = require("psycle.ui.menubar")
local serpent = require("psycle.serpent")
local plugincatcher = require("psycle.plugincatcher")
local treenode = require("psycle.ui.canvas.treenode")

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
   self.maincanvas = maincanvas:new()   
   self.maincanvas.togglecanvas:connect(machine.togglecanvas, self)
   self.editmacidx_ = -1
   self:setcanvas(self.maincanvas)   
end

function machine:createframe()  
  self.frame = frame:new()
  self.frame:setview(self.maincanvas)  
  self.frame:settitle("Psycle Plugineditor")  
  local w, h = sysmetrics.screensize()
  local fw = w * 0.9
  local fh = h * 0.9
  self.frame:setpos((w-fw)/2, (h-fh)/2, fw, fh)
  local that = self
  function self.frame:onclose(ev) 		       
    that:exit()
  end
end

function machine:editmacidx()
  return self.editmacidx_;
end

function machine:onexecute(msg, macidx, plugininfo, trace)  
  if msg == nil then  
    self:openinmainframe()  
  else  
    self.editmacidx_ = macidx 
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
  self.frame:show()
end

function machine:openinmainframe() 
  self.frame = nil
  self.maincanvas:setwindowiconout()
  self:setcanvas(self.maincanvas)
end

function machine:onactivated()
  self:initmenu()  
end

function machine:ondeactivated()    
  self.root:remove(1)  
  self.node = nil
  self.testnode = nil;
  collectgarbage()
  self.menubar:invalidate()
end

function machine:initmenu()   
   self.testnode = treenode:new()  
   
   self.menubar = menubar:new()   
   self.root = treenode:new()   
   self.menubar:setrootnode(self.root) 
   
   local that = self
   function self.menubar:onclick(node)       
      if node == that.subnode then 
        psycle.alert("subnode clicked")
      elseif node == that.subnode1 then 
        psycle.alert("subnode 1 clicked")
      end
   end
   
   --self.menubar:mainmenu()        
   self.node = treenode:new()   
   self.subnode = treenode:new()        
   self.node:settext("Plugineditor")
   self.subnode:settext("sub_node");
   self.node:add(self.subnode);   
   self.subnode1 = treenode:new()           
   self.subnode1:settext("sub_node1");
   self.node:add(self.subnode1);
   
   psycle.output("#1."..serpent.dump(self.root._children))
   self.root:add(self.node);
   psycle.output("#2"..serpent.dump(self.root._children))
   self.menubar:update();
   --self.maincanvas.pluginexplorer:setrootnode(self.root)  
   --node = self.root:at(0)
   --psycle.output(node:text())
end

function machine:onmenu(menuitem)
  psycle.output("here")
end

return machine
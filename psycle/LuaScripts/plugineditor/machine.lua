-- psycle plugineditor (c) 2015 by psycledelics
-- File: machine.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()

machine = require("psycle.machine"):new()

local maincanvas = require("maincanvas")
local frame = require("psycle.ui.canvas.frame")
local sysmetrics = require("psycle.ui.systemmetrics")

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
   self.editmacidx_ = -1
   self:setcanvas(self.maincanvas)     
end

function machine:createframe()  
  self.frame = frame:new():setcanvas(self.maincanvas)  
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

function machine:onexecute(msg, macidx, trace)
  self.editmacidx_ = macidx 
  self.maincanvas:setoutputtext(msg)
  self.maincanvas:setcallstack(trace)
  for i=1, #trace do
    if self.maincanvas:openinfo(trace[i]) then
      break
    end
  end
  if self.frame == nil then
    self:createframe()
  end
  self.frame:show() 
end

return machine
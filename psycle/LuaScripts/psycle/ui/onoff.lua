-- psycle onoff module (c) 2013/2014 by psycledelics
-- file : psycle/onoff.lua
-- copyright 2014 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

local group = require("psycle.ui.canvas.group")
local rect = require("psycle.ui.canvas.rect")

local onoff = group:new()

function onoff:new(parent, par) 
  local c = group:new(parent)
  setmetatable(c, self)
  self.__index = self
  c:init(par)  
  return c
end
  
function onoff:init(par)    
  self.rect = rect:new(self):setpos(0, 0, 10, 5)    
  self.istweak = false  
  self.par = par  
  if (self.par ~= nil) then
    self.par:addlistener(self)
  end
  self:updateview()
  return self
end

function onoff:setparam(par)
  self.par = par  
  self.par:addlistener(self)
end

function onoff:param()
  return self.par
end

function onoff:onmousedown(x, y)  
  if (self.par~=nil) then        
     if self.par:val() == 0 then
	   self.par:setval(1)
	 else
	   self.par:setval(0)
	 end
  end
end

function onoff:ontweaked()
  self:updateview()  
end

function onoff:updateview()
   if (self.par~=nil) then     
     if (self.par:val() == 0) then
       self.rect:setcolor(0x323232)
	 else
	   self.rect:setcolor(0x323232)
	 end
   end
end

function onoff:tostring()
  return "onoff"
end

return onoff

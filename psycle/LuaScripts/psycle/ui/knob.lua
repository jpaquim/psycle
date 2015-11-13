-- psycle knob module (c) 2013/2014 by psycledelics
-- file : psycle/knob.lua
-- copyright 2014 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

local item = require("psycle.ui.canvas.item")
local image = require("psycle.ui.image")

local knob = item:new()

function knob:new(parent, par) 
  if (par~=nil) then
    psycle.output("par"..par:val())
  end
  local c = item:new(parent)
  setmetatable(c, self)
  self.__index = self
  --c.rect = rect:new(c):setpos(0, 0, 10, 5)    
  c:init(par)
  return c
end
  
function knob:init(par)    
  self.img = image:new()    
  self.istweak = false
  self.finetweak = false
  self.ultrafinetweak = false
  self.visualtweakvalue = 0.0
  self.sourcex = 0
  self.sourcepoint = 0  
  self.par = par  
  if (self.par ~= nil) then
    self.par:addlistener(self)
  end  
  self.screenh = 800  
  return self
end

function knob:setparam(par)
  self.par = par  
  self.par:addlistener(self)
end

function knob:param()
  return self.par
end

function knob:onmousedown(ev)  
  if self.par ~= nil then         
    self.istweak = true    
    self.sourcex = ev.clientx
	self.sourcepoint = ev.clienty
	local x, y, z = 0, 0, 0
    x, y, z = self.par:range()		
	self.steps = z	
    self.tweakbase = self.par:norm()*self.steps
	self.visualtweakvalue = self.tweakbase
	self:canvas():mousecapture()
	self:canvas():hidecursor()
  end
end

function knob:onmousemove(ev)  
  if (self.istweak == true and self.par ~=nil) then
    local freak = self:computefreak(ev.shiftkey, ev.ctrlkey)	
	local nv = (self.sourcepoint - ev.clienty)*freak + self.tweakbase
	nv = math.min(math.max(0, nv), self.steps)	
    self.visualtweakvalue = nv/self.steps
	self:fls()
    nv = math.floor(nv + 0.5)	
	self.par:setnorm(nv/self.steps)
    local space = 30	
    if ev.clienty < self.sourcepoint-space or
	   ev.clienty > self.sourcepoint+space then
	  self:canvas():setcursorpos(self.sourcex, self.sourcepoint)
	  self.tweakbase = self.visualtweakvalue*self.steps
	end	
  end
end

function knob:onmouseup()    
  self.istweak = false
  self:canvas():mouserelease()
  self:canvas():setcursorpos(self.sourcex, self.sourcepoint)
  self:canvas():showcursor()
end

function knob:ontweaked()
  self:fls()
end

function knob:draw(g)
  local xsrc = 28*math.floor(63*(self.par:norm()))
  g:drawimage(self.img, 0, 0, 30, 30, xsrc, 0)
end

function knob:computefreak(finetweak, ultrafinetweak)
   if ((self.ultrafinetweak and not shift) or    -- shift-key has been left.
	  (not self.ultrafinetweak and shift)) then  -- shift-key has just been pressed	
       self.tweakbase = self.par:norm()*self.steps
       self.ultrafinetweak = not self.ultrafinetweak
   elseif ((self.finetweak and not ctrl) or   -- control-key has been left.
		  (not self.finetweak and ctrl)) then -- control-key has just been pressed     
       self.tweakbase = self.par:norm()*self.steps
       self.finetweak = not self.finetweak
   end
   local freak = 0.5
   if self.ultrafinetweak then
	  freak = 0.5
   elseif (self.steps < self.screenh/4) then
	  freak = (self.steps)/(self.screenh/4)
   elseif (self.steps < self.screenh*2/3) then
	  freak = (self.steps)/(self.screenh/3)
   else
      freak = (self.steps)/(self.screenh*3/5)
   end
   if (self.finetweak) then freak = freak/5 end  	
   return freak
end

function knob:tostring()
  return "knob"
end

function knob:onupdateregion(region)
   region:setrect(0, 0, 30, 30)
end

return knob

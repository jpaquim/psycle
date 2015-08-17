-- psycle knob module (c) 2013/2014 by psycledelics
-- file : psycle/paramtext.lua
-- copyright 2014 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

local text = require("psycle.ui.canvas.text")
local serpent = require("psycle.serpent")

local paramtext = text:new()

paramtext.DISPNAME = 1
paramtext.DISPDESC = 2
paramtext.DISPVAL = 3

function paramtext:new(parent, par)   
  local c = text:new(parent)
  setmetatable(c, self)
  self.__index = self
  c:init(par)
  return c
end
  
function paramtext:init(par)    
  self.par = par  
  self.mode = paramtext.DISPNAME
  if (self.par ~= nil) then
    self.par:addlistener(self)
	self:updateview()
  end  
end

function paramtext:setparam(par)
  self.par = par  
  self.par:addlistener(self)
  self:updateview()
  return self
end

function paramtext:setmode(mode)
  self.mode = mode
  self:updateview()  
  return self
end

function paramtext:param()
  return self.par
end

function paramtext:ontweaked(par)  
  self:updateview(par)
end

function paramtext:updateview()
  if (self.par ~= nil) then
    if (self.mode==paramtext.DISPDESC) then
      self:settext(self.par:display().."")  
    elseif (self.mode==paramtext.DISPNAME) then
      self:settext(self.par:name())  
    elseif (self.mode==paramtext.DISPVAL) then	  
      self:settext(self.par:val())  
    end
  end
end

function paramtext:tostring()
  return "paramtext"
end

return paramtext

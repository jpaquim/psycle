--[[ 
psycle pianoroll (c) 2017 by psycledelics
File:  screenrange.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local screenrange = {}

function screenrange:new()
  local m = {}                
  setmetatable(m, self)
  self.__index = self    
  m:init()  
  return m
end

function screenrange:init()
  self.left, self.right = 0, 0 
end

function screenrange:has(event)
  return event:position() + event.length >= self.left and event:position() < self.right
end

function screenrange:haspos(pos)
  return pos >= self.left and pos < self.right
end

function screenrange:onscreen()
  return self.right >= 0 and self.left ~= self.right
end

return screenrange

--[[ 
psycle pianoroll (c) 2017 by psycledelics
File:  keymap.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local keymap = {}

function keymap:new(from, to)
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  m:init(from, to)
  return m
end

function keymap:init(from, to)
  if from and to then
    self.range = {}
    self.range.from = from
    self.range.to = to
  else
    self.range = { from = 12, to = 119 }
  end
  self.numkeys_ = self.range.to - self.range.from
end
  
function keymap:has(key)
  return key >= self.range.from and key <= self.range.to
end

function keymap:numkeys()
  return self.numkeys_ + 1
end

return keymap
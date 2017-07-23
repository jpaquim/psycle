--[[ psycle pianoroll (c) 2017 by psycledelics
File: clearrowcommand.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.  
]]

local player = require("psycle.player"):new()

local clearrowcommand = {}

function clearrowcommand:new(...)
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  m:init(...)
  return m
end

function clearrowcommand:init(sequence, cursor)
  self.sequence_ = sequence
  self.cursor_ = cursor
end

function clearrowcommand:execute()  
  local events = self.sequence_:selection()
  if #events > 0 then
    self.sequence_:eraseselection()
    self.cursor_:notify()
  else
    local pattern = self.sequence_:at(self.cursor_:seqpos()) 
    local event = self:search(pattern)
    if event and event:position() == self.cursor_:position() then
      pattern:erase(event)
    end
    self.cursor_:setposition(math.max(0, self.cursor_:position() +  player:bpt()))
  end
end

function clearrowcommand:search(pattern)  
  local e = pattern:firstnote()
  if e and e:track() ~= self.cursor_:track() then
    e = pattern:nextevent(self.cursor_:track(), e)
  end
  while e and e:position() < self.cursor_:position() do   
    e = pattern:nextevent(self.cursor_:track(), e)
  end 
  if e then
    e = e:position() == self.cursor_:position() and e or nil
  end
  return e
end

return clearrowcommand

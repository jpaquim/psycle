--[[ psycle pianoroll (c) 2017 by psycledelics
File: cursor.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.  
]]

local listener = require("psycle.listener")
local player = require("psycle.player"):new()
local rawpattern = require("psycle.pattern"):new()

local cursor = {}

cursor.NOTES = 1
cursor.DRUMS = 2

cursor.MAXCOL = 9

function cursor:new(...)  
  local m = {} 
  setmetatable(m, self)
  self.__index = self
  m:init(...)
  return m
end

function cursor:init(sequence)
  self.sequence_ = sequence
  self.listeners_ = listener:new()
  self:reset()
end

function cursor:settrackview(trackview)
  self.trackview_ = trackview
  return self
end

function cursor:setgridgroup(gridgroup)
  self.gridgroup_ = gridgroup
  return self
end

function cursor:reset()
  self.position_, self.seqpos_ = 0, 1
  self.column_, self.track_ = 1, 0
  self.trackmodes_ = {}
  for i=1, 65 do
    self.trackmodes_[i] = cursor.NOTES
  end
  self.scrolloffset_ = 0
  rawpattern.settrackedit(self.track_)
  return self
end

function cursor:settrack(track)
  if track ~= self.track_ then
    self.track_ = math.max(0, math.min(track, rawpattern:numtracks() - 1))
    rawpattern.settrackedit(self.track_)
    self:notify()
  end
  return self
end

function cursor:track()
  return self.track_
end

function cursor:inctrack()
  self.column_ = 1    
  self.track_ = self.track_ + 1
  if self.track_ >= rawpattern:numtracks() then
    self.track_ = 0
    self.scrolloffset_ = 0
  elseif self.trackview_ and self.track_ - self.scrolloffset_ >= self.trackview_:visibletracks() then
    self:scrollright(self.trackview_:visibletracks())
  end
  rawpattern.settrackedit(self.track_)
  self:notify()
  return self
end

function cursor:dectrack(column)
  self.column_ = column and column or 1  
  self.track_ = self.track_ - 1
  if self.track_ < 0 then
    if self.trackview_ then
      self.scrolloffset_ = math.max(0, rawpattern:numtracks() - self.trackview_:visibletracks())
    end
    self.track_ = rawpattern:numtracks() - 1
  elseif self.track_ - self.scrolloffset_ < 0  then
    self:scrollleft()
  end
  rawpattern.settrackedit(self.track_)
  self:notify()
  return self
end

function cursor:setcolumn(column)
  self.column_ = column
end

function cursor:inc()        
  self.column_ = self.column_ + 1
  if self.column_ > self.MAXCOL then
    self:inctrack()
  else
    self:notify()
  end
  return self
end

function cursor:dec()
  self.column_ = self.column_ - 1
  if self.column_ < 1 then
    self:dectrack(cursor.MAXCOL)
  else
    self:notify()
  end
  return self
end

function cursor:column()
  return self.column_
end

function cursor:setposition(pos)  
  self.position_ = math.floor(pos / player:bpt()) * player:bpt()
  rawpattern.settrackline(math.floor(self.position_ * player:tpb()))
  self:notify()
  return self
end

function cursor:position()
  return self.position_
end

function cursor:setseqpos(seqpos)
  self.seqpos_ = seqpos
  return self
end

function cursor:seqpos()
  return self.seqpos_
end

function cursor:incrow()
  local newpos = self.position_ + player:bpt()
  local scroll = true
  if newpos < self.sequence_:at(self.seqpos_):numbeats() then
    self.position_ = newpos      
  else
    self.position_ = 0
    if self.gridgroup_.gridpositions:displaymode() == self.gridgroup_.gridpositions.DISPLAYALL and
      self.seqpos_  < self.sequence_:len() then
      self.seqpos_ = self.seqpos_ + 1
    else
      if self.gridgroup_.gridpositions:displaymode() == self.gridgroup_.gridpositions.DISPLAYALL then
        self.seqpos_ = 1
      end
      self.gridgroup_.patternview.scroller:setdx(0)
      scroll = false
    end    
  end
  if self.gridgroup_ and scroll then
    local range = self.gridgroup_.patternview:screenrange(self:seqpos())
    if self:position() >= range.right then         
      local visi = range.right - math.max(0, range.left)      
      self.gridgroup_.patternview.scroller:setdx(
          -(self.gridgroup_.patternview:gridpositions():pos(self:seqpos()) + 
          self.gridgroup_.patternview:zoom():beatwidth(
              range.left + (self:position() + player:bpt() - range.right))))
    end 
  end
  rawpattern.settrackline(math.floor(self.position_ * player:tpb())) 
  self:notify()
  return self  
end

function cursor:decrow()
  self.position_ = self.position_ - player:bpt()
  scroll = true
  if self.position_ < 0 then
    if self.gridgroup_.gridpositions:displaymode() == self.gridgroup_.gridpositions.DISPLAYALL and self.seqpos_  > 1 then
      self.seqpos_ = self.seqpos_ - 1
      self.position_ = self.sequence_:at(self.seqpos_):numbeats() - player:bpt()  
    else     
      if self.gridgroup_.gridpositions:displaymode() == self.gridgroup_.gridpositions.DISPLAYALL then
        self.seqpos_ = self.sequence_:len()
      end
      self.position_ = self.sequence_:at(self.seqpos_):numbeats() - player:bpt()      
      self.gridgroup_.patternview.scroller:setdx(math.min(0,
          -self.gridgroup_.patternview:gridpositions():gridwidth()
          + self.gridgroup_.patternview:position():width()
          ))
      scroll = false
    end       
  end  
  if self.gridgroup_ and scroll then  
    local range = self.gridgroup_.patternview:screenrange(self:seqpos())
    if self:position() < math.max(0, range.left) then         
       local newpos =  math.max(0, math.max(0, range.left) - player:bpt())
       self.gridgroup_.patternview.scroller:setdx(
          -(self.gridgroup_.patternview:gridpositions():pos(self:seqpos()) +
          self.gridgroup_.patternview:zoom():beatwidth(newpos)))
    end
  end
  rawpattern.settrackline(math.floor(self.position_ * player:tpb())) 
  self:notify()
end

function cursor:steprow()
  self.position_ = self.position_ + rawpattern:patstep()*player:bpt()
  rawpattern.settrackline(math.floor(self.position_ * player:tpb()))
  self:notify()
  return self
end

function cursor:scrollleft()
  if self.scrolloffset_ > 0 then
    self.scrolloffset_ = self.scrolloffset_ - 1
    self:notify()
  end
  return self
end
  
function cursor:scrollright(visibletracks)
  if self.scrolloffset_ < rawpattern:numtracks() - visibletracks then
    self.scrolloffset_ = self.scrolloffset_ + 1
    self:notify()
  end
  return self
end

function cursor:addlistener(listener)
  self.listeners_:addlistener(listener)
  return self
end

function cursor:notify()
  self.listeners_:notify(self, "ontrackchanged")
  return self
end

function cursor:preventnotify()
  self.listeners_:prevent()
  return self
end

function cursor:enablenotify()
  self.listeners_:enable()
  return self
end

return cursor

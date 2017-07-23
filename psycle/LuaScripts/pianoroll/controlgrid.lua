--[[ 
psycle pianoroll (c) 2017 by psycledelics
File:  controlgrid.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local player = require("psycle.player")
local hitarea = require("hitarea")
local rastergrid = require("rastergrid")

local controlgrid = rastergrid:new()

function controlgrid:new(...)
  local m = rastergrid:new(...)
  setmetatable(m, self)
  self.__index = self    
  m:init(...)
  return m
end

function controlgrid:init()
  self.preferredsectionheight_ = 50
end

function controlgrid:draw(g, screenrange, pattern, seqpos, section)
  g:setcolor(0xFF999999)
  g:drawstring(section:display(), point:new(self.view:position():width() - self.view.scroller:dx() - 50, 2))
  self:drawcursorbar(g, seqpos)
  self:drawevents(g, screenrange, pattern, seqpos, section.eventfilter)  
  self:drawdragging(g)
end

function controlgrid:drawevents(g, screenrange, pattern, seqpos, filter)
  self.visiblenotes = self.view:visiblenotes()
  local event = pattern:firstcmd()
  while event do    
    if self.view.sequence.trackviewmode == self.view.sequence.VIEWALLTRACKS and self.view.keyboard.keymap:has(event:note()) 
        and screenrange:has(event) and filter:accept(event) then
      self:drawevent(g, event, self.view.cursor.modes[event:track()], seqpos)   
    end
    event = event.next
  end
  local event = self.view.gridpositions_:firstnote(screenrange, pattern)
  while event do  
    if self.view.sequence.trackviewmode == self.view.sequence.VIEWALLTRACKS and
        (event:cmd() ~= 0 or event:parameter() ~= 0) and screenrange:has(event) and filter:accept(event) then            
      self:drawevent(g, event, self.view.cursor.modes[event:track()], seqpos)
    end
    event = event.next
  end
end

function controlgrid:drawevent(g, event, trackmode, seqpos)
  local iseventplayed = self.view.playtimer:currsequence() == seqpos - 1 and self.view.playtimer:isnoteplayed(event)
  self:seteventcolor(g, event, self.view.cursor:track(), iseventplayed)
  local x = event:position() * self.view:zoom():width()
  g:drawline(point:new(x, self:preferredheight()), point:new(x, (self:preferredheight() - 4) * (1 - event:norm()) + 4))
  g:drawrect(rect:new(point:new(x - 2, (self:preferredheight() - 4) * (1 - event:norm())),dimension:new(4, 4)))                                  
end

function controlgrid:drawdragging(g)                                     
end

function controlgrid:hittest(position, gridpos_, pattern_, seqpos_)
  local grid = math.min(math.floor(position:y() / self:preferredheight()) + 1, #self.view.sections)
  local yoffset = grid*self:preferredheight()
  local result = { 
    hitarea = hitarea.NONE, pattern = pattern_, seqpos = seqpos_, 
    mousepos = position, gridpos = gridpos_,
    note = self.view.keyboard.insertnote,
    beat = self.view:zoom():beat(position:x() - gridpos_)        
  }    
  local beat = self.view:zoom():beat(position:x() - gridpos_)
  local event = nil
  if pattern_ then
    if self.view.sections[grid] then
      event = pattern_:firstcmd()
      while event do       
         if beat >= event:position() and beat < event:position() + self.player:bpt() 
             and self.view.sections[grid].eventfilter:accept(event) then
           result.event = event
           result.startpos = event:position() * self.view:zoom():width()      
           result.index = index
           result.hitarea = hitarea.MIDDLE
         end
         event = event.next   
      end
      if result.hitarea == hitarea.NONE then
        event = pattern_:firstnote()
        while event do       
           if beat >= event:position() and beat < event:position() + self.player:bpt() 
               and self.view.sections[grid].eventfilter:accept(event)
               and (event:cmd() ~= 0 or event:parameter() ~= 0) then
             result.event = event
             result.startpos = event:position() * self.view:zoom():width()      
             result.index = index
             result.hitarea = hitarea.MIDDLE
           end
           event = event.next   
        end
      end
    end    
  end    
  return result
end

function controlgrid:hittestselrect()
  local result = {}
  if self.view.selectionrect then
    local rect = self.view.selectionrect
    local num = self.view.sequence:len()
    for i=1, num do         
      local gridpos = self.view:gridpositions():pos(i)      
      local beatleft = (rect:left() - gridpos)  / self.view:zoom():width()
      local beatright = (rect:right() - gridpos)  / self.view:zoom():width() 
      local event = self.view.sequence:at(i):firstcmd()
      while event do        
        if event:position() + event.length >= beatleft and event:position() <= beatright
        and self.view.sections[self.view.selectionsection].eventfilter:accept(event)
        then
          if not result[i] then
            result[i] = {}
          end
          result[i][#result[i]+1] = event              
        end
        event = event.next        
      end
      local event = self.view.sequence:at(i):firstnote()
      while event do        
        if event:position() + event.length >= beatleft and event:position() <= beatright
        and self.view.sections[self.view.selectionsection].eventfilter:accept(event)
        and (event:cmd() ~= 0 or event:parameter() ~= 0)
        then
          if not result[i] then
            result[i] = {}
          end
          result[i][#result[i]+1] = event              
        end
        event = event.next        
      end
    end
  end  
  return result
end

function controlgrid:bardimension()
  return dimension:new(math.floor(self.beatsperbar * self.view:zoom():width()),
      self:preferredheight())
end

function controlgrid:preferredheight()
  return self.preferredsectionheight_
end

return controlgrid

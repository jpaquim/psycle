-- psycle splitter (c) 2015 by psycledelics
-- file : splitter.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

local canvas = require("psycle.ui.canvas")
local rect = require("psycle.ui.canvas.rect")
local style = require("psycle.ui.canvas.itemstyle")
local serpent = require("psycle.serpent")

local splitter = rect:new()

splitter.HORZ = 1
splitter.VERT = 2

function splitter:new(parent, orientation)
  local c = rect:new(parent)
  setmetatable(c, self)
  self.__index = self  
  c:init(orientation)
  return c
end
  
function splitter:init(orientation)
  self:setcolor(0x404040)
  self.orientation = orientation
  if orientation == splitter.HORZ then  
    self:style():setalign(style.ALBOTTOM + style.ALLEFT + style.ALRIGHT)
    self:setheight(5)
  elseif orientation == splitter.VERT then
    self:style():setalign(style.ALLEFT + style.ALTOP + style.ALBOTTOM)                    
    self:setwidth(5)
  end
end

function splitter:onmousedown(e)
  self:canvas():mousecapture()
  self.dosplit_ = true
  self.dragpos = -1
  self.par = self:parent() 
  local idx = self.par:itemindex(self)
  local items = self.par:items() 
  self.item = items[idx-1]
  local x, y = self.item:clientpos()
  self.itemclientpos = 0
  if self.orientation == self.HORZ then    
    self.itemclientpos = y + self.item:height()
  else
    self.itemclientpos = x
  end
end  

function splitter:onmousemove(e)         
  if self.dosplit_ then
    if self.orientation == splitter.HORZ then
      if (self.dragpos ~= e.clienty) then      
        self.dragpos = e.clienty
        local cw, ch = self.par:clientsize()       
        self.item:setheight(self.itemclientpos - self.dragpos)          
        self.par:setsize(cw, ch)      
      end      
    elseif self.orientation == splitter.VERT then
      if (self.dragpos ~= e.clientx) then      
        self.dragpos = e.clientx
        local cw, ch = self.par:clientsize()       
        self.item:setwidth(self.dragpos - self.itemclientpos)          
        self.par:setsize(cw, ch)      
      end 
    end
  else
    if self.orientation == splitter.HORZ then
      self:canvas():setcursor(canvas.CURSOR.ROW_RESIZE) 
    elseif self.orientation == splitter.VERT then
      self:canvas():setcursor(canvas.CURSOR.COL_RESIZE)
    end
  end
end

function splitter:onmouseout()        
  if not self.dosplit_ then
    self:canvas():setcursor(canvas.CURSOR.DEFAULT)
  end
end

function splitter:onmouseup(e)
  self.dosplit_ = false
  self:canvas():mouserelease()
end 

return splitter
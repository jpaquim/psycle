-- psycle callstack (c) 2015 by psycledelics
-- File: callstack.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local style = require("psycle.ui.canvas.itemstyle")

local callstack = group:new()

local rowstyle = style:new():setalign(style.ALLEFT + style.ALTOP + style.ALBOTTOM)
                            :setmargin(0, 0, 2, 0)
local colstyle = style:new():setalign(style.ALTOP + style.ALLEFT + style.ALRIGHT)
                            :setmargin(0, 0, 0, 2)  

function callstack:new(parent, listener)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  c.listener_ = listener
  return c
end

function callstack:init() 
  self.colors = { 
    header = 0xAAAAAA,
    headerfont = 0x000000,
    row1font = 0xFF0000,
    row = 0xFFFFFF,
    font = 0x000000
  }                           
  self.row1 = group:new(self):setstyle(rowstyle)  
  self.row2 = group:new(self):setstyle(rowstyle)
  self.row3 = group:new(self):setstyle(rowstyle)   
  rect:new(self.row1):setcolor(self.colors.header):setwidth(20):setstyle(colstyle)  
  rect:new(self.row2):setcolor(self.colors.header):setwidth(400):setstyle(colstyle)
  --text:new(self.row2):settext("Name"):setcolor(self.colors.headerfont)
  rect:new(self.row3):setcolor(self.colors.header):setwidth(200):setstyle(colstyle)
  --text:new(self.row3):settext("Source"):setcolor(self.colors.headerfont)
end

function onrowclick(self)
  self.that.listener_:oncallstackclick(self.info) 
  local index = self:parent():itemindex(self)
  self.that:setdepth(index-2)  
end

function callstack:add(info)
  -- row1  
  self.r1 = group:new(self.row1):setstyle(colstyle)  
  rect:new(self.r1):setcolor(self.colors.row):setstyle(colstyle):setheight(12)
  self.r1.text = text:new(self.r1):settext("*"):setcolor(self.colors.row1font):setpos(2, 0)
  -- row2       
  local r2 = group:new(self.row2):setstyle(colstyle)  
  rect:new(r2):setcolor(self.colors.row):setstyle(colstyle):setheight(12)
  text:new(r2):settext(info.name.." Line "..info.line)
              :setcolor(self.colors.font)
              :setpos(2, 0)
  -- row3    
  local r3 = group:new(self.row3):setstyle(colstyle)
  rect:new(r3):setcolor(self.colors.row):setstyle(colstyle):setheight(12)  
  text:new(r3):settext(info.source:match("([^\\]+)$"))
              :setcolor(self.colors.font):setpos(2, 0)  
  self.r1.info, r2.info, r3.info = info, info, info
  self.r1.that, r2.that, r3.that = self, self, self
  --self.r1.onmousedown = onrowclick
  r2.onmousedown = onrowclick
  r3.onmousedown = onrowclick
end

function callstack:setdepth(depth)
  local items = self.row1:items()
  for i=2, #items do
    local item = items[i] 
    if depth == i-1 then
      item.text:settext("*")
    else
      item.text:settext("")
    end
  end  
end

return callstack
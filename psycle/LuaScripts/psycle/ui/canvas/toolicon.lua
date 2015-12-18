-- psycle toolicon (c) 2015 by psycledelics
-- File: toolicon.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local image = require("psycle.ui.image")
local item = require("psycle.ui.canvas.item")
local listener = require("listener")
local cfg = require("psycle.config"):new("PatternVisual")

local toolicon = item:new()

local settings = { 
  colors = {
    default = {
      bg = cfg:get("pvc_row4beat"),
      text = cfg:get("pvc_font")
    },
    mousepress = {
      bg = cfg:get("pvc_row"),
      text = cfg:get("pvc_font")
    },
    mousemove = {
      bg  = cfg:get("pvc_row"),
      text = cfg:get("pvc_font")
    } 
  }
}

function toolicon:new(parent, filename, trans)
  local c = item:new()
  setmetatable(c, self)
  self.__index = self  
  c:init(filename, trans)
  if parent ~= nil then
    parent:add(c)
  end
  return c
end

function toolicon:onclick() end

function toolicon:init(filename, trans)
  self.w, self.h = 20, 20  
  self.cc = settings.colors.default.bg
  self.on = false  
  self.text_ = ""
  if filename then
    self.img = image:new():load(filename)
    if trans~=nil then
      self.img:settransparent(trans)
    end  
    local w, h = self.img:size()  
    self.centerx, self.centery = (self.w-w)/2, (self.h-h)/2  
  end  
  self.clicklistener_ = listener:new("onclick", true)
  self.istoggle_ = false
end

function toolicon:draw(g)
  g:setcolor(self.cc)   
  g:fillroundrect(0, 0, self.w, self.h, 5, 5)  
  local xpos = 0
  if self.img then
    g:drawimage(self.img, self.centerx, self.centery) 
    local ix, iy = self.img:size()    
    xpos = xpos + self.centerx + ix
  end
  g:setcolor(self.cc)
  g:setcolor(settings.colors.default.text)
  g:drawstring(self.text_, xpos, 0)
end

function toolicon:onmousedown(ev)  
  self:seton(not self.on)  
  self:onclick()
  self.clicklistener_:notify(self)
end

function toolicon:onmousemove(ev)  
  if (not self.on) then 
    self.cc = settings.colors.mousemove.bg
    self:fls()
  end
end

function toolicon:onmouseout(ev)  
  if (not self.on) then 
    self.cc = settings.colors.default.bg
    self:fls()
  end
end

function toolicon:onmouseup(ev)  
  if not self.is_toggle then
    self:seton(not self.on)  
  end
end

function toolicon:seton(on)  
  if self.on ~= on then
    self.on = on
    if on then 
      self.cc = settings.colors.mousepress.bg
    else
      self.cc = settings.colors.default.bg
    end
    self:fls()
    if self.toolbar~=nil and on and self.istoggle_ then
      self.toolbar:onnotify(self)      
    end  
  end
  return self
end

function toolicon:settoolbar(toolbar) self.toolbar = toolbar end

function toolicon:onsize(cw, ch)
  self.w = cw
  self.h = ch
end

function toolicon:onupdateregion(rgn)
   rgn:setrect(0, 0, self.w, self.h)   
end

function toolicon:addlistener(listener)    
  self.clicklistener_:addlistener(listener)  
  return self
end

function toolicon:settext(text)
  self.text_ = text
  return self
end

function toolicon:text()
  return self.text_
end

return toolicon
-- psycle toolicon (c) 2015 by psycledelics
-- File: toolicon.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local image = require("psycle.ui.image")
local item = require("psycle.ui.canvas.item")
local listener = require("psycle.listener")
local serpent = require("psycle.serpent")

local toolicon = item:new()

local settings = { 
  colors = {
    default = {
      bg = 0x3E3E3E,
      text = 0xCACACA
    },
    mousepress = {
      bg = 0x363636,
      text = 0xCACACA
    },
    mousemove = {
      bg  = 0x363636,
      text = 0xCACACA
    } 
  }
}

function toolicon:new(parent, filename, trans)
  local c = item:new()
  setmetatable(c, self)
  self.__index = self
  if parent ~= nil then  
   parent:add(c)   
  end
  c:init(filename, trans)  
  return c
end

function toolicon:onclick() end

function toolicon:init(filename, trans)
  self:setautosize(true, true)  
  self.cc = settings.colors.default.bg
  self.on = false  
  self.text_ = ""
  if filename then
    self.img = image:new():load(filename)
    if trans~=nil then
      self.img:settransparent(trans)
    end    
  end  
  self.clicklistener_ = listener:new("onclick", true)
  self.istoggle_ = false
  self:setpos(0, 0, 20, 20)
end

function toolicon:setpos(x, y, w, h)
  if self.img ~= nil then
    local iw, ih = self.img:size()  
    self.centerx, self.centery = (w - iw)/2, (h - ih)/2    
  end  
  if w ~= nil then    
    item.setpos(self, x, y, w, h)
  else    
    item.setpos(self, x, y)
  end  
  return self
end

function toolicon:draw(g)  
  local x, y, w, h = self:pos()
  g:setcolor(self.cc)   
  g:fillroundrect(0, 0, w-1, h-1, 5, 5)  
  local xpos = 0
  if self.img then
    g:drawimage(self.img, self.centerx, self.centery) 
    local ix, iy = self.img:size()    
    xpos = xpos + self.centerx + ix
  end
  g:setcolor(self.cc)
  g:setcolor(settings.colors.default.text)  
  local textwidth, textheight = g:textsize(self.text_)
  g:drawstring(self.text_, xpos + 2, (h-textheight)/2)
end

function toolicon:onmousedown(ev)  
  self:seton(not self.on)  
  self:onclick()
  self.clicklistener_:notify(self)
end

function toolicon:onmouseenter(ev)  
  if (not self.on) then 
    self.cc = settings.colors.mousemove.bg
    self:fls()
  end
end

function toolicon:onmousemove(ev)  
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

function toolicon:settoolbar(toolbar)
  self.toolbar = toolbar
  return self
end

function toolicon:addlistener(listener)    
  self.clicklistener_:addlistener(listener)  
  return self
end

function toolicon:settext(text)  
  self.text_ = text
  self:fls()
  return self
end

function toolicon:text()
  return self.text_
end

function toolicon:oncalcautodimension()
  return 20, 20
end

function toolicon:onupdatearea(area, width, height)  
  local auto_w, auto_h = self:autosize()
  if auto_w then
    width = 20
  end
  if auto_h then
    height = 20
  end
  area:setrect(0, 0, width, height)  
  return true
end

return toolicon
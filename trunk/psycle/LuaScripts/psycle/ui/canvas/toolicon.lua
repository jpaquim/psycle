-- psycle toolicon (c) 2015 by psycledelics
-- File: toolicon.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local image = require("psycle.ui.image")
local item = require("psycle.ui.canvas.item")
local listener = require("psycle.listener")
local serpent = require("psycle.serpent")

local toolicon = item:new()

toolicon.settings = { 
  colors = {
    default = {
      bg = 0x292929,
      text = 0xCACACA
    },
    mousepress = {
      bg = 0x568857,
      text = 0xCACACA
    },
    mousemove = {
      bg  = 0x414131,
      text = 0xCACACA
    },
    checked = {
	  bg = 0xf1f1f1,
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
  self.cc = toolicon.settings.colors.default.bg
  self.on_ = false  
  self.text_ = ""
  if filename then
    self.img = image:new():load(filename)
    if trans~=nil then
      self.img:settransparent(trans)
    end    
  end  
  self.clicklistener_ = listener:new("onclick", true)
  self.istoggle_ = false
  self:setposition(rect:new(point:new(0, 0), dimension:new(20, 20)))  
end

function toolicon:settoggleimage(image)
  self.toggleimage_ = image
  self.oldimage = nil  
end

function toolicon:setposition(rect)
  local w = rect:width()
  local h = rect:height()
  if self.img ~= nil then
    local iw, ih = self.img:size()  
    self.centerx, self.centery = (w - iw)/2, (h - ih)/2    
  end  
  item.setposition(self, rect)  
  return self
end

function toolicon:draw(g)    
  g:setcolor(self.cc)   
  local dim = self:dimension()
  g:fillrect(rect:new(point:new(), dim))
  local xpos = 0
  if self.img then
    g:drawimage(self.img, self.centerx, self.centery) 
    local ix, iy = self.img:size()    
    xpos = xpos + self.centerx + ix
  end
  g:setcolor(self.cc)
  g:setcolor(toolicon.settings.colors.default.text)  
  local textwidth, textheight = g:textsize(self.text_)
  g:drawstring(self.text_, xpos + 2, (dim:height() - textheight)/2)
end

function toolicon:onmousedown(ev) 
  self:mousecapture()
  self:seton(not self.on_)  
  self:onclick()
  self.clicklistener_:notify(self)
end

function toolicon:onmouseenter(ev)  
  if (not self.on_) then     
    self.cc = toolicon.settings.colors.mousemove.bg
    self:fls()
  end
end

function toolicon:onmousemove(ev)  
end

function toolicon:onmouseout(ev)   
  if (not self.on_) then 
    self.cc = toolicon.settings.colors.default.bg	
    self:fls()    
  end
end

function toolicon:onmouseup(ev)  
  self:mouserelease()
  if not self.is_toggle then
    self:seton(false)  
  end
end

function toolicon:toggle()
  self:seton(not self.on_)
end

function toolicon:seton(on)  
  if self.on_ ~= on then
    self.on_ = on
    if on then 	  
      self.cc = toolicon.settings.colors.mousepress.bg
	  if (self.toggleimage_) then
	    self.oldimage = self.img
	    self.img = self.toggleimage_		
	  end
    else
	  if self.oldimage then	    
	    self.img = self.oldimage
	    self.oldimage = nil
	  end
      self.cc = toolicon.settings.colors.default.bg
    end
    self:fls()
    if self.toolbar~=nil and on and self.istoggle_ then
      self.toolbar:onnotify(self)      
    end  
  end
  return self
end

function toolicon:on()
  return self.on_;
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

function toolicon:transparent()  
  return false
end

return toolicon
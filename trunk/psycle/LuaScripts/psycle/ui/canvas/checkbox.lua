-- psycle checkbox (c) 2015 by psycledelics
-- File: checkbox.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local text = require("psycle.ui.canvas.text")
local rect = require("psycle.ui.canvas.rect")
local style = require("psycle.ui.canvas.itemstyle")
local cfg = require("psycle.config"):new("PatternVisual")

local checkbox = group:new()

local settings = { 
  colors = {
    default = {
      bg = cfg:get("pvc_row4beat"),
      text = cfg:get("pvc_font"),
      checker = cfg:get("pvc_font")
    },
    mousepress = {
      bg = cfg:get("pvc_row")
    },
    mousemove = {
      bg  = cfg:get("pvc_row") 
    }     
  }
}

function checkbox:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function checkbox:init(param)  
  self.width = 100
  self.height = 20  
  self.check_ = false
  
  self.checkgroup = group:new(self)  
  self.checkgroup:style():setalign(style.ALLEFT)
                         :setmargin(0, 0, 4, 0)
  self.checkrect = rect:new(self.checkgroup)                       
                       :setcolor(settings.colors.default.bg)
  self.checkrect:style():setalign(style.ALLEFT)         
  self.checktext = text:new(self.checkgroup)
                       :setpos(3, 0)
                       :setcolor(settings.colors.default.checker)
                                              
  --self.checktext:style():setalign(style.ALLEFT)
  
  self.text = text:new(self):settext("A Checkbox"):setcolor(settings.colors.default.text)
  self.text:style():setalign(style.ALLEFT)
  
  
  local that = self
  function self.checkgroup:onmousedown()
    psycle.output("ondown")
    that.check_ = not that.check_
    if that.check_ then
      that.checkrect:setcolor(settings.colors.mousepress.bg)
      that.checktext:settext("x")
    else
      that.checkrect:setcolor(settings.colors.default.bg)
      that.checktext:settext("")
    end
    that.onclick(self.check)
  end
  
end

function checkbox:settext(text)
  self.text:settext(text)
  return self
end

function checkbox:text() 
  return self.text:text()
end

function checkbox:check() 
  return self.check_;
end


--[[function checkbox:onsize(w, h)
  self.width = w
  self.height = h
  self:updateregion()
end

function checkbox:onupdateregion(rgn)
  rgn:setrect(0, 0, self.width, self.height)
end

]]
function checkbox:onclick(ischecked) end


return checkbox
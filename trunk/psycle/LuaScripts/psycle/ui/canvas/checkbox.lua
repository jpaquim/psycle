-- psycle checkbox (c) 2015 by psycledelics
-- File: checkbox.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local group = require("psycle.ui.canvas.group")
local item = require("psycle.ui.canvas.item")
local text = require("psycle.ui.canvas.text")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()

local checkbox = group:new()

local settings = { 
  colors = {
    default = {
      bg = 0x3E3E3E,
      text = 0xB0C8B1,
      checker = 0xCACACA
    },
    mousepress = {
      bg = 0x3E3E3E
    },
    mousemove = {
      bg  = 0x3E3E3E
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

function checkbox:init()  
  self:setautosize(true, true) 
  self.check_ = false  
  self.checkgroup = text:new(self)  
                         :setautosize(false, false)
                         :setposition(rect:new(point:new(0, 0), dimension:new(10, 10)))
                         :addornament(ornamentfactory:createfill(settings.colors.default.bg))
                         :setalign(item.ALLEFT)
                         :setmargin(0, 4, 0, 0)  
  self.text = text:new(self):settext("A Checkbox"):setalign(item.ALLEFT):setautosize(true, true)    
  local that = self
  function self.checkgroup:onmousedown()        
    that.check_ = not that.check_
    if that.check_ then      
      that.checkgroup:settext("x")
    else      
      that.checkgroup:settext("")
    end
    that:onclick(that.check_)
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

checkbox.published = {
  settext = checkbox.settext
}

function checkbox:onclick(ischecked) end

return checkbox
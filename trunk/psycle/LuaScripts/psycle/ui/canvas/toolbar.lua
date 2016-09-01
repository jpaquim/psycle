-- psycle toolbar (c) 2015 by psycledelics
-- File: toolbar.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local group = require("psycle.ui.canvas.group")
local item = require("psycle.ui.canvas.item")

local setmetatable = setmetatable
_ENV = nil

local toolbar = group:new()

function toolbar:new(parent)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self 
  c:init()
  return c
end

function toolbar:init()
  self:setautosize(true, true)
  self.icontable = {}
  --self:style()--:setmargin(4, 4, 4, 4)
end

function toolbar:add(icon)
  icon:settoolbar(self)
  icon:setalign(item.ALLEFT)
  icon:setmargin(0, 5, 0, 0)
  group.add(self, icon)	
end

function toolbar:seticons(icontable)
  for i=1, #icontable do    
    self:add(icontable[i])
  end
  return self
end

function toolbar:onnotify(sender)  
  local items = self:items()
  for i=1, #items do
    local icon = items[i]
	  if icon.istoggle_ and icon ~= sender then
	    icon:seton(false)
	  end
  end
end

return toolbar
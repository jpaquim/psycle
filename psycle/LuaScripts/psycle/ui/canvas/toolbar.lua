-- psycle toolbar (c) 2015 by psycledelics
-- File: toolbar.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local group = require("psycle.ui.canvas.group")

local toolbar = group:new()

function toolbar:new(parent)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  return c
end

function toolbar:init(x)

end

function toolbar:init() 
  self.iconw = 40 
  self.ident = 5
  self.xp = 0
  self.icontable = {}
end

function toolbar:add(icon)
  group.add(self, icon:setpos(self.xp, 0))	
  self.xp = self.xp + icon:width() + self.ident
end

function toolbar:seticons(icontable)
  self.icontable = icontable  
  for i=1, #icontable do
    local icon = icontable[i]    
	  icon:settoolbar(self)
    group.add(self, icon:setpos(self.xp, 0))	
	  self.xp = self.xp + icon:width() + self.ident
  end
  return self
end

function toolbar:onnotify(sender)  
  for i=1, #self.icontable do
    local icon = self.icontable[i]
	  if icon.istoggle_ and icon ~= sender then
	    icon:seton(false)
	  end
  end
end

return toolbar
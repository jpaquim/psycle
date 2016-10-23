-- psycle closebutton (c) 2015, 2016 by psycledelics
-- File: closebutton.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local item = require("psycle.ui.item")
local text = require("psycle.ui.text")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local signal = require("psycle.signal")

local closebutton = {}

function closebutton.new(parent)  
  local t = text:new(parent)
                :setautosize(false, false)				   
				:setposition(rect:new(point:new(0, 0), dimension:new(20, 15)))
                :settext("X")
	 	        :setverticalalignment(item.ALCENTER)
				:setjustify(text.CENTERJUSTIFY)
                :setalign(item.ALRIGHT)
				:setpadding(boxspace:new(1, 1, 1, 1))
  t:setdebugtext("close")				
  function t:onmousedown()
    self:setpadding(boxspace:new(1, 1, 1, 1))
    self:removeornaments()  
    self.dohide:emit()
  end      
  function t:onmousemove()  
    if not self:ornaments() then
	  self:setpadding(boxspace:new(0, 0, 0, 0))
      self:addornament(ornamentfactory:createlineborder(0xFFFFFE))
	end
  end
  function t:onmouseout()    
    self:setpadding(boxspace:new(1, 1, 1, 1))
    self:removeornaments()
  end
  t.dohide = signal:new()
  return t
end

return closebutton
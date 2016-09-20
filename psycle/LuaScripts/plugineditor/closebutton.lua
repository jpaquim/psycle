local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local text = require("psycle.ui.canvas.text")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local signal = require("psycle.signal")

local closebutton = {}

function closebutton.new(parent)
  local g = group:new(parent)
                 :setautosize(false, false)
				 :setposition(rect:new(point:new(0, 0), dimension:new(20, 10)))
				 :setalign(item.ALRIGHT)  
  g.closebtn = text:new(g)
                   :setautosize(false, false)
                   :settext("X")
	 		       :setverticalalignment(item.ALCENTER)
				   :setjustify(text.CENTERJUSTIFY)
                   :setalign(item.ALTOP)  
  function g.closebtn:onmousedown()
    local that = parent
     that:hide():parent():updatealign()
     g.dohide:emit()
  end  
  
  function g.closebtn:onmouseenter()  
    self:addornament(ornamentfactory:createlineborder(0xFFFFFE))
  end
  function g.closebtn:onmousemove()  
  end
  function g.closebtn:onmouseout()     
    self:removeornaments()
  end
  g.dohide = signal:new()
  return g
end



return closebutton;
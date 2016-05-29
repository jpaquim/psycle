local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local text = require("psycle.ui.canvas.text")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()

local closebutton = {}

function closebutton.new(parent)
  local g = group:new(parent):setpos(0, 0, 20, 0):setalign(item.ALRIGHT)  
  local closebtn = text:new(g)
                       :setautosize(false, true)
                       :settext("X")
                       :setalign(item.ALTOP)                       
  local that = parent
  function closebtn:onmousedown()
     that:hide():parent():updatealign()
  end  
  
  function closebtn:onmouseenter()  
    self:setornament(ornamentfactory:createlineborder(0xFFFFFE))
  end
  function closebtn:onmousemove()  
  end
  function closebtn:onmouseout()     
    self:setornament(nil)    
  end
  return self
end



return closebutton;
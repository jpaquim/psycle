local machine = require("psycle.machine"):new()

local param = require("psycle.parameter")
local osc = require("psycle.osc")
local shapes = {"sin", "saw", "square", "tri"}

function machine:info()
  return  {vendor="psycle", name="parameterdemo", generator=1, version=0, api=0}
end				   
				   
function machine:init()
    p = require("orderedtable"):new()

	p.shape = param:newknob("Waveform", "", 1, 4, 3, 1):addlistener(self)
    p.shape.display = function(self) 
                        return shapes[self:val()]
		  		      end				   
	self:addparameters(p)
	osc1 = osc:new(p.shape:val(), 440)
	osc1:start(0)
end

function machine:work()
  osc1:work(self:channel(0))
  self:channel(1):copy(self:channel(0))
end

function machine:ontweaked(param)
  if param==p.shape then
     osc1:setshape(param:val())
  end
end

return machine
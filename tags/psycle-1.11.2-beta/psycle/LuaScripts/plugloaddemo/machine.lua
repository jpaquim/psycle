-- plugloaddemo/machine.lua
-- demo module that loads and uses a vst plugin inside

-- get machine base
machine = require("psycle.machine"):new()

function machine:info()
  return {vendor="psycle examples", name="pluginloaddemo", generator = 0, version=0,
	      api=0}   
end

-- plugin constructor
function machine:init()   
   reverb = require("psycle.machine"):new("freeverb3vst-freeverb")
   p = require("orderedtable"):new()
   for i = 1, #reverb.params do
     p["fverb"..i] = reverb.params[i]
   end    
   self:addparameters(p);   
end

function machine:work(num)     
  reverb:setbuffer({self:channel(0),self:channel(1)});  
  reverb:work(num)  
end

return machine
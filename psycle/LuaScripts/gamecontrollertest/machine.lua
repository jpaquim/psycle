-- File: gamecontrollertest.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.

machine = require("psycle.machine"):new()

-- add your requires here
local maincanvas = require("maincanvas")


function machine:info()
  return {
    vendor = "psycle",
    name = "gamecontrollertest",
    mode = machine.FX,
    version = 0,
    api=0
  }
end

function machine:init()  
  self.maincanvas = maincanvas:new()  
  self:setcanvas(self.maincanvas)
end

function machine:work(num)  
end

return machine

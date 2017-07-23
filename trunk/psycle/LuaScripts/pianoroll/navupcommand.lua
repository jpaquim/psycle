local rawpattern = require("psycle.pattern"):new()

local navupcommand = {}

function navupcommand:new(...)
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  m:init(...)
  return m
end

function navupcommand:init(cursor)
  self.cursor_ = cursor
end

function navupcommand:execute()
  self.cursor_:decrow()  
end

return navupcommand

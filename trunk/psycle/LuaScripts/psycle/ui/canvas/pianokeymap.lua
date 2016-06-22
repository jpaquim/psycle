local midihelp = require("psycle.midi")

local pianokeymap = {}

function pianokeymap:new(first, last)
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  self.first_ = first
  self.last_ = last
  return m
end

function pianokeymap:label() return "piano"..last()-first() end
function pianokeymap:name(note) return midihelp.notename(note) end
function pianokeymap:first() return self.first_ end
function pianokeymap:last() return self.last_ end
function pianokeymap:range() return self.last_ - self.first_ end

return pianokeymap
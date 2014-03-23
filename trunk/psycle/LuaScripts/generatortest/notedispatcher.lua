local notedispatcher = {}

function notedispatcher:new(listener)
  local v = {}      
  setmetatable(v, self)
  self.__index = self  
  v.listener = listener
  return v
end

function notedispatcher::seqtick(channel, note, ins, cmd, val)
   if (note <= 119) then
      listener.noteon(
   end
end

return m
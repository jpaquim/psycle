--[[
  psycle parameter module
  file : psycle/parameter.lua
]]

local parameter = {MPFNULL = 0, MPFLABEL = 1,	MPFSTATE = 2}

function parameter:new(name, label, min, max, step, val, mpf, id)
  local o = {name_ = name,
			 label_ = label,
			 min_ = min,
			 max_ = max,			 
			 step_ = step,
			 val_ = (val-min)/(max-min),
			 mpf_ = mpf,			 
			 listener_ = {}}
  if id==nil then
    o.id_ = name
  else
    o.id_ = id;
  end
  setmetatable(o.listener_, {__mode ="kv"})  -- weak table
  setmetatable(o, self)
  self.__index = self
  return o
end

function parameter:__tostring()
   return self.name_
end

function parameter:newspace()
  return parameter:new("", 0, 0, 0, 0, 0, parameter.MPFNULL)
end

function parameter:newlabel(name)
  return parameter:new(name, 0, 0, 0, 0, 0, parameter.MPFLABEL)
end

function parameter:newknob(name, label, min, max, step, val)
   return parameter:new(name, label, min, max, step, val, parameter.MPFSTATE)
end

function parameter:range() 
  return self.min_, self.max_, self.step_
end

function parameter:mpf()
  return self.mpf_
end

function parameter:id()
  return self.id_
end

function parameter:name()
  return self.name_
end

function parameter:label()
  return self.label_
end

function parameter:val()
  return self.val_*(self.max_-self.min_)+self.min_
end

function parameter:setval(val)
  self:setnorm((val-self.min_)/(self.max_-self.min_))
end

function parameter:display()
  return self:val()
end

function parameter:norm()
  return self.val_
end

function parameter:setnorm(val)
  if self.val_ ~= val then
    self.val_ = val
    self:notify()
  end
end

function parameter:addlistener(listener)
  self.listener_[#self.listener_+1]=listener
  return self
end

function parameter:notify()
  for k, v in pairs(self.listener_) do      
     v:ontweaked(self)
  end
end

return parameter
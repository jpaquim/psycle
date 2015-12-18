local signal = {}

function signal:new()
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  m.listenerfunc_  = { }
  m.listenerinstance_ = { }
  setmetatable(m.listenerfunc_, {__mode ="kv"})  -- weak table  
  setmetatable(m.listenerinstance_, {__mode ="kv"})  -- weak table  
  return m
end

function signal:connect(func, instance)  
  local found = false
  for k=1, #self.listenerfunc_ do      
    if (self.listenerinstance_[k] == instance and
        self.listenerfunc_[k] == func) then	   
      found = true
	    break
	  end
  end
  if not found then
    self.listenerfunc_[#self.listenerfunc_ + 1] =  func
    self.listenerinstance_[#self.listenerinstance_ + 1] = instance
  end   
  return self  
end

function signal:emit(...)    
  for k = 1, #self.listenerfunc_ do    
    self.listenerfunc_[k](self.listenerinstance_[k], ...)
  end	  
  return self
end

return signal
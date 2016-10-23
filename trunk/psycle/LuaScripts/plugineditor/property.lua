local property = {}

function property:new(value, label, typename)  
  local c = {}
  setmetatable(c, self)
  self.__index = self
  c:init(value, label, typename)
  return c
end

function property:init(value, label, typename)
  self.label_ = ""    
  self.value_ = value  
  self.label_ = label  
  self.preventedit_ = false
  self.typename_ = typename
end

function property:label()
  return self.label_
end

function property:setvalue(value)
  self.value_ = value
end

function property:value()
  return self.value_
end

function property:tostring()
  local result, typename = "", self:typename()
  if typename == "color" then    
    result = string.format("%x", self.value_)  
  else  
    result = self.value_..""
  end
  return result
end

function property:write(file)
  file:write('property:new(')  		  
  if self:typename() == "color" then		    
    file:write("0x"..self:tostring())
  elseif self:typename() == "boolean" then
    if self:value() == "true" then
      file:write("true") 
    else
      file:write("false")
    end		    
  elseif self:typename() == "string" then
	file:write('"'..self:tostring()..'"')
  elseif self:typename() == "number" then
    file:write(self:tostring())
  end		 		  
  if self:typename() == "color" then
    file:write(', "'.. self:label()..'", "color")')
  else
    file:write(', "'.. self:label()..'")')
   end
end

function property:typename()
  local result = "unknown"
  if not self.typename_ then
    result = type(self.value_)
  else
    result = self.typename_
  end
  return result
end

function property:enableedit()
  self.editable_ = true
end

function property:preventedit()
  self.editable_ = false
end

function property:iseditprevented()
  return self.preventedit_
end

function property:settypename(typename)
  self.typename_ = typename
end

return property
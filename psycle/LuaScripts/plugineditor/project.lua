local project = {}

function project:new()
  local m = {}
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function project:init()   
end

function project:setplugininfo(plugininfo)
  self.plugininfo_ = plugininfo
  return self
end

function project:plugininfo()
  return self.plugininfo_
end

return project
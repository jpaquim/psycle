local project = {}

function project:new()
  local m = {}
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function project:init()   
  self.pluginindex_ = -1
end

function project:setplugininfo(plugininfo)
  self.plugininfo_ = plugininfo
  return self
end

function project:plugininfo()
  return self.plugininfo_
end

function project:setpluginindex(index)
  self.pluginindex_ = index
end

function project:pluginindex()
  return self.pluginindex_
end

return project
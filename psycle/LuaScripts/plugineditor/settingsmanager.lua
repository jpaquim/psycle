local cfg = require("psycle.config"):new("PatternVisual")
local property = require("property")
local filehelper = require("psycle.file")
local serpent = require("psycle.serpent")

local settingsmanager = {}

local endl = "\n" 

function settingsmanager:new(pluginname)  
  local c = {}
  setmetatable(c, self)
  self.__index = self
  c:init(pluginname)
  return c
end

function filenamewithoutextension(filename)
  local result = ""
  local found, len, remainder = string.find(filename, "^(.*)%.[^%.]*$")
  if found then
    result = remainder
  else
    result = path
  end
  return result		
end	

function settingsmanager:init(pluginname) 
  self.pluginname_ = pluginname   
  self:loadglobal()
  self:load(filenamewithoutextension(self.globalsetting_.settingname))
end

function settingsmanager:loadglobal()
  local path = cfg:presetpath().."\\" .. self.pluginname_ .. "\\global.lua"    
  local f = loadfile(path)
  if f then
    self.globalsetting_ = f()
  else
    local path = cfg:presetpath()
    filehelper.mkdir(path .. "\\" .. self.pluginname_)  
    self.globalsetting_ = require(self.pluginname_ .. ".global")
	self:saveglobal("default")    
  end
  return self
end

function settingsmanager:saveglobal(name)
  self.globalsetting_.settingname = name .. ".lua"
  local endl = "\n"
  local path = cfg:presetpath() .. "\\" .. self.pluginname_ .. "\\global.lua"
  local file,err = io.open(path, "wb")
  if err then
    return err 
  end
  file:write([[
local global = {
   settingname = "]] .. self.globalsetting_.settingname .. [["
 }
 
return global
]])    
  return self
end

function settingsmanager:setting()
  return self.setting_
end

function settingsmanager:settingname()  
  return filenamewithoutextension(self.globalsetting_.settingname)
end

function settingsmanager:path()
  return cfg:presetpath() .. "\\" .. self.pluginname_ .. "\\" .. self.globalsetting_.settingname
end

function settingsmanager:load(name)  
  if name .. ".lua"  ~= self.globalsetting_.settingname then     
    self:saveglobal(name)
  end  
  local f = loadfile(self:path())
  if f then
    self.setting_ = f()		
	local default = require(self.pluginname_ .. "..defaultsetting")			
	if default.meta.version ~= self.setting_.meta.version then	  
	  self:extractmeta()	
	  self:mergewithdefault(default, self.setting_)
	  self.setting_ = default	  
      self:extractmeta()
	  self:save()	  
	else	  
      self:extractmeta()
    end	
  else  
    self:copydefault()	
	self.setting_ = dofile(self:path())	
	self:extractmeta()	
  end
  return self
end

function settingsmanager:restoredefault()  
  self.setting_ = require(self.pluginname_ .. ".defaultsetting")  
  self.globalsetting_.settingname = "default.lua"
  self:extractmeta()  
  self:save()  
  self:saveglobal("default")
end

function settingsmanager:copydefault()
  local path = cfg:presetpath()
  filehelper.mkdir(path .. "\\" .. self.pluginname_) 
  local default_path = cfg:luapath() .. "\\" .. self.pluginname_ .. "\\defaultsetting.lua"
  
  infile = io.open(default_path, "r")
  instr = infile:read("*a")
  infile:close()
  self.globalsetting_.settingname = "default.lua"
  outfile = io.open(self:path(), "w")
  outfile:write(instr)
  outfile:close()
end

function settingsmanager:extractmeta()
  if self.setting_.meta then
    self.meta = self.setting_.meta
    self.setting_:del("meta")
  end
end

function settingsmanager:mergewithdefault(defaultsetting, currentsetting)    
  for name, settinggroup in defaultsetting:opairs() do       
    self:mergelabel(settinggroup, currentsetting)	    			
    self:mergeproperties(settinggroup, currentsetting)
	if settinggroup.children and currentsetting.children then
	  self:mergewithdefault(settinggroup, settinggroup.children)
	end
  end  
end

function settingsmanager:mergelabel(settinggroup, currentsetting)
  if settinggroup.label then	  
	if currentsetting[name] and currentsetting[name].label then
	  settinggroup.label = currentsetting[name].label
	end
   end	
end

function settingsmanager:mergeproperties(name, settinggroup, currentsetting)
  if settinggroup.properties and currentsetting.properties then
	for name, property in pairs(settinggroup.properties) do
	  if currentsetting.properties[name] then
	    property = currentsetting.properties[name]
	  end
	end
  end	  
  return self
end

function settingsmanager:saveas(name)  
  self:saveglobal(name)
  self:save()  
  self.globalsetting_.settingname = name .. "lua"
end

function settingsmanager:save()
  local setting = self:setting()  
  local file,err = io.open(self:path(), "wb")
  if err then
    return err 
  end
  file:write([[
local orderedtable = require("psycle.orderedtable")
local fontinfo = require("psycle.ui.fontinfo")
local stock = require("psycle.stock")
local property = require("property")

local settings = orderedtable.new()  

settings.meta = {
  name = "]] .. self.meta.name .. [[",
  version = ]] .. self.meta.version .. 
[[

}

]])
  self:saverec(file, "settings", setting)
 file:write([[
 
return settings
 ]] .. endl)
 file.close()
 return self
end

function settingsmanager:saverec(file, prefix, setting) 
 for name, settinggroup in setting:opairs() do    
    file:write(prefix .. '.' .. name .. " = {}" .. endl)	
    for key, value in pairs(settinggroup) do	  	  
	  local prefix = prefix .. "." .. name .. "." .. key
	  if key == "label" then	    
	    file:write(prefix .. ' = "' .. value..'"' .. endl)
      elseif key == "properties" then	    
	    file:write(prefix .. " = orderedtable.new()"..endl)		
	    for propname, property in value:opairs() do		  		  
		  file:write(prefix .. "." .. propname .. ' = ')
		  property:write(file)
          file:write(endl)		  
		end
		file:write(endl)       
	  elseif key == "children" then
	    file:write(prefix .. " = orderedtable.new()" .. endl)	
	    self:saverec(file, prefix, settinggroup.children)
	  end
    end	
	file:write(endl)
  end
end

return settingsmanager
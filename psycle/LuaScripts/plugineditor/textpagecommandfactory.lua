--[[ psycle plugineditor (c) 2017 by psycledelics
File: textpagecommands.lua
copyright 2017 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.  
]]

local boxspace = require("psycle.ui.boxspace")
local command = require("psycle.command")
local textpage = require("textpage")
local systems = require("psycle.ui.systems")
local filesave = require("psycle.ui.filesave")
local fileopen = require("psycle.ui.fileopen")

local blockbegincommand = command:new()

function blockbegincommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function blockbegincommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():setblockbegin():setfocus()
  end
end

local blockendcommand = command:new()

function blockendcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer  
  return c
end

function blockendcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():setblockend():setfocus()
  end
end

local blockdeletecommand = command:new()

function blockdeletecommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function blockdeletecommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():deleteblock():setfocus()
  end
end

local charleftcommand = command:new()

function charleftcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function charleftcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():charleft():setfocus()
  end
end

local charrightcommand = command:new()

function charrightcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function charrightcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():charright():setfocus()
  end
end

local charleftcommand = command:new()

function charleftcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function charleftcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():charleft():setfocus()
  end
end

local lineupcommand = command:new()

function lineupcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function lineupcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():lineup():setfocus()
  end
end

local linedowncommand = command:new()

function linedowncommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function linedowncommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():linedown():setfocus()
  end
end

local wordleftcommand = command:new()

function wordleftcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function wordleftcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():wordleft():setfocus()
  end
end

local wordrightcommand = command:new()

function wordrightcommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function wordrightcommand:execute()
  if self.pagecontainer_:activepage() then   
    self.pagecontainer_:activepage():wordright():setfocus()
  end
end

local commandfactory = {}

commandfactory.SETBLOCKBEGIN = 1
commandfactory.SETBLOCKEND = 2
commandfactory.BLOCKDELETE = 3
commandfactory.CHARLEFT = 4
commandfactory.CHARRIGHT = 5
commandfactory.LINEUP = 6
commandfactory.LINEDOWN = 7
commandfactory.WORDLEFT = 8
commandfactory.WORDRIGHT = 9

function commandfactory.build(cmd, ...)
  local result = nil
  if cmd == commandfactory.SETBLOCKBEGIN then
    result = blockbegincommand:new(...)
  elseif cmd == commandfactory.SETBLOCKEND then
    result = blockendcommand:new(...)
  elseif cmd == commandfactory.BLOCKDELETE then
    result = blockdeletecommand:new(...)  
  elseif cmd == commandfactory.CHARLEFT then
    result = charleftcommand:new(...)  
  elseif cmd == commandfactory.CHARRIGHT then
    result = charrightcommand:new(...)
  elseif cmd == commandfactory.LINEUP then
    result = lineupcommand:new(...)
  elseif cmd == commandfactory.LINEDOWN then
    result = linedowncommand:new(...)   
  elseif cmd == commandfactory.WORDLEFT then
    result = wordleftcommand:new(...)
  elseif cmd == commandfactory.WORDRIGHT then
    result = wordrightcommand:new(...)   
  end
  return result
end

return commandfactory
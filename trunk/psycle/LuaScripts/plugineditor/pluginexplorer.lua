-- psycle plugineditor (c) 2015 by psycledelics
-- File: pluginexplorer.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local tree = require("psycle.ui.canvas.tree"):new()
local treeitem = require("psycle.ui.canvas.texttreeitem")
local machine = require("psycle.machine"):new()
local settings = require("settings")
local signal = require("psycle.signal")

local pluginexplorer = tree:new()

function pluginexplorer:new(parent)  
  local c = tree:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function pluginexplorer:init()
  self.click = signal:new() 
end

function pluginexplorer:dirLookup(directory)
  local p = io.popen('dir "'..directory..'\\*.lua" /B')  
  for file in p:lines() do   
    local item = treeitem:new():settext(file)   
    item.path = directory
    item.filename = file   
    local that = self
    function item:onclick()
      local ev = { 
        sender = that, 
        path = item.path,
        filename = item.filename         
      }
      that.click:emit(ev)
    end
    self:add(item)
  end
end

function getpath(str,sep)
    sep=sep or'/'
    return str:match("(.*"..sep..")")
end

function scriptpath()
  local str = debug.getinfo(2, "S").source:sub(2)  
  return getpath(str, "\\")
end

function pluginexplorer:setfilepath(path)
  -- local path = scriptpath()
  self:clear()
  self:dirLookup(path) 
end

return pluginexplorer
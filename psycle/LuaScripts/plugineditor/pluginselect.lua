-- psycle pluginselector (c) 2015 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local listview = require("psycle.ui.canvas.listview")
local node = require("psycle.node")
local filehelper = require("psycle.file")
local signal = require("psycle.signal")
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local settings = require("settings")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local closebutton = require("closebutton")
local text = require("psycle.ui.canvas.text")

local pluginselector = group:new()

function pluginselector:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function pluginselector:init()
   
end



return pluginselector
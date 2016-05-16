-- psycle plugineditor (c) 2015 by psycledelics
-- File: menudesigner.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local canvas = require("psycle.ui.canvas")
local group = require("psycle.ui.canvas.group")
local item = require("psycle.ui.canvas.item")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local tree = require("psycle.ui.canvas.tree")
local node = require("psycle.node")
local edit = require("psycle.ui.canvas.edit")

local menudesigner = canvas:new()

function menudesigner:new(parent)
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()  
  return c
end

function menudesigner:init() 
  self:setautosize(false, false)    
  self:setpos(0, 0, 100, 0)
  self:setornament(ornamentfactory:createfill(0x528A68))
  self.edit = edit:new(self):setautosize(false, false):setalign(item.ALTOP):setpos(0, 0, 0, 20)
  self.tree = tree:new(self):setautosize(false, false):setalign(item.ALCLIENT)  
  self.rootnode = node:new() 
  local node = node:new():settext("empty")
  self.rootnode:add(node)
  local node = node:new():settext("empty1")
  self.rootnode:add(node)
  local node = node:new():settext("empty2")
  self.rootnode:add(node)
  self.tree:setrootnode(self.rootnode)
  function self.tree:onkeydown(ev)
    psycle.output("onkeydown")
    ev:preventdefault()
  end    
end


return menudesigner
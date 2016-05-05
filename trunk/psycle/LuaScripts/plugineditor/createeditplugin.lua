-- psycle plugineditor (c) 2015 by psycledelics
-- File: search.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local item = require("psycle.ui.canvas.item")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local edit = require("psycle.ui.canvas.edit")
local iconbutton = require("psycle.ui.canvas.toolicon")
local checkbox = require("psycle.ui.canvas.checkbox")
local signal = require("psycle.signal")
local settings = require("settings")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local catcher = require("psycle.plugincatcher")
local machine = require("psycle.machine")
local serpent = require("psycle.serpent")
local cfg = require("psycle.config"):new("PatternVisual")
local filehelper = require("psycle.file")
local signal = require("psycle.signal")
local checkbox = require("psycle.ui.canvas.checkbox")
local button = require("psycle.ui.canvas.button")

local createeditplugin = group:new()

function createeditplugin:new(parent)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init()  
  return c
end

function createeditplugin:init() 
  self:setautosize(false, false)    
  self:setpos(0, 0, 100, 100)
  self:setornament(ornamentfactory:createfill(0x528A68))    
  self:initnameprompt()    
  self:initpluginlist()  
  self:initcreateoptions()
  self.docreate = signal:new()  
  self.doopen = signal:new()
end

function createeditplugin:initnameprompt()
  local g = group:new(self):setautosize(true, true):setalign(item.ALTOP)
  text:new(g):settext("Name of Plugin to Edit?"):setalign(item.ALLEFT)
  self.nameedit = edit:new(g):setautosize(false, false):setpos(0, 0, 200, 20):setalign(item.ALLEFT):setmargin(5, 0, 0, 0)
  local that = self
  function self.nameedit:onkeydown(ev)    
    if ev:keycode() == 13 then
      that:createoredit()
    end
  end   
end

function createeditplugin:createoredit()
  local catcher = catcher:new()
  local infos = catcher:infos()  
  local found = false
  for i=1, #infos do
    if infos[i]:type() == machine.MACH_LUA then
      if infos[i]:name() == self.nameedit:text() then
        self.doopen:emit(self.nameedit:text())        
        self:hide():parent():updatealign()
        found = true
        break
      end
    end
  end
  if not found then
    self.pluginlist:hide()
    self.createoptions:show()
    self:updatealign()    
  end
end

function createeditplugin:initpluginlist()  
  self.pluginlist = group:new(self):setautosize(true, true):setalign(item.ALTOP)  
  self:updatepluginlist()  
end

function createeditplugin:initcreateoptions()  
  self.createoptions = group:new(self):setautosize(true, true):setalign(item.ALTOP):hide()
  self.machmode = checkbox:new(self.createoptions):settext("Is Generator"):setalign(item.ALTOP)      
  local btn = button:new(self.createoptions):settext("Create"):setalign(item.ALTOP)  
  local that = self
  function btn:onclick() that:createplugin() end  
end

function createeditplugin:createplugin()  
  if not filehelper.isdirectory(self.nameedit:text()) then    
    self.docreate:emit(self.nameedit:text())
    self:hide()
    self.createoptions:hide()
    self.pluginlist:show()    
    self:parent():updatealign()
  end 
end

function createeditplugin:updatepluginlist()
  local catcher = catcher:new()
  local infos = catcher:infos()    
  local lua_count = 0
  for i=1, #infos do
    if infos[i]:type() == machine.MACH_LUA then
      text:new(self.pluginlist):setautosize(true, true):settext(infos[i]:name()):setmargin(0, 0, 10, 0)
      lua_count = lua_count + 1
    end
  end   
  self.pluginlist:setaligner(math.floor(lua_count / 3), 3)
end

return createeditplugin
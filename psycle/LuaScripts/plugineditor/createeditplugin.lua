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
local closebutton = require("closebutton")
local pic = require("psycle.ui.canvas.pic")
local image = require("psycle.ui.image")
local listview = require("psycle.ui.canvas.listview")
local node = require("psycle.node")
local templateparser = require("templateparser")
local pluginselector = require("pluginselector")

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
  self:setpos(0, 0, 100, 300)
  self:addornament(ornamentfactory:createfill(0x528A68))
  closebutton.new(self)
  --self:initnameprompt()         
  self:initcreateoptions()
  self.docreate = signal:new()  
  self.doopen = signal:new()
  self.pluginselector = pluginselector:new(self):setautosize(false, false):setalign(item.ALTOP):setpos(0, 0, 0, 200)
end

function createeditplugin:initnameprompt()
  local g = group:new(self):setautosize(true, true):setalign(item.ALBOTTOM)  
  self.nameedit = edit:new(g):settext("Search or create plugin"):setautosize(false, false):setpos(0, 0, 200, 20):setalign(item.ALLEFT):setmargin(5, 0, 0, 0)
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
        local dir = self:machinepath(infos[i])        
        self:hide()        
        self.doopen:emit(dir, infos[i]:name(), infos[i]) 
        found = true
        break
      end
    end
  end
  if not found then
    self.pluginlist:hide()
    self.createoptions:show()
    self:updatealign()
    self:parent():updatealign()
    self:parent():invalidate()
  end
end

function createeditplugin:initpluginlist()    
  self.pluginlist = listview:new(self)
                            :setautosize(false, false)
                            :setpos(0, 0, 0, 200)
                            :setalign(item.ALTOP)
                            :setbackgroundcolor(0x528A68)
                            :settextcolor(0xFFFF00)
  local that = self
  function self.pluginlist:onchange(node)
    local dir = that:machinepath(node.info)        
    that:hide()        
    that.doopen:emit(dir, node.info:name(), node.info)  
  end
  self:updatepluginlist()  
end

function createeditplugin:initcreateoptions()  
  self.createoptions = group:new(self):setautosize(true, true):setalign(item.ALTOP):setmargin(0, 5, 0, 5):hide()  
  self.templatelist = listview:new(self.createoptions):setalign(item.ALLEFT):setautosize(false, false):setpos(0, 0, 200, 140)
  self.listnode = node:new()  
  local filetree = filehelper.filetree(cfg:luapath().."\\plugineditor\\templates")    
  if filetree then        
    for element in node_iter(filetree).next do            
      if element.extension == ".lu$" then          
        local str = templateparser.evaluate(element.path..element.filename)     
        if str == "" then            
        else
          local node = node:new():settext(str)
          node.path = element.path..element.filename
          self.listnode:add(node)
        end
      end  
    end
  end
  self.templatelist:setrootnode(self.listnode)
  local that = self    
  function self.templatelist:onchange(node)
    that:createplugin(node.path)
  end  
end

function createeditplugin:createplugin(templatepath)  
  if not filehelper.isdirectory(self.nameedit:text()) then
    self:hide()
    self.createoptions:hide()
    self.pluginlist:show()    
    self.docreate:emit(self.nameedit:text(), templatepath)    
  end 
end

function createeditplugin:machinepath(info)  
  local file = io.open(info:dllname(), "r")  
  local str = ""
  for line in file:lines() do
    str = string.match(line, "require(%b())")
    if str then
      str = str:sub(3, -3)      
      str = str:gsub("%.", "\\")      
      break
    end      
  end
  file:close()    
  return str
end

function createeditplugin:updatepluginlist()
  local catcher = catcher:new()
  local infos = catcher:infos()    
  self.rootnode = node:new()
  local lua_count = 0
  for i=1, #infos do
    if infos[i]:type() == machine.MACH_LUA then      
      local node = node:new():settext(infos[i]:name())      
      node.info = infos[i]      
      self.rootnode:add(node)      
    end
  end 
  self.pluginlist:setrootnode(self.rootnode)  
end

return createeditplugin
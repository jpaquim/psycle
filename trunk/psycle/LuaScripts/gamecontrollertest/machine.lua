-- File: gamecontrollertest.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.


local node = require("psycle.node")
machine = require("psycle.machine"):new()


-- add your requires here
local mainviewport = require("mainviewport")

function machine:info()
  return {
    vendor = "psycle",
    name = "gamecontrollertest",
    mode = machine.FX,
    version = 0,
    api=0
  }
end

function machine:init()    
  self.mainviewport = mainviewport:new(self)    
  self:setviewport(self.mainviewport)  
  self.menu = self:menus()
  psycle.setmenurootnode(self.menu.root)
end

function machine:work(num)  
end

function machine:menus()
   local menus = {}
   menus.root = node:new():settext("Plugineditor")   
   menus.filemenu = node:new():settext("File")
   menus.root:add(menus.filemenu)
   menus.newmenu = node:new():settext("New")
   menus.filemenu:add(menus.newmenu)   
   menus.newfile = node:new():settext("New File")
   menus.newmenu:add(menus.newfile)
   menus.newgroup = node:new():settext("New Group")
   menus.newmenu:add(menus.newgroup)
   menus.newmodule = node:new():settext("New Module")
   menus.newmenu:add(menus.newmodule)   
   menus.openfile = node:new():settext("Open")
   menus.filemenu:add(menus.openfile) 
   --- tools
   menus.toolmenu = node:new():settext("Tools")
   menus.root:add(menus.toolmenu)
   menus.menudesigner = node:new():settext("Menu Designer")
   menus.toolmenu:add(menus.menudesigner)
   menus.separator = node:new():settext("-")
   menus.root:add(menus.separator)
   return menus
end

return machine

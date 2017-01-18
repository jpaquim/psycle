--[[ psycle plugineditor (c) 2017 by psycledelics
File: commands.lua
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

local newpagecommand = command:new()

function newpagecommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function newpagecommand:execute()
  local page = textpage:new(nil)
                       :setmargin(boxspace:new(2, 0, 0, 0))
  systems:new():updatewindow(textpage.windowtype, page)                  
  self.pagecontainer_:addpage(page, page:createdefaultname())  
end

local savepagecommand = command:new()

function savepagecommand:new(pagecontainer, fileexplorer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  c.fileexplorer_ = fileexplorer
  c.filesaveas = filesave:new() 
  return c
end

function savepagecommand:execute()
  local page = self.pagecontainer_:activepage()
  if page then
    local fname = ""     
    if page:hasfile() then
      fname = page:filename()    
      local sep = "\\"  
      local dir = fname:match("(.*" .. sep .. ")")    
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pagecontainer_:setlabel(page, fname)
    else    
      self.filesaveas:setfolder(self.fileexplorer_:path())
      if self.filesaveas:show() then      
        fname = self.filesaveas:filename()          
        page:savefile(fname)
        fname = fname:match("([^\\]+)$")
        self.pagecontainer_:setlabel(page, fname)
      end
    end
  end  
end

local undopagecommand = command:new()

function undopagecommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function undopagecommand:execute()
  if self.pagecontainer_:activepage() then
    self.pagecontainer_:activepage():undo()
  end
end

local redopagecommand = command:new()

function redopagecommand:new(pagecontainer)
  local c = command:new()  
  setmetatable(c, self)
  self.__index = self
  c.pagecontainer_ = pagecontainer
  return c
end

function redopagecommand:execute()
  if self.pagecontainer_:activepage() then
    self.pagecontainer_:activepage():redo()
  end
end

commandfactory = {}

commandfactory.NEWPAGE = 1
commandfactory.SAVEPAGE = 2
commandfactory.REDOPAGE = 3
commandfactory.UNDOPAGE = 4

function commandfactory.build(cmd, ...)
  local result = nil
  if cmd == commandfactory.NEWPAGE then
    result = newpagecommand:new(...)
  elseif cmd == commandfactory.SAVEPAGE then
    result = savepagecommand:new(...)
  elseif cmd == commandfactory.REDOPAGE then
    result = redopagecommand:new(...)
  elseif cmd == commandfactory.UNDOPAGE then
    result = undopagecommand:new(...)
  end 
  return result
end

return commandfactory
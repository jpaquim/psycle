-- psycle fileexplorer (c) 2016 by psycledelics
-- File: fileexplorer.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local signal = require("psycle.signal")
local node = require("psycle.node")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local edit = require("psycle.ui.canvas.edit")
local listview = require("psycle.ui.canvas.listview")
local text = require("psycle.ui.canvas.text")
local filehelper = require("psycle.file")
local settings = require("settings")
local closebutton = require("closebutton")

local fileexplorer = group:new()

function fileexplorer:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function fileexplorer:init()
   self:initlistview()   
   self:initicons()
   self:initheader()
   self:initdefaultstyle()
   self:initsignals()   
end

function fileexplorer:initheader()
  self:setdebugtext("left")
  self.header = group:new(self)
                     :setalign(item.ALTOP)
                     :setautosize(false, false)
					 :setposition(rect:new(point:new(0, 0), dimension:new(0, 20)))
					 :addornament(ornamentfactory:createlineborder(0x3F3F3F))					 
  self:initmachineselector()					 
  local closebutton = closebutton.new(self.header)
  local that = self
  function closebutton:onmousedown()
     that:setposition(rect:new(point:new(0, 0), dimension:new(0, 0)))
	 that:parent():flagnotaligned():updatealign()     
  end 
end

function fileexplorer:initmachineselector()
  self.machineselector = edit:new(self.header)
                             :setautosize(false, false)
							 :settext("Name of Plugin to Edit?")
							 :setposition(rect:new(point:new(0, 0), dimension:new(0, 20)))
							 :setalign(item.ALCLIENT)
							 :setbackgroundcolor(0x2F2F2F)
							 :settextcolor(0xCACACA)							 
							-- :setjustify(text.CENTERJUSTIFY)
							-- :setverticalalignment(item.ALCENTER)
  local that = self
  function self.machineselector:onmousedown()    
    that.onselectmachine:emit()
  end
end

function fileexplorer:initlistview()
  self.listview = listview:new(self)
                          :setalign(item.ALCLIENT)
						  :setautosize(false, false)
						  :viewlist()			
						  :setmargin(boxspace:new(10, 0, 0, 10))
  local that = self
  function self.listview:ondblclick(ev)    
	local node = self:selected()
	if node and not node.isdirectory then  
	  local ev = { 
		sender = self, 
		path = node.path,
		filename = node.filename,
		extension = node.extension
	  }
	  that.click:emit(ev)
	elseif node then
	  that:setpath(node.path.."\\"..node.filename)
	end
  end
end

function fileexplorer:initdefaultstyle()
  self:addornament(ornamentfactory:createfill(0x2F2F2F))
  self.listview:setbackgroundcolor(0x2F2F2F)
  self.listview:settextcolor(0xFFFFFF)  
end

function fileexplorer:initicons()
  self.text = image:new():load(settings.picdir.."document.png")
  self.textlua = image:new():load(settings.picdir.."document.png")
  self.folder = image:new():load(settings.picdir.."folder_small.png")
  self.folder = image:new():load(settings.picdir.."folder_small.png")  
  self.images = images:new()
  self.images:add(self.text):add(self.folder):add(self.textlua)   
  self.listview:viewsmallicon()
  self.listview:setimages(self.images)  
end

function fileexplorer:initsignals()
  self.click = signal:new()
  self.onremove = signal:new()
  self.onselectmachine = signal:new()
end

function fileexplorer:setpath(path)
  local dir = filehelper.directorylist(path)
  self.rootnode = node:new()
  self:addparentdirectorynode(path)
  for i=1, #dir do
    if dir[i] then      
      local node = node:new():settext(dir[i].filename)
	  node.filename = dir[i].filename
	  node.isdirectory = dir[i].isdirectory
	  node.extension = dir[i].extension
	  node.path = dir[i].path	  
	  if node.isdirectory then	    
        node:setimageindex(1):setselectedimageindex(1)
      elseif node.extension == ".lua" then
        node:setimageindex(2):setselectedimageindex(2)
      end
	  
	  self.rootnode:add(node)   
	end
  end    
  self.listview:setrootnode(self.rootnode) 
  self.listview:updatelist()
end

function fileexplorer:setmachinename(name)
  self.machineselector:settext(name)
end

function fileexplorer:addparentdirectorynode(path)
  local parentdirectory = filehelper.parentdirectory(path)
  if parentdirectory then
	local node = node:new():settext(".. ["..parentdirectory.filename.."]")
	node:setimageindex(1):setselectedimageindex(1)
	node.isdirectory = true
	node.filename = parentdirectory.filename
	node.extension = ""  
	node.path = parentdirectory.path
	self.rootnode:add(node)  
  end
end

return fileexplorer
-- psycle pluginselector (c) 2016 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local cfg = require("psycle.config"):new("PatternVisual")
local machine = require("psycle.machine")
local catcher = require("psycle.plugincatcher")
local signal = require("psycle.signal")
local node = require("psycle.node")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local listview = require("psycle.ui.canvas.listview")
local button = require("psycle.ui.canvas.button")
local edit = require("psycle.ui.canvas.edit")
local text = require("psycle.ui.canvas.text")
local filehelper = require("psycle.file")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local closebutton = require("closebutton")
local pluginselector = group:new()
local templateparser = require("templateparser")
local settings = require("settings")

function pluginselector:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function pluginselector:init()   
   self.doopen = signal:new()
   self.docreate = signal:new()
   self.tg = tabgroup:new(self)
                     :setalign(item.ALCLIENT)
					 :disableclosebutton()
   local g = group:new()
                  :setautosize(false, false)
   self:initpluginlist(g)
   self.creategroup = self:initplugincreate()
   self.creategroup.propertypage = nil
   self.tg:addpage(g, "All")
   local closebutton = closebutton.new(self.tg.tabbar)
                                  :setalign(item.ALRIGHT)
   closebutton.dohide:connect(pluginselector.onhide, self)   
end

function pluginselector:onhide()
  self:hide():parent():updatealign()     
end

function pluginselector:initplugincreate()
  local g = group:new():setautosize(false, false)
  self.tg:addpage(g, "Create")   
  self.plugincreatelist = listview:new(g)
                                  :setalign(item.ALCLIENT)
								  :setautosize(false, false)
                                  :setbackgroundcolor(0x3E3E3E)
                                  :settextcolor(0xFFFFFF)								  								  
  local path = cfg:luapath().."\\plugineditor\\templates"
  self.plugincreatelistrootnode = node:new()    
  local dir = filehelper.directorylist(path)    
  for i=1, #dir do
    if not dir[i].isdirectory and dir[i].extension == ".lua" then
	  local fname = string.sub(dir[i].filename, 1, -5)      
      local template = require("templates."..fname)	   
	  local newnode = node:new():settext(template.label)
	  newnode.template = template
	  self.plugincreatelistrootnode:add(newnode)
	end
  end        
  self.plugincreatelist:setrootnode(self.plugincreatelistrootnode)  
  local that = self
  function self.plugincreatelist:onchange(node)    
    that.template = node.template
	if that.creategroup.propertypage then
	  that.creategroup:remove(that.creategroup.propertypage)
	  that.creategroup.propertypage = nil	  
	end
	that.creategroup.propertypage = that:createproperties(node.template.properties)	
	that:parent():flagnotaligned()
	that:parent():updatealign()
  end  
  return g
end

function pluginselector:createproperties(properties)
  local g = group:new(self.creategroup)
                 :setautosize(false, false)
                 :setposition(rect:new(point:new(0, 0), dimension:new(200, 0)))
				 :setalign(item.ALRIGHT)
  g:addornament(ornamentfactory:createfill(0x2F2F2F))
  if (properties) then
	for i=1, #properties do
	  if properties[i].edit then	  	  
		local div = group:new(g)
		                 :setautosize(false, false)
						 :setalign(item.ALTOP)
						 :setposition(rect:new(point:new(0, 0), dimension:new(0, 20)))
		g[properties[i].property] = edit:new(div):setautosize(false, false)
		                                :setposition(rect:new(point:new(0, 0), dimension:new(100, 20)))
										:setalign(item.ALCLIENT)
										:settext(properties[i].value)										
		text:new(div):setautosize(true, true)
		             :setalign(item.ALLEFT)
		             :settext(properties[i].property)				
                     :setmargin(boxspace:new(0, 5, 0, 0))
      end	
	end
  end
  local b = button:new(g):settext("Create"):setalign(item.ALBOTTOM)
  local that = self
  function b:onclick()
   that:oncreateplugin(that.template)  
  end
  return g
end

function pluginselector:oncreateplugin(template)
  local filters = template.filters  
  local env = {}  
  local properties = self.template.properties
  for i=1, #properties do
	if properties[i].edit then
	  env[properties[i].property] = self.creategroup.propertypage[properties[i].property]:text()
	else
	  if properties[i].type == "string" then
	    env[properties[i].property] = properties[i].value
	  elseif properties[i].type == "machtype" then
	    if properties[i].value == "fx" then
		  env[properties[i].property] = "machine.FX"
		elseif properties[i].value == "generator" then
	      env[properties[i].property] = "machine.GENERATOR"
	    end
	  end
	end	  
  end	      
  filehelper.mkdir(env.pluginname)
  templateparser.work(cfg:luapath().."\\plugineditor\\templates\\pluginregister.lu$",
	 		          cfg:luapath().."\\"..env.pluginname..".lua",
					  env)
  for i=1, #filters do	
    local templatepath = cfg:luapath().."\\plugineditor\\templates\\"..filters[i].templatepath
    templateparser.work(templatepath,
                        cfg:luapath().."\\"..env.pluginname.."\\"..filters[i].outputpath,
						env)    
  end 
  self.docreate:emit(template, env.pluginname)  
end  

function pluginselector:initpluginlist(parent)
  self.pluginlist = listview:new(parent)
                            :setautosize(false, false)
                            :setposition(rect:new(point:new(0, 0), dimension:new(0, 200)))
                            :setalign(item.ALTOP)
                            :setbackgroundcolor(0x3E3E3E)
                            :settextcolor(0xFFFFFF)
							:setmargin(boxspace:new(0, 5, 0, 0))
  local that = self  
  function self.pluginlist:onchange(node)
    local dir = that:machinepath(node.info)        
    that:hide()        
    that.doopen:emit(dir, node.info:name(), node.info)  
  end  
  self:updatepluginlist()  
end

function pluginselector:machinepath(info)  
  local file = io.open(info:dllname(), "r")  
  local str = ""
  for line in file:lines() do
    local pos = string.find(line, "psycle.setmachine")
	if pos then
      str = string.match(line, "require(%b())")	  
      if str then
        str = str:sub(3, -3)      
        str = str:gsub("%.", "\\")      
        break
      end
    end	
  end
  file:close()    
  return str
end

function pluginselector:updatepluginlist()
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
  self.pluginlist:updatelist()
end

return pluginselector
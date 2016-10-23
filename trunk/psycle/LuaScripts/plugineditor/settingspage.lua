-- psycle pluginselector (c) 2016 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local cfg = require("psycle.config"):new("PatternVisual")
local filehelper = require("psycle.file")
local node = require("psycle.node")
local signal = require("psycle.signal")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local button = require("psycle.ui.button")
local edit = require("psycle.ui.edit")
local combobox = require("psycle.ui.combobox")
local text = require("psycle.ui.text")
local tree = require("psycle.ui.treeview")
local checkbox = require("psycle.ui.canvas.checkbox")
local propertiesview = require("propertiesview")
local settings = require("settings")
local property = require("property")
local systems = require("psycle.ui.systems")

local settingspage = group:new()

settingspage.hasinstance = false

function settingspage:new(parent, machine)  
  local result = nil
  if not settingspage.hasinstance then
    result = group:new(parent)  
    setmetatable(result, self)
    self.__index = self
    result:init(machine)
	settingspage.hasinstance = true
  end
  return result
  
end

function settingspage:init(machine)  
  self.doapply = signal:new()  
  self.settingsmanager = machine.settingsmanager
  self.activepropertygroup = nil  
  self:inittree()  
  self.propertygroup = self:initpropertygroup(self)
  self:initsettinggroup(self.propertygroup.header)
  self:selectsetting()
  self:initapplygroup(self.propertygroup)      
  self:updatetree()
  self.tree:selectnode(self.rootnode:at(1))
end

function settingspage:initsettinggroup(parent)
  local group = group:new(parent)
                     :setalign(item.ALCLIENT)
					 :setautosize(false, true)
                     :addornament(ornamentfactory:createfill(settings.sci.default.background))
  systems:new():importfont(cfg:luapath() .. "\\plugineditor\\zorque.ttf")
  text:new(group):setalign(item.ALLEFT)
                 :setautosize(true, false)
				 :setverticalalignment(item.ALCENTER)
                 :settext("Settings")
				 :setmargin(boxspace:new(0, 5, 0, 0))				 
  self.settingnamebox = combobox:new(group):setalign(item.ALLEFT)
                                           :setautosize(false, false)
				                           :setposition(rect:new(point:new(), dimension:new(70, 20)))
										   :setmargin(boxspace:new(0, 10, 0, 0))
  self.saveasbutton = button:new(group):setalign(item.ALLEFT)
                            :setautosize(false, false)
				            :setposition(rect:new(point:new(), dimension:new(60, 0)))				 
                            :settext("Save As")
				            :setmargin(boxspace:new(0, 5, 0, 0))  
  self.savenameedit = edit:new(group):setalign(item.ALLEFT)
                          :setautosize(true, false)				 
                          :settext("")
			              :setposition(rect:new(point:new(), dimension:new(60, 0))) 
				          :setmargin(boxspace:new(0, 5, 0, 0))
				          :hide()
  local that = self
  function self.savenameedit:onkeydown(ev)         
    if ev:keycode() == ev.RETURN then	  
	  that:updatesetting(that.rootnode)
      that.settingsmanager:saveas(self:text())
	  that.settingnamebox:clear()
	  that:fillcombobox()
	  that.settingnamebox:setitembytext(self:text())	  	  	  
    end    
	if ev:keycode() == ev.RETURN or 
	   ev:keycode() == ev.ESCAPE then	   
	   that.saveasbutton:show()
	   that.savenameedit:hide()
	   self:parent():updatealign()
	   ev:preventdefault()
	end	
  end  						  
  local that = self
  function self.saveasbutton:onclick()							 
    that.savenameedit:show()
	that.saveasbutton:hide()
	self:parent():flagnotaligned():updatealign()
	that.savenameedit:setfocus()
  end						  
  local removesettingbutton = button:new(group):setalign(item.ALLEFT)
                 :setautosize(false, false)
				 :setposition(rect:new(point:new(), dimension:new(50, 0)))
                 :settext("Remove")
				 :setmargin(boxspace:new(0, 5, 0, 0))
  function removesettingbutton:onclick()
    local name = that.settingnamebox:items()[that.settingnamebox:itemindex()]
    local path = cfg:presetpath().."\\plugineditor\\"..name..".lua"
    filehelper.remove(path)
	that:fillcombobox()
	that:load("default")
	that.settingnamebox:setitembytext("default")
  end	
				 
  function self.settingnamebox:onselect()    
    local filename = self:items()[self:itemindex()]		
	that:load(filename)
  end  
  self:fillcombobox()
  return group
end

function settingspage:load(name)  
  self.propertygroup.client:removeall()
  collectgarbage()  
  self.settingsmanager:load(name)   
  self:updatetree()
  self.tree:selectnode(self.rootnode:at(1))
end

function settingspage:fillcombobox()  
  self.settingnamebox:clear()
  local path = cfg:presetpath().."\\plugineditor"
  local dir = filehelper.directorylist(path)
  for i=1, #dir do
    if dir[i] then	  
	  if not dir[i].isdirectory and dir[i].extension == ".lua" and dir[i].filename ~= "global.lua" then	 
	    local name = ""
        local found, len, remainder = string.find(dir[i].filename, "^(.*)%.[^%.]*$")
	    if found then
		  name = remainder
	    else
		  name = path
	    end				
	    self.settingnamebox:additem(name)  
      end	  	  
	end
  end  
end

function settingspage:selectsetting()  
  self.settingnamebox:setitembytext(self.settingsmanager:settingname())
end

function settingspage:initpropertygroup(parent)
  local propertygroup = group:new(parent)
                             :setalign(item.ALCLIENT)
					         :setautosize(false, false)
                             :addornament(ornamentfactory:createfill(settings.sci.default.background))
  propertygroup.header =  group:new(propertygroup)
                               :setalign(item.ALTOP)
					           :setautosize(false, true)							   
                               :addornament(ornamentfactory:createfill(settings.sci.default.background))
							   :setmargin(boxspace:new(5, 0, 0, 10))
  propertygroup.client =  group:new(propertygroup)
                               :setalign(item.ALCLIENT)
					           :setautosize(false, false)    
                               :addornament(ornamentfactory:createfill(settings.sci.default.background))
  propertygroup.headertext = text:new(propertygroup.header)
                                 :setautosize(true, true)
					             :settext("")					 
					             :setalign(item.ALRIGHT)
					             --:setcolor(0x87CA7E)
								 :setcolor(0xA8444C)
					             :setmargin(boxspace:new(0, 10, 0, 0))  
								 :setfont({name="zorque", height=18})
                                 :setdebugtext("zork")
  return propertygroup					 
end

function settingspage:initapplygroup(parent)
  local group = group:new(parent)
                     :setalign(item.ALBOTTOM)
					 :setautosize(false, true)
                     :setmargin(boxspace:new(0, 0, 10, 0))
  local applybutton = button:new(group)
                            :settext("Apply Changes")
							:setalign(item.ALRIGHT)
						    :setmargin(boxspace:new(0, 10, 0, 0))
							:setposition(rect:new(point:new(), dimension:new(90, 20)))  
  local that = self							
  function applybutton:onclick()
    that:updatesetting(that.rootnode)
	that.doapply:emit(that.settingsmanager:setting())	
	that.settingsmanager:save()
  end
  return group
end

function settingspage:updatesetting(parent)
  for i=1, parent:size() do
	local node = parent:at(i)	
    if node.propertygroup then        
      node.propertygroup:parseproperties()		
	end
    if not node:empty()	then
	   settingspage:updatesetting(node)
	end
  end	
end

function settingspage:inittree()
  self.tree = tree:new(self)
                  :setautosize(false, false)
				  :setalign(item.ALLEFT)
				  :setposition(rect:new(point:new(), dimension:new(200, 0)))
  self.tree:showbuttons():showlines()
  self.tree:setbackgroundcolor(0x2F2F2F)
  self.tree:settextcolor(0xFFFFFF)
  local that = self
  function self.tree:onchange(node)
    that:settab(node)  
  end 
end

function settingspage:settab(node)
  if node.settingpageid ~= nil then
      if not node.propertygroup then
	    node.propertygroup = propertiesview:new(self.propertygroup.client, node.settingpageid, node.settinggroup.properties, self.settingsmanager:setting())
	                                       :setalign(item.ALCLIENT)                               
                                           :setmargin(boxspace:new(10))                               	  
	    node.propertygroup.doapply:connect(settingspage.onapply, self)        
      end
	  node.propertygroup:show()	  
	  if self.activepropertygroup ~= nil then
	    self.activepropertygroup:hide()
	  end	 
	  self.activepropertygroup = node.propertygroup		
	  self.propertygroup.headertext:settext(node.settingpageid)
	  self.propertygroup.header:updatealign()
	  self.propertygroup.client:updatealign()	  
	end
end

function settingspage:updatetree()
  self.rootnode = node:new()
  self:inittreenodes(self.rootnode, self.settingsmanager:setting())  
  self.tree:setrootnode(self.rootnode)  
end

function settingspage:inittreenodes(parent, setting)    
  for name, settinggroup in setting:opairs() do    
    local label = ""
	if settinggroup.label then	  
	  label = settinggroup.label	  
	else
      label = name
    end	
    local node = node:new(parent)
	                 :settext(label)
	node.settingpageid = name
	node.settinggroup = settinggroup
	if settinggroup.children then
	  self:inittreenodes(node, settinggroup.children)
	end
  end  
end

function settingspage:onapply(name, properties)     
end

function settingspage:updatestatus(statusdisplay)  
end

function settingspage:modified()
  return false
end

function settingspage:applysetting(setting)
  self:applypropertiesviews(self.rootnode, setting)  
end

function settingspage:applypropertiesviews(parent, setting)
  for i=1, parent:size() do
	local node = parent:at(i)	
    if node.propertygroup then        
      node.propertygroup:applysetting(setting)
	end
    if not node:empty()	then
	  self:applypropertiesviews(node, setting)
	end
  end	
end

function settingspage:filename()
  return ""
end

function settingspage:onclose()
  settingspage.hasinstance = false
end

return settingspage
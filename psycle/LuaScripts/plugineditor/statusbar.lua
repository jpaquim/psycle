--[[ 
psycle plugineditor (c) 2016 by psycledelics
File: statusbar.lua
copyright 2016 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local signal = require("psycle.signal")
local systems = require("psycle.ui.systems")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local text = require("psycle.ui.text")
local edit = require("psycle.ui.edit")

local statusbar = group:new()

statusbar.windowtype = 52

function statusbar:new(parent, setting)  
  local c = group:new(parent)
                 :setautosize(false, false)  
  setmetatable(c, self)
  self.__index = self  
  c:init(setting)  
  systems:new():changewindowtype(statusbar.windowtype, c)
  return c
end

function statusbar:init(setting)  
  self.gotopage = signal:new()
  self.escape = signal:new()
  self:setalign(item.ALTOP)
      :setautosize(false, false)       
      :setposition(rect:new(point:new(), dimension:new(0, 20)));
  self.status, self.label = {}, {}  
  self.status.insert = self:createtext("ON", 30)
  self.label.insert = self:createtext("INSERT")
  self.status.col = self:createtext("1", 30)
  self.label.col = self:createtext("COL")   
  self.status.line = self:createedit("1", 30)
  self.label.line = self:createtext("LINE")
  self.status.modified = self:createtext("", 100)
  self.status.searchrestart = self:createtext("", 100)  
  self.status.control = self:createedit("", 30, item.ALLEFT)
  function self.status.control:onkeydown(ev)
    ev:preventdefault()
  end
  local that = self
  function self.status.control:onkillfocus()
    that.info:jumpmain()
    that:clearcontrol()    
    that.info:hide():parent():updatealign()
  end
  self:initdefaultcolors()
  self.prefix = ""
end

function statusbar:setinfo(info)
  self.info = info
end

function statusbar:updatestatus(status)
  if not self.status.line:hasfocus() then
    self.status.line:settext(status.line)  
  end
  self.status.col:settext(status.column)     
  self.status.insert:settext(
      statusbar.booltostring(status.ovrtype, "ON", "OFF"))
  self.status.searchrestart:settext(
      statusbar.booltostring(status.searchrestart,"SEARCH AT BEGINNING POINT"))
  self.status.modified:settext(
      statusbar.booltostring(status.modified, "MODIFIED"))
end

function statusbar:setcontrolkey(keycode)
  self.prefix = "^" .. string.char(keycode)
  self.status.control:settext(self.prefix)  
end

function statusbar.booltostring(value, ontext, offtext)
  local result = ""
  if value then
    result = ontext
  elseif offtext then
    result = offtext    
  end
  return result
end

function statusbar:clearcontrol()
  self.status.control:settext("")
  self.prefix = ""
end

function statusbar:onkeydown(ev)  
  if (ev:ctrlkey() and ev:keycode() == 0x45) or ev:keycode() == ev.ESCAPE then    
    self.info:jumpmain()
    self:clearcontrol()
    ev:stoppropagation()
    ev:preventdefault()
    self.info:hide():parent():updatealign()
    self.escape:emit(self)
  else
    self.info:jump(ev:keycode())
    self.status.control:settext(self.prefix .. string.char(ev:keycode()))
    ev:stoppropagation()
    ev:preventdefault()
  end  
end

function statusbar:createedit(label, width, align)
  if not align then
    align = item.ALRIGHT
  end
  local wrapper = group:new(self):setautosize(false, false)
                       :setalign(align)
                       :setmargin(boxspace:new(2, 0, 2, 5))
                       :setpadding(boxspace:new(1))
  local result = edit:new(wrapper)
                     :settext(label)            
                     :setalign(item.ALCLIENT)                     
  local that = self
  function result:onkeydown(ev)         
    if ev:keycode() == ev.RETURN then      
      that.gotopage:emit(self)
      ev:preventdefault()
    elseif ev:keycode() == ev.ESCAPE then    
      that.escape:emit(self)
      ev:preventdefault()
    end
  end
  function wrapper:onmouseenter()                     
    self:setpadding(boxspace:new(0))
    self:addornament(ornamentfactory:createlineborder(0xFFFFFFFF))                     
  end
  function wrapper:onmouseout()                     
    self:removeornaments()
        :setpadding(boxspace:new(1))
  end
  if width then
    result:setautosize(false, false)    
    result:setposition(rect:new(point:new(), dimension:new(width, 15)))
    wrapper:setposition(rect:new(point:new(), dimension:new(width, 0)))
  else    
    result:setautosize(true, false)
  end        
  return result
end

function statusbar:createtext(label, width)
  local result = text:new(self)
                     :settext(label)
                     :setalign(item.ALRIGHT)
                     :setverticalalignment(item.ALCENTER)
                     :setmargin(boxspace:new(0, 0, 0, 5))
  if width then
    result:setautosize(false, false)    
    result:setposition(rect:new(point:new(), dimension:new(width, 0)))
  else    
    result:setautosize(true, false)
  end        
  return result
end

function statusbar:initdefaultcolors()
  self:setcolor(0xFFFFFFFF)
    :setstatuscolor(0xFFFFFFFF)
  -- :setbackgroundcolor(0xFF333333)
  return self
end

function statusbar:setcolor(color) 
  for i = 1, #self.label do
    self.label[i]:setcolor(color)
  end
  return self
end

function statusbar:setstatuscolor(color) 
  for i = 1, #self.status do
    self.status[i]:setcolor(color)
  end    
  return self
end

function statusbar:setbackgroundcolor(color)   
  local backgroundfill = ornamentfactory:createfill(color)
  self:addornament(backgroundfill)
  for i = 1, #self.status do
    self.status[i]:removeornaments():addornament(backgroundfill)
  end
  for i = 1, #self.label do
    self.label[i]:removeornaments():addornament(backgroundfill)
  end  
  return self
end

function statusbar:setproperties(properties)
  local setters = {"color", "statuscolor", "backgroundcolor"}
  for _, setter in pairs(setters) do        
    local property = properties[setter]      
    if property then      
      statusbar["set"..setter](self, property:value())
    end
  end
  self:fls()  
end

return statusbar
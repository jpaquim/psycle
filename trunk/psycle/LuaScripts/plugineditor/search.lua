-- psycle plugineditor (c) 2015, 2016 by psycledelics
-- File: search.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local signal = require("psycle.signal")
local point = require("psycle.ui.point")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local button = require("psycle.ui.button")
local edit = require("psycle.ui.edit")
local text = require("psycle.ui.text")
local checkbox = require("psycle.ui.canvas.checkbox")
local iconbutton = require("psycle.ui.canvas.toolicon")
local closebutton = require("closebutton")
local settings = require("settings")
local search = group:new()

search.DOWN = 1
search.UP = 2                            

search.iconpath = settings.picdir
                            
function search:new(parent, setting)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init(setting)  
  return c
end

function search:init(setting) 
  self:setautosize(false, true)
  self:setpadding(boxspace:new(5))  
  self.dosearch = signal:new()
  self.dohide = signal:new()
  self.doreplace = signal:new()
  local g = group:new(self):setautosize(true, true):setalign(item.ALRIGHT)
  local closebutton = closebutton.new(g):setalign(item.ALTOP)  
  closebutton.dohide:connect(search.onclosebutton, self)
  self:createeditgroup(self)  
  self:createreplacegroup(self)    
  self:applysetting(setting)
end

function search:createeditgroup(parent)       
 self.editgroup = group:new(parent)
                       :setautosize(true, true):setalign(item.ALLEFT) 
 local optionrow = group:new(self.editgroup)
                        :setautosize(true, true)
						:setalign(item.ALTOP)
						:setmargin(boxspace:new(5, 0, 5, 0))
 self:createoptions(optionrow) 
 local editrow = group:new(self.editgroup)
                      :setautosize(true, true)
					  :setalign(item.ALTOP)
					  :setmargin(boxspace:new(0, 0, 5, 0))
 self:createeditfield(editrow):initeditevents()   
 self:createsearchbuttons(editrow)
 return self
end

function search:createeditfield(parent)
  self.edit = edit:new(parent)
                  :setposition(rect:new(point:new(0, 0), dimension:new(200, 20)))
				  :setalign(item.ALLEFT)				  
  return self
end

function search:createsearchbuttons(parent)
  self.up = iconbutton:new(parent, search.iconpath.."up.png", 0xFFFFFF)
                      :setalign(item.ALLEFT)
					  :setmargin(boxspace:new(0, 2, 0, 5))
  self.down = iconbutton:new(parent, search.iconpath.."down.png", 0xFFFFFF)
                        :setalign(item.ALLEFT)
  local that = self
  function self.up:onclick()
    that.dosearch:emit(that.edit:text(), 
                       search.UP,
                       that.casesensitive:check(),
                       that.wholeword:check(),
                       that.useregexp:check())
  end
  function self.down:onclick()
    that.dosearch:emit(that.edit:text(),
                       search.DOWN,
                       that.casesensitive:check(),
                       that.wholeword:check(),
                       that.useregexp:check())
  end 
  return self
end

function search:initeditevents() 
  local that = self
  function self.edit:onkeydown(ev)         
    if ev:keycode() == ev.RETURN then
      local dir = search.DOWN
      if (ev:shiftkey()) then
        dir = search.UP       
      end
      that.dosearch:emit(that.edit:text(), 
                         dir, 
                         that.casesensitive:check(),
                         that.wholeword:check(),
                         that.useregexp:check())
      ev:preventdefault()
    elseif ev:keycode() == ev.ESCAPE then
      that:hide():parent():updatealign()
      that.dohide:emit()
    end
  end  
  return self
end

function search:createreplacegroup(parent)
  self.replacegroup = group:new(parent)
                           :setalign(item.ALLEFT)
						   :setautosize(true, true)
						   :setmargin(boxspace:new(0, 0, 0, 20))
						   :setpadding(boxspace:new(0, 5, 0, 5))
  self:createreplacefield(self.replacegroup)  
end

function search:createreplacefield(parent)  
  self.replaceactive = checkbox:new(parent)
                               :settext("replace selection with")
							   :setalign(item.ALTOP)
							   :setmargin(boxspace:new(5, 0, 5, 0))
  self.replacefieldgroup = group:new(parent)
                                :setalign(item.ALTOP)
								:setautosize(true, true)
								:setmargin(boxspace:new(0, 0, 5, 0))
  self.replacefield = edit:new(self.replacefieldgroup)
                          :disable()
                          :setalign(item.ALLEFT)
						  :setautosize(false, false)						  
						  :setcolor(0xA0A0A0)
						  :setbackgroundcolor(0x323232)						  
  local that = self
  function self.replacefield:onkeydown(ev)         
    if ev:keycode() == ev.RETURN then      
      that.doreplace:emit()
      ev:preventdefault()
    elseif ev:keycode() == ev.ESCAPE then
      that:hide():parent():updatealign()
      that.dohide:emit()
    end
  end
  self:createreplacebutton(self.replacefieldgroup)  
  function self.replaceactive:onclick(ischecked)
     if self:check() then
       that.replacefield:enable()
       that.replacebtn:enable()
	   that.replacefield:setbackgroundcolor(0x373533)       
	   that.replacefield:setcolor(0xFFFFFF)
	   that.replacegroup:addornament(ornamentfactory:createlineborder(0x525252))
     else
       that.replacefield:disable()
       that.replacebtn:disable()
       that.replacegroup:removeornaments()	   
	   that.replacefield:setbackgroundcolor(0x323232)	   
	   that.replacefield:setcolor(0xA0A0A0)
	   that.replacefield:fls()
     end
  end
  return self
end

function search:createreplacebutton(parent)
  self.replacebtn = button:new(parent)
                          :settext("replace")
						  :setalign(item.ALLEFT)
                          :setmargin(boxspace:new(0, 0, 0, 2))
                          :setposition(rect:new(point:new(0, 0), dimension:new(60, 20)))
                          :disable()
  local that = self
  function self.replacebtn:onclick()
    that.doreplace:emit()
  end  
  return self
end


function search:createoptions(parent)
  self.casesensitive = checkbox:new(parent)
                               :setalign(item.ALLEFT)
							   :settext("match case")
							   :setmargin(boxspace:new(0, 5, 0, 0))							   
  self.wholeword = checkbox:new(parent)
                           :setalign(item.ALLEFT)
						   :settext("match whole words only")
						   :setmargin(boxspace:new(0, 5, 0, 0))						   
  self.useregexp = checkbox:new(parent)
                           :setalign(item.ALLEFT)
						   :settext("use regexp")
						   :setmargin(boxspace:new(0, 5, 0, 0))						   
  return self
end

function search:onfocus()  
  self.edit:setfocus()
end

function search:onclosebutton()  
  self.dohide:emit()
end

function search:applysetting(setting)  
  self:addornament(ornamentfactory:createlineborder(0x253E2F))
  local items = {    
    self.casesensitive,
	self.wholeword,
	self.useregexp,
	self.replaceactive    
  }
  for i=1, #items do
    items[i]:setcolor(setting.general.properties.foregroundcolor:value())
  end
  
  local items = {    
    self.edit	
  }
  for i=1, #items do
    local item = items[i]
    item:setcolor(setting.general.children.ui.properties.editforegroundcolor:value())
	    :setbackgroundcolor(setting.general.children.ui.properties.editbackgroundcolor:value())
  end  
end

return search
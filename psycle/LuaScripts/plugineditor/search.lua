-- psycle plugineditor (c) 2015 by psycledelics
-- File: search.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local group = require("psycle.ui.canvas.group")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local style = require("psycle.ui.canvas.itemstyle")
local edit = require("psycle.ui.canvas.edit")
local iconbutton = require("psycle.ui.canvas.toolicon")
local checkbox = require("psycle.ui.canvas.checkbox")
local signal = require("psycle.signal")
local settings = require("settings")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()


local search = group:new()

local rowstyle = style:new():setalign(style.ALLEFT)
                            --:setmargin(0, 0, 2, 0)
local colstyle = style:new():setalign(style.ALTOP)
                            --:setmargin(0, 0, 0, 2)  

search.DOWN = 1
search.UP = 2                            
                            
function search:new(parent)
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init()  
  return c
end

function search:init() 
  self:setautosize(false, true)    
  --self:style():setmargin(3, 3, 3, 3):setpadding(10, 5, 10, 5)
  self.dosearch = signal:new()   
  self:createeditgroup(self)  
--  self:createreplacegroup(self)
  self:createclosebutton()
  --self:setornament(ornamentfactory:createlineborder(0x528A68))  
 --self.border = ornamentfactory:createlineborder(0x528A68)
  --self.border:setborderradius(10, 0, 0, 10)
 --self:setornament(self.border)
 self:setornament(ornamentfactory:createfill(0x528A68))   
end

function search:createeditgroup(parent)      
 self.editgroup = group:new(parent):setautosize(true, true)
 self.editgroup:style():setalign(style.ALLEFT)
 local optionrow = group:new(self.editgroup):setautosize(true, true)
 optionrow:style():setalign(style.ALTOP)--:setmargin(0, 0, 0, 5)
 self:createoptions(optionrow) 
 local editrow = group:new(self.editgroup):setautosize(true, true)
 editrow:style():setalign(style.ALTOP)--:setmargin(0, 0, 0, 0)
 self:createeditfield(editrow):initeditevents()  
 self:createsearchbuttons(editrow)
 return self
end

function search:createeditfield(parent)
  self.edit = edit:new(parent):setpos(0, 0, 200, 20)
  self.edit:style():setalign(style.ALLEFT)
  return self
end

function search:createsearchbuttons(parent)
  self.up = iconbutton:new(parent, settings.picdir.."up.png", 0xFFFFFF)
  self.up:style():setalign(style.ALLEFT)--:setmargin(2, 0, 2, 0)
  self.down = iconbutton:new(parent, settings.picdir.."down.png", 0xFFFFFF)
  self.down:style():setalign(style.ALLEFT)--:setmargin(0, 0, 0, 0)
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
    if ev:keycode() == 13 then
      that.dosearch:emit(self:text(), 
                         search.DOWN, 
                         that.casesensitive:check(),
                         that.wholeword:check(),
                         that.useregexp:check())
    end
  end  
  return self
end

function search:createreplacegroup(parent)
  self.replacegroup = group:new(parent)
  self.replacegroup:style():setalign(style.ALLEFT)
  self:createreplacefield(self.replacegroup)  
end

function search:createreplacefield(parent)
  self.replaceactive = checkbox:new(parent):settext("replace with")
  self.replaceactive:style():setalign(style.ALTOP)--:setmargin(0, 0, 2, 4)
  self.replacefield = edit:new(parent)
  self.replacefield:style():setalign(style.ALTOP)
  return self
end

function search:createoptions(parent)
  self.casesensitive = checkbox:new(parent):settext("match case")
  self.casesensitive:style():setalign(style.ALLEFT)--:setmargin(0, 0, 0, 0)
  self.wholeword = checkbox:new(parent):settext("match whole words only")
  self.wholeword:style():setalign(style.ALLEFT)--:setmargin(5, 0, 0, 0)
  self.useregexp = checkbox:new(parent):settext("use regexp")
  self.useregexp:style():setalign(style.ALLEFT)--:setmargin(5, 0, 0, 0)
  return self
end

function search:createclosebutton()
  local g = group:new(self):setautosize(true, true)
  g:style():setalign(style.ALRIGHT)--:setmargin(2, 2, 2, 2)
  text:new(g):settext("X"):setcolor(0xFFFFFF):style() --:setalign(style.ALRIGHT)  
  local that = self
  function g:onmousedown()    
    that:hide()
    that:canvas():align()
  end
  return self
end

function search:onfocus()  
  self:canvas():setfocus(self.edit)
end

return search
-- psycle plugineditor (c) 2015 by psycledelics
-- File: search.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local item = require("psycle.ui.canvas.item")
local text = require("psycle.ui.canvas.text")
local edit = require("psycle.ui.canvas.edit")
local iconbutton = require("psycle.ui.canvas.toolicon")
local checkbox = require("psycle.ui.canvas.checkbox")
local signal = require("psycle.signal")
local settings = require("settings")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local closebutton = require("closebutton")

local search = group:new()

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
  self.dosearch = signal:new()
  self.dohide = signal:new()
  local closebtn = closebutton.new(self)
  closebtn.dohide:connect(search.onclosebutton, self)
  self:createeditgroup(self)  
--self:createreplacegroup(self)  
  self:setornament(ornamentfactory:createlineborder(0x253E2F))  
end

function search:createeditgroup(parent)      
 self.editgroup = group:new(parent):setautosize(true, true):setalign(item.ALLEFT)
 local optionrow = group:new(self.editgroup):setautosize(true, true):setalign(item.ALTOP)-- :setmargin(0, 0, 0, 5)
 self:createoptions(optionrow) 
 local editrow = group:new(self.editgroup):setautosize(true, true):setalign(item.ALTOP)
 self:createeditfield(editrow):initeditevents()  
 self:createsearchbuttons(editrow)
 return self
end

function search:createeditfield(parent)
  self.edit = edit:new(parent):setpos(0, 0, 200, 20)
  self.edit:setdebugtext("search"):setalign(item.ALLEFT)
  return self
end

function search:createsearchbuttons(parent)
  self.up = iconbutton:new(parent, settings.picdir.."up.png", 0xFFFFFF):setalign(item.ALLEFT)
  self.down = iconbutton:new(parent, settings.picdir.."down.png", 0xFFFFFF):setalign(item.ALLEFT)
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
  self.replacegroup = group:new(parent):setalign(item.ALLEFT)
  self:createreplacefield(self.replacegroup)  
end

function search:createreplacefield(parent)
  self.replaceactive = checkbox:new(parent):settext("replace with"):setalign(item.ALTOP)
  self.replacefield = edit:new(parent):setalign(item.ALTOP)
  return self
end

function search:createoptions(parent)
  self.casesensitive = checkbox:new(parent)
  self.casesensitive.published.settext(self.casesensitive, "match case")
  self.casesensitive:setalign(item.ALLEFT)
  self.wholeword = checkbox:new(parent):settext("match whole words only"):setalign(item.ALLEFT)
  self.useregexp = checkbox:new(parent):settext("use regexp"):setalign(item.ALLEFT)
  return self
end

function search:onfocus()  
  self.edit:setfocus()
end

function search:onclosebutton()
  self.dohide:emit()
end

return search
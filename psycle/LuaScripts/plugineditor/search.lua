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
local button = require("psycle.ui.canvas.button")
local checkbox = require("psycle.ui.canvas.checkbox")
local signal = require("psycle.signal")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local closebutton = require("closebutton")

local search = group:new()

search.DOWN = 1
search.UP = 2                            

search.iconpath = "C:\\Users\\User\\Documents\\Visual Studio 2015\\Projects\\exforward\\exforward\\LuaScripts\\psycle\\ui\\icons\\"
                            
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
  self.doreplace = signal:new()
  local closebtn = closebutton.new(self)
  closebtn.dohide:connect(search.onclosebutton, self)
  self:createeditgroup(self)  
  self:createreplacegroup(self)  
  self:addornament(ornamentfactory:createlineborder(0x253E2F))
end

function search:createeditgroup(parent)       
 self.editgroup = group:new(parent):setautosize(true, true):setalign(item.ALLEFT) 
 local optionrow = group:new(self.editgroup):setautosize(true, true):setalign(item.ALTOP):setmargin(5, 0, 5, 0)                        
 self:createoptions(optionrow) 
 local editrow = group:new(self.editgroup):setautosize(true, true):setalign(item.ALTOP):setmargin(0, 0, 5, 0)
 self:createeditfield(editrow):initeditevents()   
 self:createsearchbuttons(editrow)
 return self
end

function search:createeditfield(parent)
  self.edit = edit:new(parent):setpos(0, 0, 200, 20):setalign(item.ALLEFT)
  return self
end

function search:createsearchbuttons(parent)
  self.up = iconbutton:new(parent, search.iconpath.."up.png", 0xFFFFFF):setalign(item.ALLEFT):setmargin(0, 2, 0, 5)
  self.down = iconbutton:new(parent, search.iconpath.."down.png", 0xFFFFFF):setalign(item.ALLEFT)
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
  self.replacegroup = group:new(parent):setalign(item.ALLEFT):setautosize(true, true):setmargin(0, 0, 0, 20)
  self:createreplacefield(self.replacegroup)  
end

function search:createreplacefield(parent)  
  self.replaceactive = checkbox:new(parent):settext("replace selection with"):setalign(item.ALTOP):setmargin(5, 0, 5, 0)  
  self.replacefieldgroup = group:new(parent):setalign(item.ALTOP):setautosize(true, true):setmargin(0, 0, 5, 0)
  self.replacefield = edit:new(self.replacefieldgroup):setalign(item.ALLEFT):setautosize(false, false):disable()  
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
       that.replacegroup:addornament(ornamentfactory:createfill(0xFFBF00))
     else
       that.replacefield:disable()
       that.replacebtn:disable()
       that.replacegroup:removeornaments()
     end
  end
  return self
end

function search:createreplacebutton(parent)
  self.replacebtn = button:new(parent):settext("replace"):setalign(item.ALLEFT)
  --:setmargin(5, 0, 2, 0)
  :setpos(0, 0, 60, 20):disable()
  local that = self
  function self.replacebtn:onclick()
    that.doreplace:emit()
  end  
  return self
end


function search:createoptions(parent)
  self.casesensitive = checkbox:new(parent):setalign(item.ALLEFT):settext("match case  1223344 genau")
  self.wholeword = checkbox:new(parent):setalign(item.ALLEFT):settext("match whole words only")
  self.useregexp = checkbox:new(parent):setalign(item.ALLEFT):settext("use regexp")
  return self
end

function search:onfocus()  
  self.edit:setfocus()
end

function search:onclosebutton()
  self.dohide:emit()
end

return search
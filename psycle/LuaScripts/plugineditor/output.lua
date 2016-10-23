-- psycle pluginselector (c) 2016 by psycledelics
-- File: pluginselector.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local cfg = require("psycle.config"):new("PatternVisual")
local node = require("psycle.node")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local lexer = require("psycle.ui.lexer")
local scintilla = require("psycle.ui.scintilla")
local settings = require("settings")
local search = require("search")

local output = scintilla:new()

function output:new(parent, setting)    
  local c = scintilla:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init(setting)
  return c
end

function output:init(setting)
  self:setautosize(false, false)
  self:applysetting(setting)  
end

function output:applysetting(setting)  
  self:setforegroundcolor(setting.general.properties.foregroundcolor:value())
  self:setbackgroundcolor(setting.general.properties.backgroundcolor:value())   
  self:styleclearall()
  self:setlinenumberforegroundcolor(setting.general.properties.linenumberforegroundcolor:value())
  self:setlinenumberbackgroundcolor(setting.general.properties.linenumberbackgroundcolor:value())  
  self:setmarginbackgroundcolor(setting.general.properties.marginbackgroundcolor:value()) 
  self:setselbackgroundcolor(setting.general.properties.selbackgroundcolor:value())    
  self:setselalpha(75)
  self:setfont(settings.sci.lexer.font.name, settings.sci.lexer.font.size)  
  self:setcaretcolor(0x939393)
  self:setcaretlinebackgroundcolor(0x373533)
  self:showcaretline()
end

function output:onkeydown(ev)
  if ev:ctrlkey() then
    if ev:keycode() == 70 or ev:keycode() == 83 then
      ev:preventdefault()
    end            
  end
end    

function output:findprevsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = 0
  if self:hasselection() then 
    cpmax = cpmax - 1
  end
  return cpmin, cpmax
end

function output:findnextsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = self:length()
  if self:hasselection() then       
    cpmin = cpmin + 1
  end
  return cpmin, cpmax
end

function output:findsearch(page, dir, pos)
  local cpmin, cpmax = 0, 0
  if dir == search.DOWN then      
    cpmin, cpmax = self:findnextsearch(self, pos)
  else
    cpmin, cpmax = self:findprevsearch(self, pos)
  end
  return cpmin, cpmax
end

function output:onsearch(searchtext, dir, case, wholeword, regexp)    
  self:setfindmatchcase(case):setfindwholeword(wholeword):setfindregexp(regexp)      
  local cpmin, cpmax = self:findsearch(self, dir, self:selectionstart())
  local line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)
  if line == -1 then      
    if dir == search.DOWN then
	  self:setsel(0, 0)
	  local cpmin, cpmax = self:findsearch(self, dir, 0)
	  line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)        
	else
	  self:setsel(0, 0)
	  local cpmin, cpmax = self:findsearch(self, dir, self:length())
	  line, cpselstart, cpselend = self:findtext(searchtext, cpmin, cpmax)
	end             
  end
  if line ~= -1 then
	self:setsel(cpselstart, cpselend)
	if self.searchbeginpos == cpselstart then
	  self.searchbegin = -1        
	  self.searchrestart = true
	else
	  self.searchrestart = false
	end
	if self.searchbeginpos == -1 then
	  self.searchbeginpos = cpselstart        
	end
  end      
end

return output
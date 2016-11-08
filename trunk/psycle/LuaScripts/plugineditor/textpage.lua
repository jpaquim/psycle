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
local fontinfo = require("psycle.ui.fontinfo")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local lexer = require("psycle.ui.lexer")
local scintilla = require("psycle.ui.scintilla")
local settings = require("settings")
local search = require("search")
local serpent = require("psycle.serpent")

local textpage = scintilla:new()

textpage.pagecounter = 0

function textpage:new(parent, setting)    
  local c = scintilla:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init(setting)
  textpage.pagecounter = textpage.pagecounter + 1
  return c
end

function textpage:init(setting)
  self:setautosize(false, false)
  self:applysetting(setting)  
end

function textpage:applysetting(setting)  
  self:setforegroundcolor(setting.general.properties.foregroundcolor:value())
  self:setbackgroundcolor(setting.general.properties.backgroundcolor:value())   
  self:styleclearall()
  self:setlinenumberforegroundcolor(setting.general.properties.linenumberforegroundcolor:value())
  self:setlinenumberbackgroundcolor(setting.general.properties.linenumberbackgroundcolor:value())    
  self:setfoldingbackgroundcolor(setting.general.properties.foldingbackgroundcolor:value())
  self:setselbackgroundcolor(setting.general.properties.selbackgroundcolor:value())    
  self:setselalpha(75)
  local lex = lexer:new()  
  lex:setkeywords(setting.lualexer.properties.keywords:value())  
  lex:setcommentcolor(setting.lualexer.properties.commentcolor:value())
  lex:setcommentlinecolor(setting.lualexer.properties.commentlinecolor:value())
  lex:setcommentdoccolor(setting.lualexer.properties.commentdoccolor:value())
  lex:setidentifiercolor(setting.lualexer.properties.identifiercolor:value())
  lex:setnumbercolor(setting.lualexer.properties.numbercolor:value())
  lex:setfoldingmarkerforecolor(0xFF000000) -- setting.lualexer.properties.foldingmarkerforecolor:value())
  lex:setfoldingmarkerbackcolor(0xFF000000) -- setting.lualexer.properties.foldingmarkerbackcolor:value())
  lex:setoperatorcolor(setting.lualexer.properties.operatorcolor:value())
  lex:setstringcolor(setting.lualexer.properties.stringcolor:value())
  lex:setwordcolor(setting.lualexer.properties.wordcolor:value())   
  self:setlexer(lex)  
  self:setfontinfo(setting.general.properties.font:value()) --fontinfo:new():setsize(settings.sci.lexer.font.size):setfamily(settings.sci.lexer.font.name))
  self:setcaretcolor(setting.general.properties.caretcolor:value())
  self:setcaretlinebackgroundcolor(setting.general.properties.caretlinebackgroundcolor:value())  
  self:showcaretline()
end

function textpage:onkeydown(ev)
  if ev:ctrlkey() then
    if ev:keycode() == 70 or ev:keycode() == 83 then
      ev:preventdefault()
    end            
  end
end    

function textpage:onmarginclick(linepos)
  self:addbreakpoint(linepos)
end

function textpage:addbreakpoint(linepos)
  self:definemarker(1, 31, 0x0000FF, 0x0000FF)
  self:addmarker(linepos, 1)
end

function textpage:updatestatus(statusdisplay)
  local that = self
  local status = {
    line = that:line() + 1,
    column = that:column() + 1,
	modified = that:modified(),
	ovrtype = that:ovrtype()
  }   
  statusdisplay:onupdatetextpagestatus(status)
end

function textpage:createdefaultname()
  return "new"..textpage.pagecounter  
end

function textpage:findprevsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = 0
  if self:hasselection() then 
    cpmax = cpmax - 1
  end
  return cpmin, cpmax
end

function textpage:findnextsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = self:length()
  if self:hasselection() then       
    cpmin = cpmin + 1
  end
  return cpmin, cpmax
end

function textpage:findsearch(page, dir, pos)
  local cpmin, cpmax = 0, 0
  if dir == search.DOWN then      
    cpmin, cpmax = self:findnextsearch(self, pos)
  else
    cpmin, cpmax = self:findprevsearch(self, pos)
  end
  return cpmin, cpmax
end

function textpage:onsearch(searchtext, dir, case, wholeword, regexp)    
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

return textpage
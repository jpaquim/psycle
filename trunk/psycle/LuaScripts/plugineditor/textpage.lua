--[[ 
psycle pluginselector (c) 2016 by psycledelics
File: pluginselector.lua
copyright 2016 members of the psycle project http://psycle.sourceforge.net
This source is free software ; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation ; either version 2, or (at your option) any later version.
]]

local systems = require("psycle.ui.systems")
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
local serpent = require("psycle.serpent")
local sci = require("scintilladef")
local textpage = scintilla:new()

textpage.windowtype = 95
textpage.pagecounter = 0
textpage.preventedkeys = {0x41, 0x46, 0x4A, 0x4B, 0x44, 0x45, 0x51, 0x56, 0x45, 78, 79, 87, 70, 83, 87}
textpage.indicator = 14

textpage.EDITMODE = 0
textpage.INFOMODE = 1

function textpage:new(parent)    
  local c = scintilla:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  textpage.pagecounter = textpage.pagecounter + 1
  systems:new():changewindowtype(textpage.windowtype, c)
  return c
end

function textpage:init()
  self:setautosize(false, false)
  self.mode = textpage.EDITMODE
  self:initblockmodus()  
end

function textpage:onkeydown(ev)  
  if ev:ctrlkey() then     
    self:preventkeys(ev)    
  end        
end

function textpage:preventkeys(ev)
  for _, key in pairs(textpage.preventedkeys) do
    if ev:keycode() == key then
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

function textpage:status()  
  local that = self
  return {
    line = that:line() + 1,
    column = that:column() + 1,
	  modified = that:modified(),
	  overtype = that:overtype()
  }  
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

function textpage:setproperties(properties)  
  if properties.mapcapslocktoctrl then
    if  properties.mapcapslocktoctrl:value() then
      self:mapcapslocktoctrl()
    else
      self:enablecapslockt()
    end
  end
  if properties.color then
    self:setforegroundcolor(properties.color:value())	    
  end
  if properties.backgroundcolor then    
	  self:setbackgroundcolor(properties.backgroundcolor:value())
  end
  self:styleclearall()
  self:showcaretline()    
  local lexsetters = {"commentcolor", "commentlinecolor", "commentdoccolor",
                      "foldingmarkerforecolor", "foldingmarkerbackcolor",
                      "operatorcolor", "wordcolor", "stringcolor",
                      "identifiercolor", "numbercolor"}  
  local lex = lexer:new()  
  for _, setter in pairs(lexsetters) do        
    local property = properties[setter]      
    if property then            
      lex["set"..setter](lex, property:value())
    end
  end 
  self:setlexer(lex)  
  local setters = {"linenumberforegroundcolor", "linenumberbackgroundcolor", "foldingbackgroundcolor",
                   "selbackgroundcolor",
                   "caretcolor", "caretlinebackgroundcolor"} 
  for _, setter in pairs(setters) do        
    local property = properties[setter]      
    if property then            
      textpage["set"..setter](self, property:value())
    end
  end    
  self:setselalpha(75)    
  self:showcaretline()  
end

function textpage:setblockbegin()
  self.blockbegin = self:selectionstart()
  self:updateblock()
end

function textpage:setblockend()
  self.blockend = self:selectionstart()
  self:updateblock()
end

function textpage:deleteblock()
  if self.blockbegin ~=-1 and self.blockend ~= -1 then
    self:clearblockselection()
    self:f(sci.SCI_DELETERANGE, self.blockbegin,  self.blockend - self.blockbegin)
    self.blockbegin, self.blockend = -1, -1
  end
end

function textpage:updateblock()
  if self.blockbegin ~=-1 and self.blockend ~= -1 then
     self:clearblockselection()
     self:f(sci.SCI_SETINDICATORCURRENT, textpage.indicator, 0)
     self:f(sci.SCI_INDICATORFILLRANGE, self.blockbegin, self.blockend - self.blockbegin)      
  end
end

function textpage:clearblockselection()
  self:f(sci.SCI_SETINDICATORCURRENT, textpage.indicator, 0)
  self:f(sci.SCI_INDICATORCLEARRANGE, textpage.indicator, self:length())
end

function textpage:initblockmodus()
  self.blockbegin, self.blockend = -1, -1  
  self:setupindicators()
end

function textpage:setupindicators()
   self:f(sci.SCI_INDICSETSTYLE, 8, sci.INDIC_PLAIN)
   self:f(sci.SCI_INDICSETSTYLE, 9, sci.INDIC_SQUIGGLE)
   self:f(sci.SCI_INDICSETSTYLE, 10, sci.INDIC_TT)
   self:f(sci.SCI_INDICSETSTYLE, 11, sci.INDIC_DIAGONAL)
   self:f(sci.SCI_INDICSETSTYLE, 12, sci.INDIC_STRIKE)
   self:f(sci.SCI_INDICSETSTYLE, 13, sci.INDIC_BOX)
   self:f(sci.SCI_INDICSETSTYLE, 14, sci.INDIC_ROUNDBOX)
end

function textpage:oncmd(cmd)
  if cmd == "setblockbegin" then      
    self:setblockbegin()    
  elseif cmd == "setblockend" then    
    self:setblockend()      
  elseif cmd == "deleteblock" then
    self:deleteblock()
  elseif cmd == "deleteblock" then
  
  elseif cmd == "deleteblock" then
  end
end

return textpage

-- psycle knobex (c) 2015 by psycledelics
-- File: knobex.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local group = require("psycle.ui.canvas.group")
local text = require("psycle.ui.canvas.text")
local rect = require("psycle.ui.canvas.rect")
local knob = require("psycle.ui.canvas.knob")
local config = require("psycle.config")

local knobex = group:new()

knobex.DIALHEIGHT2 = knob.DIALWIDTH/2

knobex.cfgkeys = {
  "FontTop",
  "FontBottom",
  "HFontTop",
  "HFontBottom",
  "TitleFont",
  "Top",
  "Bottom",
  "HTop",
  "HBottom",
  "Title"         
}

knobex.skin = {color={}}  

local cfg = config:new("MacParamVisual")
for i=1, #knobex.cfgkeys do
  local keyname = knobex.cfgkeys[i]
  knobex.skin.color[string.lower(keyname)] = cfg:get("machineGUI"..keyname.."Color")
end

function knobex:new(parent, param)  
  local c = group:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init(param)
  return c
end

function knobex:init(param)  
  self.cfg = config:new()    
  self.knob = knob:new(self, param)
  self.rctop = rect:new(self)
  self.rcbottom = rect:new(self)  
  self.txttop = text:new(self):setpos(knob.DIALWIDTH, 0)
  self.txtbottom = text:new(self):setpos(knob.DIALWIDTH, knob.DIALHEIGHT/2)
  self:setwidthex(50)
  self:setnormalcolors()
  if param then    
    self.txttop:settext(param:name())
    self.txtbottom:settext(param:display())    
    param:addlistener(self)
  end
end

function knobex:setwidthex(widthex)
  self.widthex = widthex
  self.rctop:setpos(knob.DIALWIDTH, 0, self.widthex, knob.DIALHEIGHT/2)
  self.rcbottom:setpos(knob.DIALWIDTH, knob.DIALHEIGHT/2, self.widthex, knob.DIALHEIGHT/2-1)
  return self
end

function knobex:setnormalcolors()
  self.rctop:setcolor(knobex.skin.color.top)
  self.rcbottom:setcolor(knobex.skin.color.bottom)
  self.txttop:setcolor(knobex.skin.color.fonttop)
  self.txtbottom:setcolor(knobex.skin.color.fontbottom)
  return self
end

function knobex:sethighlightcolors()
  self.rctop:setcolor(knobex.skin.color.htop)
  self.rcbottom:setcolor(knobex.skin.color.hbottom)
  self.txttop:setcolor(knobex.skin.color.hfonttop)
  self.txtbottom:setcolor(knobex.skin.color.hfontbottom)
  return self
end

function knobex:onstarttweak(param)  
  self:sethighlightcolors()  
end

function knobex:ontweaked(param)  
  self.txtbottom:settext(param:display())
end

function knobex:onaftertweaked(param)  
  self:setnormalcolors()
end


return knobex
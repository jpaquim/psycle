-- File: gamecontrollertest.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version. 

local boxspace = require("psycle.ui.boxspace")
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local canvas = require("psycle.ui.canvas")
local text = require("psycle.ui.text")
local gamecontrollers = require("psycle.ui.gamecontrollers")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()

local maincanvas = canvas:new()

function maincanvas:new()
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  return c
end

function maincanvas:init() 
  self:addornament(ornamentfactory:createfill(0x000000))   
  self.gamecontrollers = gamecontrollers:new()
  local controllers = self.gamecontrollers:controllers()  
  for i=1, #controllers do    
    self:initcontrollerdisplay(i, controllers[i])
  end    
end

function maincanvas:initcontrollerdisplay(no, controller)  
  local g = group:new(self)
                        :setautosize(false, true)
                        :setalign(item.ALTOP)
                        :setmargin(boxspace:new(5, 5, 0, 0))
  self:addtext(g, "Game Controller "..no)  
  self:addtext(g, "x-pos"):addfield(g, "0", "xpos")
  self:addtext(g, "y-pos"):addfield(g, "0", "ypos")
  self:addtext(g, "z-pos"):addfield(g, "0",  "zpos")
  self:addtext(g, "button press down"):addfield(g, "0", "buttondown")
  self:addtext(g, "button press up"):addfield(g, "0", "buttonup")    
  function controller:onbuttondown(button)    
    g.buttondown:settext(button)  
  end
  function controller:onbuttonup(button)    
    g.buttonup:settext(button)
  end
  function controller:onxaxis(pos, oldpos)
    g.xpos:settext(pos)
  end
  function controller:onzaxis(pos, oldpos)
    g.ypos:settext(pos)
  end
  function controller:onyaxis(pos, oldpos)    
    g.zpos:settext(pos)
  end  
end

function maincanvas:addtext(parent, str, field)
  local text = text:new(parent)
                         :settext(str)
                         :setcolor(0x528A68)
                         :setalign(item.ALTOP) 
                         :setautosize(false, true) 
  return self
end

function maincanvas:addfield(parent, str, field)
  local text = text:new(parent)
                         :settext(str)
                         :setcolor(0xFFFFFF)
                         :setalign(item.ALTOP)
                         :setautosize(false, true) 
  parent[field] = text 
  return self
end

return maincanvas
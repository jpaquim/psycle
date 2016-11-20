-- File: gamecontrollertest.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version. 

local rect = require("psycle.ui.rect")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local boxspace = require("psycle.ui.boxspace")
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local canvas = require("psycle.ui.canvas")
local text = require("psycle.ui.text")
local gamecontrollers = require("psycle.ui.gamecontrollers")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()

local cfg = require("psycle.config"):new()

local maincanvas = canvas:new()

function maincanvas:new(machine)  
  local c = canvas:new()    
  setmetatable(c, self)  
  self.__index = self   
  c:init(machine)  
  return c
end

function maincanvas:init(machine) 
  self.machine_ = machine
  self:addornament(ornamentfactory:createfill(0xFF000000))   
  self.gamecontrollers = gamecontrollers:new()
  local controllers = self.gamecontrollers:controllers()  
  for i=1, #controllers do    
    self:initcontrollerdisplay(i, controllers[i])
  end    
  self.stickposx, self.stickposy, self.stickposz = 50, 50, 0     
  self:createstick(self)
end

function maincanvas:svgtest()  
  local path = cfg:luapath().."\\gamecontrollertest\\"..'note.svg'
  self.svg = svg:new(path)  
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
  local that = self
  function controller:onbuttondown(button)    
    g.buttondown:settext(button)      
  end
  function controller:onbuttonup(button)    
    g.buttonup:settext(button)
  end
  function controller:onxaxis(pos, oldpos)
    that:calcstickposx(pos)
    that.stick:fls()  
    g.xpos:settext(pos)
  end
  function controller:onzaxis(pos, oldpos)
   that:calcstickposz(pos)
   that.stick:fls()
   g.ypos:settext(pos)
  end
  function controller:onyaxis(pos, oldpos)    
   that:calcstickposy(pos)
   that.stick:fls()
   g.zpos:settext(pos)
  end  
end

function maincanvas:calcstickposx(pos)
  self.stickposx = self.stick:dimension():width() * pos/65335  
end

function maincanvas:calcstickposy(pos)
  self.stickposy = self.stick:dimension():height() * pos/65335
end

function maincanvas:calcstickposz(pos)
  self.stickposz = self.stick:dimension():height() * pos/65335
end

function maincanvas:addtext(parent, str, field)
  local text = text:new(parent)
                   :settext(str)
                   :setcolor(0xFF528A68)
                   :setalign(item.ALTOP) 
                   :setautosize(false, true) 
  return self
end

function maincanvas:addfield(parent, str, field)
  local text = text:new(parent)
                   :settext(str)
                   :setcolor(0xFFFFFFFF)
                   :setalign(item.ALTOP)
                   :setautosize(false, true) 
  parent[field] = text 
  return self
end

function maincanvas:createstick(parent)
  self.stick = item:new(parent)
                   :setautosize(false, false)
                   :viewdoublebuffered()
                   :addornament(ornamentfactory:createfill(0xFF000000))   
  self.stick:setposition(rect:new(point:new(100, 100), dimension:new(200, 200)))    
  local that = self
  function self.stick:draw(g)
    local dim = self:dimension()
    g:setcolor(0xFFFF0000)
    g:fillcircle(point:new(that.stickposx, that.stickposy), 10)
    g:setcolor(0xFF00FF00)
    g:drawcircle(point:new(that.stickposx, that.stickposy), 10)
    g:drawrect(rect:new(point:new(0, 0), dimension:new(dim:width()-1, dim:height()-1)))    
  end
end

return maincanvas
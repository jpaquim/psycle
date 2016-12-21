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
local controllerbox = require("controllerbox")

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
  controller.stick = self:createstick(g)
                         :setautosize(false, true)
                         :setalign(item.ALTOP)
  local that = self
  function controller:onbuttondown(button)        
    g.buttondown:settext(button)      
  end
  function controller:onbuttonup(button)    
    g.buttonup:settext(button)
  end
  function controller:onxaxis(pos, oldpos)
    self.stick:calcstickposx(pos)
    --g.xpos:settext(pos)
  end
  function controller:onzaxis(pos, oldpos)
   self.stick:calcstickposz(pos):fls()
    --g.ypos:settext(pos)
  end
  function controller:onyaxis(pos, oldpos)    
   self.stick:calcstickposy(pos):fls()
    --g.zpos:settext(pos)
  end  
  return self.stick
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
  --[[self.stick = item:new(parent)
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
  end]]


    self.stick2 = controllerbox:new(parent)
                                         :setautosize(false, false)
                                         :setalign(item.ALCLIENT)
                                        -- :setposition(rect:new(point:new(100, 100), dimension:new(200, 200)))    
                                        -- :viewdoublebuffered()                                     
   --self.stick2:setposition(rect:new(point:new(0, 0), dimension:new(500, 610))) 
  return self.stick2
end

return maincanvas
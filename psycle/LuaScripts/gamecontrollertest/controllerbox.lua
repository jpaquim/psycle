-- controllerbox.lua

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local group = require("psycle.ui.group")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local svg = require("psycle.ui.svg")
local cfg = require("psycle.config"):new()

local controllerbox = group:new()

function controllerbox:new(parent)
  local m = group:new(parent)
  setmetatable(m, self)
  self.__index = self    
  m:init()
  return m
end

function controllerbox:init() 
  self.stickposx, self.stickposy, self.stickposz = 50, 50, 0     
  self:initstick(self)
end

function controllerbox:onbuttondown(button)    
    --g.buttondown:settext(button)      
end

function controllerbox:onbuttonup(button)    
  --  g.buttonup:settext(button)
end

function controllerbox:onxaxis(pos, oldpos)
   self:calcstickposx(pos)
   self.stick:fls()
end

function controllerbox:onzaxis(pos, oldpos)
  self:calcstickposz(pos)
  self.stick:fls()
end

function controllerbox:onyaxis(pos, oldpos)    
  self:calcstickposy(pos)
  self.stick:fls()
end  

function controllerbox:initstick(parent)  
  self.svg = svg:new(cfg:luapath().."\\gamecontrollertest\\note.svg")  
  self.stick = item:new(parent)
                   :setautosize(false, false)  
                   :viewdoublebuffered()                   
  self.stick:setposition(rect:new(point:new(0, 0), dimension:new(200, 200)))    
  local that = self
  function self.stick:draw(g)
    that.svg:draw(g)
    local dim = self:dimension()
    g:setcolor(0xFFFF0000)
    g:fillcircle(point:new(that.stickposx, that.stickposy), 10)
    g:setcolor(0xFF00FF00)
    g:drawcircle(point:new(that.stickposx, that.stickposy), 10)
    g:drawrect(rect:new(point:new(0, 0), dimension:new(dim:width()-1, dim:height()-1)))    
  end
  
  function self.stick:transparent()    
    return false
  end
end

function controllerbox:calcstickposx(pos)
  self.stickposx = self.stick:dimension():width() * pos/65335
  return self
end

function controllerbox:calcstickposy(pos)
  self.stickposy = self.stick:dimension():height() * pos/65335
  return self
end

function controllerbox:calcstickposz(pos)
  self.stickposz = self.stick:dimension():height() * pos/65335
  return self
end

function controllerbox:transparent()
  return false
end

return controllerbox

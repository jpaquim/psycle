-- moduledesigner.lua

local group = require("psycle.ui.canvas.group")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local text = require("psycle.ui.canvas.text")
local item = require("psycle.ui.canvas.item")
local button = require("psycle.ui.canvas.button")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local canvas = require("psycle.ui.canvas")
local rect = require("psycle.ui.canvas.rectanglebox")
local checkbox = require("psycle.ui.canvas.checkbox")
local edit = require("psycle.ui.canvas.edit")
local listener = require("listener")
local moduledesigner = group:new()

function moduledesigner:new(parent)
  local m = group:new(parent)
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function moduledesigner:init()
  self:setornament(ornamentfactory:createfill(0x528A68))   
  self:setautosize(false, false)  
end

function moduledesigner:setobjectinspector(objectinspector)
  self.objectinspector_ = objectinspector
end

function moduledesigner:addcanvas()  
  local item = canvas:new()
  self:adddrag(item, 500, 500)
end

function moduledesigner:addedit()  
  local item = edit:new():settext("Edit1")
  self:adddrag(item, 121, 21)
end

function moduledesigner:addcheckbox()
  local item = checkbox:new():settext("CheckBox1")
  self:adddrag(item, 97, 17)
  self.objectinspector_:setobject(item)
end

function moduledesigner:adddrag(item, w, h)    
  local g = group:new(self):setautosize(false, false):setposition(10, 10, w, h)    
  function g:onmousedown(ev)    
    self.down = true    
    self. dragstart = { x = ev.x, y = ev.y }
     self:canvas():addstyle(0x02000000)
  end
  function g:onmousemove(ev)    
    if self.down ~= nil then       
      local x, y = self:position()
      self:setposition(x + ev.x - self.dragstart.x, y + ev.y - self.dragstart.y)
    end
  end
  function g:onmouseup(ev)
    self.down = nil
    self:canvas():removestyle(0x02000000)
  end
  rect:new(g):setposition(0, 0, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(w/2 - 5, 0, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setpostion(w, 0, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(0, h, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(w/2 - 5, h, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(w, h, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(0, h/2 - 5, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  rect:new(g):setposition(w, h/2 - 5, 10, 10):setcolor(0x00FFFF):setautosize(false, false)
  g:add(item)
  item:setposition(5, 5, w, h)  
  --self.maincanvas:setornament(ornamentfactory:createfill(0xFFFF00)) 
  --local r = rect:new(g):setpos(0, 0, w, h)
end

return moduledesigner
-- psycle tabgroup (c) 2015 by psycledelics
-- File: tabgroup.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local canvas = require("psycle.ui.canvas")
local group = require("psycle.ui.canvas.group")
local window = require("psycle.ui.canvas.item")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local config = require("psycle.config")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local toolicon = require("psycle.ui.canvas.toolicon")
local frame = require("psycle.ui.canvas.frame")

local tabgroup = group:new()

tabgroup.cfgkeys = {
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
tabgroup.skin = {colors={}}
local cfg = config:new("MacParamVisual")
for i=1, #tabgroup.cfgkeys do
  local keyname = tabgroup.cfgkeys[i]
  tabgroup.skin.colors[string.upper(keyname)] = cfg:get("machineGUI"..keyname.."Color")
end

-- todo skin defaults ..
tabgroup.header = { color={} }
tabgroup.header.color.BG = 0x000000
tabgroup.header.color.LABEL = 0xFF0000
tabgroup.header.color.BORDER = 0x00FF00
tabgroup.header.color.HBG = 0xFFFFFF
tabgroup.header.color.HLABEL = 0xFF0000
tabgroup.header.color.HBORDER = 0x00FF00

function tabgroup:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)  
  self.__index = self
  c:init()
  return c
end

function tabgroup:init()
  --self:addstyle(0x02000000) 
  self:setdebugtext("tabgroup")
  self:setautosize(false, false)
  self.hh = 20  
  self.tabbar = group:new(self):setautosize(false, true):setalign(window.ALTOP)
  local picdir = cfg:luapath().."\\psycle\\ui\\icons\\"  
  local icon1 = toolicon:new(self.tabbar, picdir.."arrow_more.bmp", 0xFFFFFF) :setpos(0, 0, 10, 10):setalign(window.ALRIGHT)  
  local that = self
  function icon1:onclick()    
    self.f = frame:new()  
    c = canvas:new()
    local g = group:new(c)
    g:setautosize(false, true)
    g:setalign(window.ALTOP)
    local that2 = self
    function c:onkillfocus() 
      that2.f:hide()
    end
    self.f:setview(c)
    c:setornament(ornamentfactory:createfill(0x292929))     
    local fr = f
    function fun(item)
      local t = text:new(g):setcolor(0xB0C8B1):settext(item.text:text()):setautosize(false, true) 
      t:setalignment(window.ALCENTER):setalign(window.ALTOP)      
      function t:onmousedown()        
        that:setactivepage(item.page)
      end
      function t:onmouseenter()            
        t:setornament(ornamentfactory:createfill(0x528A68))                
      end
      function t:onmousemove()       
      end
      function t:onmouseout()               
        t:setornament(nil)                
      end
    end
    that:traverse(fun, that.tabs:items())
    c:updatealign()
    local x, y, w, h = g:pos()
    local x, y, iw, ih = icon1:desktoppos()
    self.f:hidedecoration()
    self.f:setpos(x + iw - 200, y + ih, 200, h)        
    self.f:show()    
    c:getfocus()
  end
  
  self.tabs = group:new(self.tabbar):setautosize(false, true):setalign(window.ALCLIENT)
  self.tabs:setornament(ornamentfactory:createboundfill(0x2F2F2F))    
      
  self.childs = group:new(self)
  self.childs:setornament(ornamentfactory:createboundfill(0x292929))
  self.childs:setclipchildren()    
  self.childs:setautosize(false, false)
  self.childs:setalign(window.ALCLIENT)
end

function tabgroup:setlabel(page, text)
  function f(item)  
    if item.page==page then 
     item.text:settext(text)
    end
  end
  self:traverse(f, self.tabs:items())
end

function tabgroup:addpage(page, label) 
  page:setautosize(false, false):setalign(window.ALCLIENT)
  self:createheader(page, label)
  self.childs:add(page)
  self:setactivepage(page)
  self:updatealign()
end

function tabgroup:activepage()
  return self.activepage_
end

function tabgroup:setactiveheader(page)
   if page then
    local function f(item)
      if item.page ~= page then
        item:setskinnormal()
      else
        item:setskinhighlight()
      end
    end
    self:traverse(f, self.tabs:items())
  end  
end

function tabgroup:setactivepage(page)
  if page then  
    self:preventfls()  
    local items = self.childs:items()  
    for i=1, #items do    
      local item = items[i]
      if item ~= page then
        item:hide()    
      end
    end
    self:enablefls()
    self:setactiveheader(page)
    self.activepage_ = page;  
    self.tabs:updatealign()
    page:show()
    self.childs:updatealign()
    self:fls()    
  end
end

function tabgroup:removepage(header)
  local pages = self.childs:items()
  local idx = self.childs:itemindex(header.page)  
  self.childs:remove(header.page)
  self.tabs:remove(header) 
  if idx == #pages then
    idx = idx -1
  end
  local pages = self.childs:items()
  self:setactivepage(pages[idx]) 
  self:fls()
end

function tabgroup:createheader(page, label)  
  local header = group:new(self.tabs):setautosize(true, true)
  header:setalign(window.ALLEFT)--:setmargin(0, 0, 1, 0)
  header.page = page    
  header.text = text:new(header):setdebugtext("text"):settext(label):setfont({name="Arial", height = "12"})
  header.text:setalign(window.ALLEFT)
  header.close = text:new(header)    
                     :setdebugtext("close")     
                     :setcolor(tabgroup.skin.colors.TITLEFONT) 
                     :settext("x")            
                                       
  header.close:setalign(window.ALLEFT)--:setmargin(4, 0, 4, 0)
  local that = self
  function header.close:onmousedown()  
    that:removepage(self:parent())    
  end
  
  function header:setskinhighlight()    
    self:setornament(ornamentfactory:createboundfill(0x528A68))      
    self.text:setcolor(0xB0C8B1) 
  end
  function header:setskinnormal()    
    self:setornament(nil)
    self.text:setcolor(0x528A68)        
  end  
  header:setskinnormal()  
  function header:onmousedown(ev)    
    that:setactivepage(self.page)     
  end
  function header:onmouseup(ev) end  
  return header
end

function tabgroup:traverse(f, arr)  
  for i=1, #arr do f(arr[i]) end  
  return self
end

return tabgroup
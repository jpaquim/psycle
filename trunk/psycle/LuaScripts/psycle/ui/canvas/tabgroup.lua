-- psycle tabgroup (c) 2015 by psycledelics
-- File: tabgroup.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local group = require("psycle.ui.canvas.group")
local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local config = require("psycle.config")
local style = require("psycle.ui.canvas.itemstyle")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()

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
  self.tabs = group:new(self):setautosize(false, true)
  self.tabs:setdebugtext("tabs")
  self.tabs:style():setalign(style.ALTOP)    
  self.tabs:setornament(ornamentfactory:createboundfill(0x2F2F2F))  
  --self.tabs:addstyle(0x02000000) 
  --self.tabs.border = ornamentfactory:createlineborder(0x528A68)
  --self.tabs.border:setborderradius(5, 5, 0, 0)
  ---self.tabs.border:setborderstyle(style.NONE, style.NONE, style.NONE, style.SOLID)
  --self.tabs:setornament(self.tabs.border)
  self.childs = group:new(self)
  self.childs:setornament(ornamentfactory:createboundfill(0x292929))
  self.childs:setclipchildren()
  self.childs:setdebugtext("childgroup")
  --self.childs:addstyle(0x02000000)  
  self.childs:setautosize(false, false)
  self.childs:style():setalign(style.ALCLIENT)
end

function tabgroup:setlabel(page, text)
  function f(item)  
    if item.page==page then 
     item.text:settext(text)
    end
  end
  self:traverse(f, self.tabs:items())
end

function tabgroup:add(page, label)
  page:setautosize(false, false)
  page:style():setalign(style.ALCLIENT)
  self:createheader(page, label)
  self.childs:add(page)
  self:setactivepage(page)
  self:align()
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
    self.tabs:align()
    page:show()
    self.childs:align()
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
  header:setdebugtext("header")
  header:style():setalign(style.ALLEFT):setmargin(0, 0, 1, 0)
 -- header.border = ornamentfactory:createlineborder(0x528A68)
  --header.border:setborderradius(5, 5, 0, 0)
  --header:setornament(header.border)
  --header:setornament(ornamentfactory:createboundfill(0x0000FF))
  header.page = page    
  header.text = text:new(header):setdebugtext("text"):settext(label):setfont({name="Arial", height = "12"})
  header.text:style():setalign(style.ALLEFT)--:setmargin(5, 4, 2, 4)    
  header.close = text:new(header)    
                     :setdebugtext("close")     
                     :setcolor(tabgroup.skin.colors.TITLEFONT) 
                     :settext("x")            
                                       
  header.close:style():setalign(style.ALLEFT):setmargin(4, 0, 4, 0)
  local that = self
  function header.close:onmousedown()  
    that:removepage(self:parent())    
  end
  
  function header:setskinhighlight()    
    self:setornament(ornamentfactory:createboundfill(0x528A68))      
    self.text:setcolor(0xB0C8B1)
    --self:invalidate()
  end
  function header:setskinnormal()    
    --self:setornament(ornamentfactory:createboundfill(0xFF0000))    
    self:setornament(nil)
    self.text:setcolor(0x528A68)    
    --self:invalidate()
  end  
  header:setskinnormal()  
  function header:onmousedown(ev)     
    --self:style():setalign(style.ALLEFT):setmargin(-10, 0, 1, 0)
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
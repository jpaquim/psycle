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
tabgroup.header.IDENT = 2

function tabgroup:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)  
  self.__index = self
  c:init()
  return c
end

function tabgroup:init()
  self.w, self.h, self.maintop = 100, 100, 20  
  self.tabs = group:new(self)
  self.childs = group:new(self):setpos(0, self.maintop)  
end

function tabgroup:onsize(w, h)
  local t = self.maintop
  function f(item) item:setsize(w, h - t) end
  self:traverse(f, self.childs:items())
  self.w, self.h  = w, h  
end

function tabgroup:setlabel(page, text)
   function f(item)  if item.page==page then item.text:settext(text) end end
   self:traverse(f, self.tabs:items())
end

function tabgroup:add(page, label)  
  self:createheader(page, label)
  self.childs:add(page)
  page:setsize(self.w, self.h - self.maintop)
  self:setactivepage(page)
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
    page:show()
    local items = self.childs:items()  
    for i=1, #items do    
      local item = items[i]
      if item ~= page then
        items[i]:hide()    
      end
    end        
    self:setactiveheader(page)
    self.activepage_ = page;
  end
end

function tabgroup:removepage(header)
  local pages = self.childs:items()
  local idx = self.childs:itemindex(header.page)  
  self.childs:remove(header.page)
  self.tabs:remove(header)
  self:repositionheader()  
  if idx == #pages then
    idx = idx -1
  end
  local pages = self.childs:items()
  self:setactivepage(pages[idx])  
end

function tabgroup:repositionheader()
  local itemw, xpos = 100, 0
  local function f(item)    
    item:setpos(xpos, 0)    
    xpos = xpos + (itemw+tabgroup.header.IDENT)
  end
  self:traverse(f, self.tabs:items())
end

function tabgroup:createheader(page, label)
  local header = group:new(self.tabs)  
  header.page = page  
  local itemcount, itemw = self.childs:itemcount(), 100  
  local xpos = itemcount*(itemw+tabgroup.header.IDENT)
  header:setpos(xpos, 0)
  header.backgroundtop = rect:new(header):setpos(0, 0, itemw, 4)
  header.backgroundbottom = rect:new(header):setpos(0, 4, itemw, self.maintop - 4)
  header.text = text:new(header):setpos(2, 5):settext(label)
  header.closerect = rect:new(header)
                         :setpos(itemw -10, 5, 10, 10)
                         :setcolor(tabgroup.skin.colors.TOP)  
  header.close = text:new(header)
                     :setpos(itemw - 7, 4)
                     :settext("x")
                     :setcolor(tabgroup.skin.colors.TITLEFONT)          
  local that = self   
  function header.closerect:onmousedown() end
  function header.closerect:onmouseup()       
    that:removepage(self:parent())    
  end
  function header:setskinhighlight()
    self.backgroundtop:setcolor(tabgroup.skin.colors.HTOP)
    self.backgroundbottom:setcolor(tabgroup.skin.colors.HBOTTOM)
                   --:setstrokecolor(tabgroup.header.color.HBORDER)    
    self.text:setcolor(tabgroup.skin.colors.HFONTBOTTOM)
  end
  function header:setskinnormal()
    self.backgroundtop:setcolor(tabgroup.skin.colors.TOP)
    self.backgroundbottom:setcolor(tabgroup.skin.colors.BOTTOM)    
                   --:setstrokecolor(tabgroup.header.color.BORDER)
    self.text:setcolor(tabgroup.skin.colors.FONTBOTTOM)
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

function tabgroup:onupdateregion(rgn)
  rgn:setrect(0, 0, self.w, self.h)
end

return tabgroup
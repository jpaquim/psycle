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
  self.hh = 20
  self.tabs = group:new(self)
  self.tabs:style():setalign(style.ALTOP)
  self.childs = group:new(self)
  self.childs:style():setalign(style.ALCLIENT)
end

function tabgroup:setlabel(page, text)
   function f(item)  if item.page==page then item.text:settext(text) end end
   self:traverse(f, self.tabs:items())
end

function tabgroup:add(page, label)
  page:style():setalign(style.ALCLIENT)
  self:createheader(page, label)
  self.childs:add(page)
  self:setactivepage(page)
  self:canvas():updatealign()
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
    local items = self.childs:items()  
    for i=1, #items do    
      local item = items[i]
      if item ~= page then
        item:hide()    
      end
    end        
    self:setactiveheader(page)
    self.activepage_ = page;
    page:show()
    self.childs:updatealign()
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
end

function tabgroup:createheader(page, label)
  local header = group:new(self.tabs)
  header:style():setalign(style.ALLEFT)  
  header.page = page       
  header.backgroundtop = rect:new(header):setheight(4):setcolor(tabgroup.skin.colors.TOP)
  header.backgroundtop:style():setalign(style.ALTOP)  
  header.closegroup = group:new(header)
  header.closegroup:style():setalign(style.ALRIGHT)  
  header.closerect = rect:new(header.closegroup):setwidth(10)
  header.closerect:setcolor(tabgroup.skin.colors.TOP)
  header.closerect:style():setalign(style.ALTOP)
  header.close = text:new(header.closegroup)
                     :setpos(3, 0)
                     :settext("x")
                     :setcolor(tabgroup.skin.colors.TITLEFONT)   
  local g = group:new(header)
  g:style():setalign(style.ALTOP)  
  header.text = text:new(header):setpos(2, 0):settext(label)
  header.text:style():setalign(style.ALTOP):setmargin(5, 3, 5, 3)
    --                     :setpos(0, 0, 10, 10)
                           --:setcolor(0xFFFFF) --tabgroup.skin.colors.TOP)  
  header.close = text:new(header.closegroup)
                     :setpos(3, 0)
                     :settext("x")
                     :setcolor(tabgroup.skin.colors.TITLEFONT)   

--header.backgroundbottom = rect:new(g)
  --header.backgroundbottom:style():setalign(style.ALTOP)
  --header.backgroundbottom:setcolor(0xFF0000)
                       
  
  local that = self
  function header.closegroup:onmousedown()
    that:removepage(self:parent())    
  end
  
  function header:setskinhighlight()
    self.backgroundtop:setcolor(tabgroup.skin.colors.HTOP)
    self.text:setfillcolor(tabgroup.skin.colors.HBOTTOM)
                   --:setstrokecolor(tabgroup.header.color.HBORDER)    
    self.text:setcolor(tabgroup.skin.colors.HFONTBOTTOM)
  end
  function header:setskinnormal()
    self.backgroundtop:setcolor(tabgroup.skin.colors.TOP)
    self.text:setfillcolor(tabgroup.skin.colors.BOTTOM)    
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


return tabgroup
-- psycle tabgroup (c) 2015 by psycledelics
-- File: tabgroup.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local canvas = require("psycle.ui.canvas")
local group = require("psycle.ui.canvas.group")
local window = require("psycle.ui.canvas.item")
--local rect = require("psycle.ui.canvas.rect")
local text = require("psycle.ui.canvas.text")
local config = require("psycle.config")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local toolicon = require("psycle.ui.canvas.toolicon")
local frame = require("psycle.ui.canvas.frame")
local signal = require("psycle.signal")

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
tabgroup.picdir = cfg:luapath().."\\psycle\\ui\\icons\\"

function tabgroup:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)  
  self.__index = self
  c:init()
  return c
end

function tabgroup:init()
  self.isoldflsprevented_ = {}
  self.dopageclose = signal:new()
  self.hasclosebutton_ = true
  --self:addstyle(0x02000000)  
  self:setautosize(false, false)
  self.hh = 20  
  self.tabbar = group:new(self):setautosize(false, true):setalign(window.ALTOP)  
  local icon1 = toolicon:new(self.tabbar, tabgroup.picdir.."arrow_more.bmp", 0xFFFFFF) :setpos(0, 0, 10, 10):setalign(window.ALRIGHT)  
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
        self:setornament(ornamentfactory:createfill(0xFF0000)):fls()
      end
      function t:onmousemove()       
      end
      function t:onmouseout()               
        self:setornament(nil):fls()
      end
    end
    that:traverse(fun, that.tabs:items())
    c:updatealign()
    local x, y, w, h = g:pos()
    local x, y, iw, ih = icon1:desktoppos()
    self.f:hidedecoration()
    self.f:setpos(x + iw - 200, y + ih, 200, h)        
    self.f:show()    
    c:setfocus()    
  end
  
  self.tabs = group:new(self.tabbar):setautosize(false, true):setalign(window.ALCLIENT)
  self.tabs:setornament(ornamentfactory:createboundfill(0x2F2F2F))    
      
  self.children = group:new(self)
  self.children:setornament(ornamentfactory:createboundfill(0x292929))
  self.children:setclipchildren()    
  self.children:setautosize(false, false)
  self.children:setalign(window.ALCLIENT)
end

function tabgroup:setlabel(page, text)
  function f(item)  
    if item.page==page then 
     item.text:settext(text)
    end
  end
  self:traverse(f, self.tabs:items())
end

function tabgroup:saveflsstate()
  self.isoldflsprevented_[#self.isoldflsprevented_+1] = self:isflsprevented()  
  return self
end

function tabgroup:restoreflsstate()  
  if self.isoldflsprevented_[#self.isoldflsprevented_] then
    self:preventfls()
  else
    self:enablefls()
  end
  self.isoldflsprevented_[#self.isoldflsprevented_] = nil
  return self
end

function tabgroup:addpage(page, label)    
  self:saveflsstate():preventfls()
  page:setautosize(false, false):setalign(window.ALCLIENT)
  self:createheader(page, label)
  self.children:add(page)  
  self:setactivepage(page)    
  self.tabs:updatealign()  
  self.children:updatealign()    
  self:restoreflsstate()
  self:invalidate()
end

function tabgroup:activepage()
  return self.activepage_
end

function tabgroup:setactiveheader(page)  
   if page then    
    local items = self.tabs:items()
    if self.activepage_ then
      items[self.children:itemindex(self.activepage_)]:setskinnormal()
    end
    items[self.children:itemindex(page)]:setskinhighlight()
  end    
end

function tabgroup:setactivepage(page)
  if page then     
    self:saveflsstate():preventfls()
    if self.activepage_ then
      self.activepage_:hide()
    end    
    self:setactiveheader(page)
    self.activepage_ = page;    
    page:show()
    self:restoreflsstate():invalidate()
  end
end

function tabgroup:enableclosebutton()
  self.hasclosebutton_ = true
  return self
end

function tabgroup:disableclosebutton()
  self.hasclosebutton_ = false
  return self
end

function tabgroup:removeall()
  self.activepage_ = nil
  self.tabs:removeall()
  self.children:removeall()
  self:updatealign()
end

function tabgroup:removepage(page)  
  local tabs = self.tabs:items()  
  for i=1, #tabs do
    if tabs[i].page == page then
      self:removepagebyheader(tabs[i])
      break;
    end
  end  
end

function tabgroup:removepagebyheader(header)
  self:saveflsstate():preventfls()
  local pages = self.children:items()
  local idx = self.children:itemindex(header.page)  
  self.children:remove(header.page)
  self.activepage_ = nil
  self.tabs:remove(header) 
  if idx == #pages then
    idx = idx -1
  end
  local pages = self.children:items()
  self.tabs:updatealign()  
  self:setactivepage(pages[idx]) 
  self:restoreflsstate():invalidate()
end

function tabgroup:createheader(page, label)
  local header = group:new(self.tabs):setautosize(true, true)  
  header:setalign(window.ALLEFT):setmargin(0, 0, 5, 0)
  header.page = page    
  header.text = text:new(header):setdebugtext("text"):settext(label):setfont({name="Arial", height = "12"})
  header.text:setalign(window.ALLEFT)
  local that = self
  if self.hasclosebutton_ then
    header.close = text:new(header)    
                       :setdebugtext("close")     
                       :setcolor(tabgroup.skin.colors.TITLEFONT) 
                       :settext("x")                                                            
    header.close:setalign(window.ALLEFT):setmargin(4, 0, 0, 0)    
    function header.close:onmousedown()
      ev = {}
      ev.page = header.page      
      that.dopageclose:emit(ev)           
      that:removepagebyheader(self:parent())
    end
  end
  function header:setskinhighlight()        
    self:setornament(ornamentfactory:createboundfill(0x528A68))    
    self.text:setcolor(0xFFFF00) 
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
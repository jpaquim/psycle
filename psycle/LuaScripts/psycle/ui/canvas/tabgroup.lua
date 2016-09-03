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
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local toolicon = require("psycle.ui.canvas.toolicon")
local frame = require("psycle.ui.canvas.frame")
local popupframe = require("psycle.ui.canvas.popupframe")
local popupmenu = require("psycle.ui.popupmenu")
local node = require("psycle.node")
local signal = require("psycle.signal")
local centertoscreen = require("psycle.ui.canvas.centertoscreen")

local tabgroup = group:new()
--local cfg = config:new("MacParamVisual")
-- cfg:luapath()
local plugin_path = "C:\\Users\\User\\Documents\\Visual Studio 2015\\Projects\\exforward\\exforward\\LuaScripts"
tabgroup.picdir = plugin_path.."\\psycle\\ui\\icons\\"

function tabgroup:new(parent)  
  local c = group:new(parent)  
  setmetatable(c, self)  
  self.__index = self
  c:init()
  return c
end

function tabgroup:init()
  self.frames = {}
  self.isoldflsprevented_ = {}
  self.dopageclose = signal:new()
  self.hasclosebutton_ = true
  --self:setclipchildren()
  --self:addstyle(0x02000000)  
  self:setautosize(false, false)
  self.tabbar = group:new(self):setautosize(false, true):setalign(window.ALTOP):addornament(ornamentfactory:createfill(0x232323)):setpadding(2, 0, 2, 2)
  local icon1 = toolicon:new(self.tabbar, tabgroup.picdir.."arrow_more.bmp", 0xFFFFFF) :setautosize(false, false):setpos(0, 0, 15, 10):setalign(window.ALRIGHT)    
  local that = self
  self:inittabpopupmenu()
  self:initframepopupmenu()
  function icon1:onclick()    
    self.f = popupframe:new()
    --self.f:addstyle(0x02000000)
    self.c = canvas:new():invalidatedirect()
    local that1 = self    
    function self.c:onmousedown(ev)
      that1.f:hide()
    end
    local g = group:new(self.c):setautosize(false, true):setalign(window.ALTOP)    
    self.f:setviewport(self.c)
    self.c:addornament(ornamentfactory:createfill(0x292929))    
    function fun(item)
      local t = text:new(g):setcolor(0xCACACA):settext(item.text:text()):setautosize(false, true)      
      t:setjustify(window.ALCENTER):setalign(window.ALTOP)      
      function t:onmousedown()        
        that:setactivepage(item.page)
      end
      function t:onmouseenter()                            
        self:addornament(ornamentfactory:createlineborder(0x696969))
      end      
      function t:onmouseout()                       
        self:removeornaments()
      end
    end
    that:traverse(fun, that.tabs:items())
    self.c:updatealign()
    local x, y, w, h = g:pos()
    local x, y, iw, ih = icon1:desktoppos()
    self.f:hidedecoration()
    self.f:setpos(x + iw - 200, y + ih, 200, h)       
    self.f:show()            
  end  
  self.tabs = group:new(self.tabbar):setautosize(false, true):setalign(window.ALCLIENT):addornament(ornamentfactory:createfill(0x232323))      
  self.children = group:new(self):setautosize(false, false):setalign(window.ALCLIENT)
  --self.children:setclipchildren()
  self.children:addornament(ornamentfactory:createboundfill(0x232323))  
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
  page:setautosize(false, false)
  page:setalign(window.ALCLIENT)
  self:createheader(page, label)
  self.children:add(page)  
  self:setactivepage(page) 
  self.tabs:updatealign()
  self.tabbar:updatealign()
end

function tabgroup:activepage()
  if self.activeframepage_ ~= nil then
    return self.activeframepage_
  else
    return self.activepage_
  end
end

function tabgroup:setactiveheader(page)  
   if page then    
    local items = self.tabs:items()
    if self.activepage_ then
      local index = self.children:itemindex(self.activepage_)
      if index ~= 0 then
        items[index]:setskinnormal()
      end
    end
    local index = self.children:itemindex(page)
    if index ~= 0 then
      items[index]:setskinhighlight()
    end
  end    
end

function tabgroup:setactivepage(page)
  if page then     
    --self:saveflsstate():preventfls()
    if self.activepage_ then
      self.activepage_:hide()
    end    
    self:setactiveheader(page)
    self.activepage_ = page;        
	page:setdebugtext("ps")
    page:show()
	self:updatealign()    
    --self:restoreflsstate():invalidate()
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
  self.activeframepage_ = nil
  self.tabs:removeall()
  collectgarbage()
  self.children:removeall()
  collectgarbage()
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
  header:setalign(window.ALLEFT)  
  header.page = page    
  header.text = text:new(header):settext(label):setfont({name="Arial", height = "12"})
  header.text:setalign(window.ALLEFT):setmargin(5, 5, 5, 5)  
  local that = self
  if self.hasclosebutton_ then
    header.close = text:new(header)                           
                       :setcolor(0xB0D8B1)
                       :settext("x")                       
    header.close:setalign(window.ALLEFT):setmargin(5, 5, 5, 5)   
    function header.close:onmousedown()
      local ev = {}
      ev.page = self:parent().page
      that.dopageclose:emit(ev)
      that:removepagebyheader(self:parent())
    end
  end
  function header:setskinhighlight()        
    self:setpadding(0, 0, 0, 0)
	self:removeornaments()    
	self:addornament(ornamentfactory:createfill(0x333333)) 
	self:addornament(ornamentfactory:createlineborder(0x696969))    
    self.text:setcolor(0xFFFFFF) 	
  end
  function header:setskinnormal()
    self:setpadding(1, 1, 1, 1)
    self:removeornaments()	
    self.text:setcolor(0xC0C0C0)     
  end    
  header:setskinnormal()    
  function header:onmouseenter(ev)    
    if self.page == that:activepage() then
	  self.text:setcolor(0xFFFFFF) 
	else
	  self.text:setcolor(0xFFFFFF)    
	end
  end
  function header:onmouseout(ev)
    if self.page == that:activepage() then
	  self.text:setcolor(0xFFFFFF) 
	else
      self.text:setcolor(0xC0C0C0) 	
	end
  end  
  function header:onmousedown(ev)        
    that:setactivepage(self.page)
    if ev.button == 2 then
      local x, y = self:desktoppos()
      that.tabpopupmenu:track(x + 5, y + 5)
      that.tabpopupmenu.headertext = self.text:text()
    end    
  end
  function header:onmouseup(ev) end  
  return header
end

function tabgroup:inittabpopupmenu() 
  self.tabpopuprootnode = node:new()
  local node1 = node:new():settext("Close")
  self.tabpopuprootnode:add(node1)
  local node2 = node:new():settext("Show As Window")
  self.tabpopuprootnode:add(node2)
  self.tabpopupmenu = popupmenu:new():setrootnode(self.tabpopuprootnode):update()    
  local that = self    
  function self.tabpopupmenu:onclick(node)       
    if node:text() == "Show As Window" then      
      that:openinframe(that.activepage_, self.headertext)      
    elseif node:text() == "Close" then
      local ev = {}
      ev.page = that:activepage()
      that.dopageclose:emit(ev)
      that:removepage(ev.page)
    end
  end  
end

function tabgroup:initframepopupmenu() 
  self.framepopuprootnode = node:new()
  local node1 = node:new():settext("Close")
  self.framepopuprootnode:add(node1)
  local node2 = node:new():settext("Dock To Tabs")
  self.framepopuprootnode:add(node2)
  self.framepopupmenu = popupmenu:new():setrootnode(self.framepopuprootnode):update()    
  local that = self
  function self.framepopupmenu:onclick(node)       
    if node:text() == "Dock To Tabs" then      
      that:openintab(self.frame_)      
      self.frame_ = nil
    elseif node:text() == "Close" then
      local page = self.frame_:view():items()[1]
      local ev = {}
      ev.page = page
      that.dopageclose:emit(ev)
      that:unregisterframe(self.frame_)      
      that:removeframepagepointer()      
    end    
  end  
end

function tabgroup:openinframe(page, name)    
  local canvas = canvas:new()  
  self:removepage(page)
  self.activepage_ = nil  
  canvas:add(page)
  local frame = self:createframe(canvas, name)
  self.framepopupmenu.frame_ = frame
  frame:setpopupmenu(self.framepopupmenu)
  frame:show(centertoscreen:new():sizetoscreen(0.3, 0.5))
end

function tabgroup:createframe(canvas, name)  
  local frame = frame:new():setview(canvas):settitle(name)  
  frame.tabgroup = self
  self.frames[#self.frames+1] = frame
  local that = self
  function frame:oncontextpopup(ev)
     that.framepopupmenu.frame_ = self
  end
  function frame:onclose(ev)
    local page = self:view():items()[1]
    local ev = {}
    ev.page = page
    that.dopageclose:emit(ev)
    for i = 1, #that.frames do
      if that.frames[i] == self then
        that.frames[i] = nil
        break
      end
    end
    that.framepopupmenu.frame_ = nil
    that.activeframe_ = nil
    that.activeframepage_ = nil
    self = nil  
  end
  function frame:onfocus()
   local page = self:view():items()[1]
   self.tabgroup.activepage_ = nil
   self.tabgroup.activeframepage_ = page
   self.tabgroup.framepopupmenu.frame_ = self
  end
  function frame:onkillfocus()
    self.activeframepage_ = nil
  end
  return frame
end

function tabgroup:openintab(frame)     
  local page = frame:view():items()[1]
  local name = frame:title()
  frame:view():remove(page)
  self:unregisterframe(frame)  
  self:removeframepagepointer()
  self:hideallpages()
  self:setnormalskintoalltabs()
  self:addpage(page, name)
  self:setactivepage(page)    
  self.tabs:updatealign()  
  self.children:updatealign()    
end

function tabgroup:hideallpages()
  local pages = self.children:items()
  for i=1, #pages do
    pages[i]:hide()
  end
end

function tabgroup:removeframepagepointer()
  self.activepage_ = nil
  self.activeframepage_ = nil
  self.framepopupmenu.frame_ = nil
end

function tabgroup:setnormalskintoalltabs()
  local tabs = self.tabs:items()
  for i=1, #tabs do
    tabs[i]:setskinnormal()
  end
end  


function tabgroup:unregisterframe(frame)
  for i = 1, #self.frames do
    if self.frames[i] == frame then
      self.frames[i] = nil
      break
    end  
  end
end

function tabgroup:traverse(f, arr)  
  for i=1, #arr do f(arr[i]) end  
  return self
end

return tabgroup
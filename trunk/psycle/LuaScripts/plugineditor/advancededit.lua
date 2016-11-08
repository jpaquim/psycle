local signal = require("psycle.signal")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local font = require("psycle.ui.font")
local fontinfo = require("psycle.ui.fontinfo")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local edit = require("psycle.ui.edit")
local systems = require("psycle.ui.systems")


local advancededit = group:new()

advancededit.windowtype = 51

function advancededit:new(parent, setting)  
  local c = group:new(parent)
                 :setautosize(false, false)  
  setmetatable(c, self)
  self.__index = self  
  c:init(setting)  
  systems:new():changewindowtype(advancededit.windowtype, c)
  return c
end

function advancededit:init(setting)
  self.edit_ = edit:new(self)
                   :setalign(item.ALCLIENT)   
  self:initdefaultcolors()	   
  self.edit_:setautosize(false, false)
            :settext("Name of Machine to Edit?")                
	        :setcolor(self.color_)
            :setbackgroundcolor(self.backgroundcolor_)
            :setmargin(boxspace:new(10))	    
  local that = self			  
  function self.edit_:onfocus()
    that:onfocus()
  end
  function self.edit_:onmousedown(ev)
    that:onmousedown(ev)        
  end
  function self.edit_:onkillfocus()
    that:onkillfocus()
  end    
  function self.edit_:onkeydown(ev)
    that:onkeydown(ev)
  end  
  function self.edit_:onkeyup(ev)
    that:onkeydown(ev)    
  end  
end

function advancededit:setfocus()
  self.edit_:setfocus()
  return self
end

function advancededit:onkeydown(ev)
  if ev:keycode() == ev.RETURN then	  	  
  elseif ev:keycode() == ev.ESCAPE then
	self.selectorhasfocus = nil
	self:settext(self.oldtext)	  
    self:parent():setfocus()
  elseif self.firstclick then
	self:settext("")
	self.firstclick = nil	   
  end
end

function advancededit:onkeyup(ev)
  if not self.firstclick  and self:text():len() == 0 then
	self:settext("Name of Machine to Edit?")
	self.firstclick = true	
  end
end

function advancededit:onfocus()
  self.oldtext = self:text()	
  self:settext("Name of Machine to Edit?")
  self.firstclick = true  
  self.selectorhasfocus = true
  self:setcolor(self.activecolor_)
      :setbackgroundcolor(self.activebackgroundcolor_)  
  self.edit_:setsel(0, 0)
end

function advancededit:onkillfocus()
  self.firstclick = true
  self.selectorhasfocus = nil
  self:settext(self.oldtext)
  if self.selectorhasmouseenter then
	self:onmouseenter()
  else
	self:onmouseout()  
  end
end
  
function advancededit:setcolor(color) 
  self.edit_:setcolor(color)
  return self
end

function advancededit:setbackgroundcolor(color) 
  self.edit_:setbackgroundcolor(color)
  self:removeornaments()
  self:addornament(ornamentfactory:createfill(color))
  return self
end

function advancededit:settext(text)
  self.edit_:settext(text)
  return self
end

function advancededit:text(text)
  return self.edit_:text()
end

function advancededit:initdefaultcolors()
  self.color_ = 0xFF999999    
  self.backgroundcolor_ = 0xFF333333
  self.activecolor_ = 0xFFCACACA
  self.activebackgroundcolor_ = 0xFF568857
  self.hovercolor_ = 0xFF999999
  self.hoverbackgroundcolor_ = 0xFF333333
  return self
end

function advancededit:onmouseenter()
    self.selectorhasmouseenter = true
    if not self.selectorhasfocus then
	  self:setcolor(self.hovercolor_)
      self:setbackgroundcolor(self.hoverbackgroundcolor_)
	end
end

function advancededit:onmouseout()
  self.selectorhasmouseenter = nil
  if not self.selectorhasfocus then
	self:setcolor(self.color_)	  
    self:setbackgroundcolor(self.backgroundcolor_)	
  end
end

function advancededit:onmousedown(ev) 
  if not self.selectorhasfocus then
    ev:preventdefault()
	ev:stoppropagation()
	self:setfocus()
  end
end

function advancededit:setproperties(properties)  
  if properties.color then
    self.color_ = properties.color:value()
	self:setcolor(self.color_)
  end
  if properties.backgroundcolor then
    self.backgroundcolor_ = properties.backgroundcolor:value()          
	self:setbackgroundcolor(self.backgroundcolor_)
  end
  if properties.activecolor then
    self.activecolor_ = properties.activecolor:value()
  end
  if properties.activebackgroundcolor then
    self.activebackgroundcolor_ = properties.activebackgroundcolor:value()
  end
  if properties.hovercolor then
    self.hovercolor_ = properties.hovercolor:value()
  end
  if properties.hoverbackgroundcolor then
    self.hoverbackgroundcolor_ = properties.hoverbackgroundcolor:value()
  end  
  self:fls()
end

return advancededit
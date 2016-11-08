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
local text = require("psycle.ui.text")
local edit = require("psycle.ui.edit")
local systems = require("psycle.ui.systems")


local statusbar = group:new()

statusbar.windowtype = 52

function statusbar:new(parent, setting)  
  local c = group:new(parent)
                 :setautosize(false, false)  
  setmetatable(c, self)
  self.__index = self  
  c:init(setting)  
  systems:new():changewindowtype(statusbar.windowtype, c)
  return c
end

function statusbar:init(setting)  
  self:setalign(item.ALRIGHT):setautosize(true, false)        
  self.searchrestartstatus = text:new(self)
                                 :settext("")
								 :setautosize(false, false)
								 :setposition(rect:new(point:new(0, 0), dimension:new(150, 0)))
								 :setalign(item.ALLEFT)
								 :setverticalalignment(item.ALCENTER)
								 :setmargin(boxspace:new(0, 0, 0, 5))								 								 
  self.modifiedstatus = text:new(self)
                            :settext("")							
							:setautosize(false, false)
							:setposition(rect:new(point:new(0, 0), dimension:new(100, 0)))
							:setalign(item.ALLEFT)
							:setverticalalignment(item.ALCENTER)
							:setmargin(boxspace:new(0, 5, 0, 0))							
  self.linelabel = text:new(self)
      :settext("LINE")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))	  
  self.linestatus = text:new(self)
                        :settext("1")						
						:setautosize(false, false)
						:setposition(rect:new(point:new(0, 0), dimension:new(30, 0)))
						:setalign(item.ALLEFT)
						:setverticalalignment(item.ALCENTER)
						:setmargin(boxspace:new(0, 5, 0, 0))						
  self.collabel = text:new(self)
      :settext("COL")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))	   
  self.colstatus = text:new(self)
                       :settext("1")
					   :setautosize(false, false)
					   :setposition(rect:new(point:new(), dimension:new(30, 0)))
					   :setalign(item.ALLEFT)
					   :setverticalalignment(item.ALCENTER)
					   :setmargin(boxspace:new(0, 5, 0, 0))					   
  self.insertlabel = text:new(self)
      :settext("INSERT")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))	  
  self.insertstatus = text:new(self)
                          :settext("ON")						  
						  :setautosize(false, false)
						  :setposition(rect:new(point:new(), dimension:new(30, 0)))
						  :setverticalalignment(item.ALCENTER)
						  :setalign(item.ALLEFT)						  
end

function statusbar:initdefaultcolors()
  self.color_ = 0xffffffff
  self.statuscolor_ = 0xffb0d8b1    
  self.backgroundcolor_ = 0xFF333333
  return self
end

function statusbar:setcolor(color) 
  self.color_ = color
  self.linelabel:setcolor(color)
  self.collabel:setcolor(color)
  self.insertlabel:setcolor(color)  
  return self
end

function statusbar:setstatuscolor(color) 
  self.statuscolor_ = color
  self.linestatus:setcolor(color)
  self.colstatus:setcolor(color)
  self.insertstatus:setcolor(color)
  self.searchrestartstatus:setcolor(color)  
  self.modifiedstatus:setcolor(color)
  return self
end

function statusbar:setbackgroundcolor(color) 
  self.backgroundcolor_ = color
  self.linestatus:removeornaments():addornament(ornamentfactory:createfill(color))
  self.colstatus:removeornaments():addornament(ornamentfactory:createfill(color))
  self.insertstatus:removeornaments():addornament(ornamentfactory:createfill(color))
  self.searchrestartstatus:removeornaments():addornament(ornamentfactory:createfill(color))
  self.modifiedstatus:removeornaments():addornament(ornamentfactory:createfill(color))
  self:removeornaments()
  self:addornament(ornamentfactory:createfill(color))
  return self
end

function statusbar:setproperties(properties)  
  if properties.color then    
	self:setcolor(properties.color:value())
  end
  if properties.statuscolor then    
	self:setstatuscolor(properties.statuscolor:value())
  end
  if properties.backgroundcolor then    
	self:setbackgroundcolor(properties.backgroundcolor:value())
  end  
  self:fls()  
end

return statusbar
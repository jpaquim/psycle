-- modulepalette.lua

local group = require("psycle.ui.canvas.group")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local text = require("psycle.ui.canvas.text")
local item = require("psycle.ui.canvas.item")

local modulepalette = tabgroup:new()

function modulepalette:new(parent)
  local m = tabgroup:new(parent)
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function modulepalette:init()
  self:setautosize(false, false)
  self:disableclosebutton()
  self:createcategory("Standard")
  self:createcategory("Additional")
  self:createcategory("System")
  self:createcategory("Dialogs")
  self:createcategory("Audio")
  self:createcategory("Host")
  self:createcategory("Templates")
end

function modulepalette:setdesigner(moduledesigner)
  self.moduledesigner_ = moduledesigner
end

function modulepalette:createcategory(name)
  local page = group:new():setautosize(false, false)  
  local that = self
  local i1 = text:new(page):settext("Canvas"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  function i1:onmousedown(self, ev)
    that.moduledesigner_:addcanvas()  
  end
  text:new(page):settext("Group"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)   
  text:new(page):settext("Frame"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("MainMenu"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("PopupMenu"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("Label"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  local i2 = text:new(page):settext("Edit"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  function i2:onmousedown(self, ev)
    that.moduledesigner_:addedit()
  end
  text:new(page):settext("Combobox"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  local i3 = text:new(page):settext("Checkbox"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  function i3:onmousedown(self, ev)
    that.moduledesigner_:addcheckbox()  
  end
  
  text:new(page):settext("Scrollbar"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("Image"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("Scintilla"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)
  text:new(page):settext("TreeView"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)  
  text:new(page):settext("ListView"):setalign(item.ALLEFT):setautosize(true, false):setmargin(0, 0, 10, 0)      
  self:addpage(page, name)  
end

return modulepalette
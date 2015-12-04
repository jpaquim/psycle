-- psycle plugineditor (c) 2015 by psycledelics
-- File: maincanvas.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


-- require('mobdebug').start()

local canvas = require("psycle.ui.canvas")

local frame = require("psycle.ui.canvas.frame")
local rect = require("psycle.ui.canvas.rect")
local scrollbar = require("psycle.ui.canvas.scrollbar")
local group = require("psycle.ui.canvas.group")
local toolbar = require("psycle.ui.canvas.toolbar")
local toolicon = require("psycle.ui.canvas.toolicon")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local scintilla = require("psycle.ui.canvas.scintilla")
local sci = require("scintilladef")
local scilex = require("scilexerdef")
local config = require("psycle.config")
local machine = require("psycle.machine")
local fileopen = require("psycle.ui.fileopen")
local filesave = require("psycle.ui.filesave")
local settings = require("settings")

local maincanvas = canvas:new()

function maincanvas:new()
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  return c
end

function maincanvas:init()
  self.fileopen = fileopen:new()
  local that = self
  function self.fileopen:onok(fname) that:openfromfile(fname) end
  self.filesaveas = filesave:new()  
  self.toolbartop = 5
  self.tabbartop = 30  
  self.outputheight = 200
  self.splitterheight = 5
  self.skin = self:initskin()
  self:setcolor(self.skin.colors.background);
  self.pages = tabgroup:new(self)  
  self.outputs = tabgroup:new(self)
  self.splitter = rect:new(self)
                      :setcolor(self.skin.colors.rowbeat)
  local that = self           
  function self.splitter:onmousedown(e)
    self:canvas():mousecapture()
  end  
  function self.splitter:onmousemove(e)         
    that:setcursor(canvas.CURSOR.ROW_RESIZE)    
    if e.button == 1 then
      local cw, ch = that:clientsize()
      that.outputheight = math.min(math.max(that.splitterheight, ch - e.clienty), ch-that.tabbartop-that.splitterheight)
      that:onsize(cw, ch)
    end
  end
  function self.splitter:onmouseout()        
    that:setcursor(canvas.CURSOR.DEFAULT)
  end
  function self.splitter:onmouseup(e)
    self:canvas():mouserelease()
  end  
  self.output = scintilla:new()  
  self.outputs:add(self.output, "Output")  
  self.newpagecounter = 1
  local cfg = config:new("PatternVisual")
  maincanvas.picdir = cfg:luapath().."\\psycle\\ui\\icons\\"
  local tb = self:initfiletoolbar(0)
  self:initplaytoolbar(tb.xp+20)
end

function maincanvas:setoutputtext(text)
  self.output:f(sci.SCI_ADDTEXT, text:len(), text)  
end

function maincanvas:dopageexist(fname)
  local found = nil
  local items = self.pages.childs:items()    
  for i=1, #items do
    local page = items[i]
    if page:filename() == fname then
       found = page
       break
    end
  end
  return found
end

function maincanvas:setlexer(page)
  page:f(sci.SCI_SETLEXER, scilex.SCLEX_LUA, 0)
  page:f(sci.SCI_SETKEYWORDS, settings.editor.keywords, 0)
  page:f(sci.SCI_STYLESETSIZE, sci.STYLE_DEFAULT, settings.editor.font.size)
  page:f(sci.SCI_STYLESETFONT, sci.STYLE_DEFAULT, settings.editor.font.name) 
  page:f(sci.SCI_STYLECLEARALL, 0, 0)
  for k, v in pairs(settings.editor.colors) do       
     local r = bit32.band(bit32.rshift(v, 16), 0xFF)
     local g = bit32.band(bit32.rshift(v, 8), 0xFF)
     local b = bit32.band(v, 0xFF)               
     v = bit32.bor(bit32.bor(r, bit32.lshift(g, 8)), bit32.lshift(b, 16))
     bit32.band(bit32.band(bit32.rshift(v, 16), bit32.rshift(v, 8)), bit32.rshift(v, 0))
     page:f(sci.SCI_STYLESETFORE, scilex["SCE_C_"..k], v)     
  end  
end

function maincanvas:openfromfile(fname, line)
  if not line  then line = 0 end
  local page = self:dopageexist(fname)
  if page ~= nil then
    self.pages:setactivepage(page)
  else
    page = scintilla:new()
    page:loadfile(fname)  
    local that = self    
    local name = fname:match("([^\\]+)$")
    self:setlexer(page)    
    function page:onfirstmodified()        
      local fname = page:filename():match("([^\\]+)$")
      that.pages:setlabel(page, fname.."*")      
    end    
    self.pages:add(page, name)   
  end  
  page:f(sci.SCI_GOTOLINE, line, 0)
end

function maincanvas:createnewpage()
  local page = scintilla:new()
  local that = self  
  function page:onfirstmodified()    
    local fname = self:filename()
    if fname ~= "" then    
      fname = fname:match("([^\\]+)$")      
      that.pages:setlabel(self, fname.."*")      
    else      
      that.pages:setlabel(self, "new"..self.pagecounter.."*")      
    end
  end
  self:setlexer(page)
  page.modified = false
  self.pages:add(page, "new"..self.newpagecounter)
  page.pagecounter = self.newpagecounter
  self.newpagecounter = self.newpagecounter + 1
end

function maincanvas:loadpage()
  self.fileopen:show()   
end

function maincanvas:savepage()
  local page = self.pages:activepage()
  if page then
    local fname = ""    
    if page:hasfile() then
      fname = page:filename()    
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
    elseif self.filesaveas:show() then      
      fname = self.filesaveas:filename()          
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
    end  
  end
end

function maincanvas:playplugin()
  local macidx = psycle.proxy:editmacidx()
  local mac = machine:new(macidx)
  if mac then
    self:savepage()
    mac:reload()
  end
end

function maincanvas:initfiletoolbar(x)  
  local t = toolbar:new(self):setpos(x, self.toolbartop)
  local inew = toolicon:new(t, self.picdir.."new.png", self.skin, 0xFFFFFF)
  local iopen = toolicon:new(t, self.picdir.."open.png", self.skin, 0xFFFFFF)
  local isave = toolicon:new(t, self.picdir.."save.png", self.skin, 0xFFFFFF)            
  local that = self    
  function inew:onclick() that:createnewpage() end  
  function iopen:onclick() that:loadpage() end
  function isave:onclick() that:savepage() end
  return t
end

function maincanvas:initplaytoolbar(x)  
  local t = toolbar:new(self):setpos(x, self.toolbartop)  
  local istart = toolicon:new(t, self.picdir.."play.png", self.skin, 0xFFFFFF)
  local that = self
  function istart:onclick() that:playplugin() end
  return t
end

function maincanvas.initskin()
  local skin = {}
  local cfg = config:new("PatternVisual")
  maincanvas.picdir = cfg:luapath().."\\canvastest\\icons\\"
  -- get skin from pattern view
  local keys = {"background", "row", "row4beat", "rowbeat", "separator",
                "font", "cursor", "selection", "playbar"}  
  skin = {colors={}}  
  for i=1, #keys do
    local keyname = keys[i]
    skin.colors[keyname] = cfg:get("pvc_"..keyname)
  end  
  return skin  
end

function maincanvas:onsize(width, height)       
  local splitterident = 2
  local spy = height-self.outputheight-self.splitterheight    
  self.splitter:setpos(0, spy, width, self.splitterheight)
  self.pages:setpos(0, self.tabbartop, width, spy-self.tabbartop-splitterident)  
  self.outputs:setpos(0, spy+self.splitterheight, width, self.outputheight)  
end
   
function maincanvas:ontimer()
end

return maincanvas

-- psycle plugineditor (c) 2015 by psycledelics
-- File: maincanvas.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()
-- local serpent = require("psycle.serpent")

local scintilla = require("psycle.ui.canvas.scintilla")
local sci = require("scintilladef")
local scilex = require("scilexerdef")

local fileopen = require("psycle.ui.fileopen")
local filesave = require("psycle.ui.filesave")
local settings = require("settings")
local style = require("psycle.ui.canvas.itemstyle")
local group = require("psycle.ui.canvas.group")
local canvas = require("psycle.ui.canvas")
local rect = require("psycle.ui.canvas.rect")
local toolbar = require("psycle.ui.canvas.toolbar")
local toolicon = require("psycle.ui.canvas.toolicon")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local splitter = require("psycle.ui.canvas.splitter")
local search = require("search")
local pluginexplorer = require("pluginexplorer")
local callstack = require("callstack")


local maincanvas = canvas:new()

function maincanvas:new()
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  return c
end

function maincanvas:init()
  self:setcolor(settings.canvas.background);
  
    
  self.search = search:new(self):hide()
  self.search:style():setalign(style.ALBOTTOM)
  self.search.dosearch:connect(maincanvas.onsearch, self)
  
  self:inittollbar()
  self.outputs = tabgroup:new(self):setheight(120)
  self.outputs:style():setalign(style.ALBOTTOM + style.ALFIXED)  
  self.output = scintilla:new()
  self.outputs:add(self.output, "Output")
  self.callstack = callstack:new(nil, self)  
  self.outputs:add(self.callstack, "Call stack")  
  self.splitter = splitter:new(self, splitter.HORZ)
  
  self.pluginexplorer = pluginexplorer:new(self):setwidth(201)  
  self.pluginexplorer:style():setalign(style.ALLEFT)     
  self.pluginexplorer:setfilepath("test")
  self.pluginexplorer.click:connect(maincanvas.onpluginexplorerclick, self)
  self.splitter2 = splitter:new(self, splitter.VERT)
  self.fileopen = fileopen:new()
  local that = self
  function self.fileopen:onok(fname) that:openfromfile(fname) end
  self.filesaveas = filesave:new()
  
  
  self.pages = tabgroup:new(self)   
  self.pages:style():setalign(style.ALCLIENT)  
  self.newpagecounter = 1
end

function maincanvas:inittollbar()
  self.tg = group:new(self)  
  self.tg:style():setalign(style.ALTOP)
  self:initselectplugintoolbar():style():setalign(style.ALLEFT):setmargin(4, 4, 4, 4)
  self:initfiletoolbar():style():setalign(style.ALLEFT)
  self:initplaytoolbar():style():setalign(style.ALLEFT)
end

function maincanvas:setoutputtext(text)
  self.output:addtext(text)  
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
  page:f(sci.SCI_SETKEYWORDS, settings.lexer.keywords, 0)
  page:f(sci.SCI_STYLESETSIZE, sci.STYLE_DEFAULT, settings.lexer.font.size)
  page:f(sci.SCI_STYLESETFONT, sci.STYLE_DEFAULT, settings.lexer.font.name) 
  page:f(sci.SCI_STYLECLEARALL, 0, 0)
  for k, v in pairs(settings.lexer.colors) do       
     local r = bit32.band(bit32.rshift(v, 16), 0xFF)
     local g = bit32.band(bit32.rshift(v, 8), 0xFF)
     local b = bit32.band(v, 0xFF)               
     v = bit32.bor(bit32.bor(r, bit32.lshift(g, 8)), bit32.lshift(b, 16))
     bit32.band(bit32.band(bit32.rshift(v, 16), bit32.rshift(v, 8)), bit32.rshift(v, 0))
     page:f(sci.SCI_STYLESETFORE, scilex["SCE_C_"..k], v)     
  end  
end

function maincanvas:createpage()  
  local page = scintilla:new()  
  self:setlexer(page)
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
  return page
end

function maincanvas:onkeydown(ev)
  if ev:ctrlkey() then
    if ev:keycode() == 70 then
      self:displaysearch()
      --self:setfocus(self.search.edit)
      ev:preventdefault()
    elseif ev:keycode() == 83 then
      self:savepage()    
      ev:preventdefault()
    end
  end  
end

function maincanvas:openfromfile(fname, line)
  if not line  then line = 0 end
  local page = self:dopageexist(fname)
  if page ~= nil then
    self.pages:setactivepage(page)
  else
    page = self:createpage()
    page:loadfile(fname)        
    local name = fname:match("([^\\]+)$")       
    self.pages:add(page, name)   
  end  
  page:gotoline(line - 1)
end

function maincanvas:setcallstack(trace)
  for i=1, #trace do
    self.callstack:add(trace[i])
  end 
  self.callstack:setdepth(1)
end

function maincanvas:createnewpage()
  local page = self:createpage()  
  self.pages:add(page, "new"..self.newpagecounter)
  page.pagecounter = self.newpagecounter
  self.newpagecounter = self.newpagecounter + 1
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

function maincanvas:initselectplugintoolbar(parent)
  local selectmachine = toolicon:new(self.tg):settext("no plugin loaded"):setsize(100, 20)
  local that = self
  function selectmachine:onclick()
    local name, path = psycle.selmachine()
    if name then   
      path = path:sub(1, -5).."\\"
      that.pluginexplorer:setfilepath(path)
      self:settext(name):fls()      
    end    
  end 
  return selectmachine
end

function maincanvas:initfiletoolbar()  
  local t = toolbar:new(self.tg)
  local inew = toolicon:new(t, settings.picdir.."new.png", 0xFFFFFF)
  local iopen = toolicon:new(t, settings.picdir.."open.png", 0xFFFFFF)
  local isave = toolicon:new(t, settings.picdir.."save.png", 0xFFFFFF)            
  local that = self    
  function inew:onclick() that:createnewpage() end  
  function iopen:onclick() that.fileopen:show() end
  function isave:onclick() that:savepage() end
  return t
end

function maincanvas:initplaytoolbar()  
  local t = toolbar:new(self.tg)
  local istart = toolicon:new(t, settings.picdir.."play.png", 0xFFFFFF)
  local that = self
  function istart:onclick() that:playplugin() end
  return t
end

function maincanvas:openinfo(info)
  local isfile = false
  if info.source:len() > 1 then
     local firstchar = info.source:sub(1, 1)     
     if firstchar == "@" then
       local fname = info.source:sub(2) 
       self:openfromfile(fname, info.line)
       isfile = true
     end
  end
  return isfile
end
 
function maincanvas:oncallstackclick(info)
  self:openinfo(info)
end

function maincanvas:onsearch(searchtext, dir, case, wholeword, regexp)
  local page = self.pages:activepage()
  if page then  
    local cpmin, cpmax = 0, 0
    if dir == search.DOWN then
      cpmin = page:selectionstart()
      cpmax = page:length()
      if page:hasselection() then 
       cpmin = cpmin + 1
      end
    else
      cpmin = page:selectionstart()
      cpmax = 0
      if page:hasselection() then 
        cpmax = cpmax - 1
      end
    end        
    page:setfindmatchcase(case)
    page:setfindwholeword(wholeword)
    page:setfindregexp(regexp)
    local line, cpselstart, cpselend = page:findtext(searchtext, cpmin, cpmax)
    if line ~= -1 then
      page:setsel(cpselstart, cpselend)
    end
  end
end

function maincanvas:displaysearch(ev)
  self.search:show()  
  self:updatealign()
  self.search:onfocus()
end

function maincanvas:onpluginexplorerclick(ev) 
   if ev.filename ~= "" then
     self:openfromfile(ev.path..ev.filename, 0)
   end
end

return maincanvas
-- info.lua

local systems = require("psycle.ui.systems")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local fontinfo = require("psycle.ui.fontinfo")
local item = require("psycle.ui.item")
local group = require("psycle.ui.group")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local text = require("psycle.ui.text")
local scintilla = require("psycle.ui.scintilla")
local cfg = require("psycle.config"):new("PatternVisual")

local info = group:new(parent)

info.windowtype = 96

info.infodir = cfg:luapath().."\\plugineditor\\info\\"

function info:new(parent)
  local m = group:new(parent)
  setmetatable(m, self)
  self.__index = self    
  m:init()
  systems:new():changewindowtype(info.windowtype, m)
  return m
end

function info:init()  
  self.title = text:new(self)
                   :setalign(item.ALTOP)
                   :setautosize(false, true)
                   :setjustify(text.CENTERJUSTIFY)                   
  self.memo = scintilla:new(self)
                       :setalign(item.ALCLIENT)
                       :hidelinenumbers()  
                       :hidebreakpoints()
                       :hidehorscrollbar()
  self:loadrootlink()                       
end

function info:loadrootlink()
  path = info.infodir.."infomain.lua"    
  local f = loadfile(path)
  if f then
    self.link = f()
  else
    self.link = nil
  end   
end

function info:setproperties(properties)  
  if properties.color then    
    self.memo:setforegroundcolor(properties.color:value())	    
  end
  if properties.backgroundcolor then    
	  self.memo:setbackgroundcolor(properties.backgroundcolor:value())
  end
  self.memo:styleclearall()  
  if properties.color then    
    self.memo:setcaretcolor(properties.color:value())
  end
  if properties.backgroundcolor then	  
    --self.memo:setcaretlinebackgroundcolor(properties.backgroundcolor:value())
  end
  self.memo:setselalpha(75)
  self.memo:setfontinfo(fontinfo:new():setsize(12):setfamily("consolas"))   
end

function info:jumpmain()  
  self:loadrootlink()
  self:builddisplay()
end

function info:jump(keycode)
  if self.link.keymap then
    local src = self.link.keymap[string.char(keycode)]
    if src then
      path = info.infodir .. src .. ".lua"    
      local f = loadfile(path)
      if f then
        self.link = f()
        self:builddisplay()
      else
        self.memo:addtext("Jump Error : info")  
      end    
    end
  end
  return self
end

function info:builddisplay()
  if self.link then
    self.memo:clearall()      
    local titlewithwhitespaces = self.link.title:gsub(".", "%1 "):sub(1,-2)
    self.title:settext("< < <      " .. titlewithwhitespaces .. "      > > >")    
    if self.link.display then
      for _, value  in pairs(self.link.display) do        
        self.memo:addtext(value.shortdesc .. "    " .. value.desc .. "\n")
      end
    end    
  end
end
  
return info

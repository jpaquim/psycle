-- info.lua

local signal = require("psycle.signal")
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
local fontinfo = require("psycle.ui.fontinfo")

local info = group:new(parent)

info.windowtype = 96
info.infodir = cfg:luapath().."\\plugineditor\\info\\"
info.timeout = 25

function info:new(parent)
  local m = group:new(parent)
  setmetatable(m, self)
  self.__index = self    
  m:init()
  systems:new():changewindowtype(info.windowtype, m)
  return m
end

function info:init()
  self.timercounter = 0
  self.cmd = signal:new()
  self.dojump = signal:new()
  self.strobe = 0
  self.activated = 0
  self.helplevel = 3
  self.title = text:new(self)
                   :setalign(item.ALTOP)
                   :setautosize(false, true)
                   :setjustify(text.CENTERJUSTIFY)                   
  self.memo = scintilla:new(self)
                       :setalign(item.ALCLIENT)
                       :hidelinenumbers()  
                       :hidebreakpoints()
                       :hidehorscrollbar()
                       :setfontinfo(fontinfo:new("Lucida Sans Typewriter", 8))                        
  self:loadrootlink()  
  if self.helplevel < 3 then                  
    self:hide()
  end
end

function info:loadrootlink()
  local path = info.infodir.."infomain.lua"    
  local f = loadfile(path)
  if f then
    self.link = f()
  else
    self.link = nil
  end
  if self.helplevel == 3 then    
    self:builddisplay()
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
  self.memo:setselalpha(75)
  self.memo:setfontinfo(fontinfo:new():setsize(12):setfamily("consolas"))   
end

function info:jumpmain()  
  self:loadrootlink()
  self:builddisplay()
  self.activated = 0
end

function info:onidle(canvas)
  if self.helplevel < 3 then
    if self.activated == 1 and self.strobe == 0 then    
      self.timercounter = self.timercounter + 1
      if self.timercounter > info.timeout then
        self.strobe = 1
        self.timercounter = 0
      end   
    end
    if self.strobe == 1 and self.activated == 1 and not self:visible() then    
      self:show();
      canvas:updatealign();    
    end  
  end
end

function info:setactivated()
  if self.helplevel > 0 then
    self.strobe = 0
    self.timercounter = 0
    self.activated = 1
  end
end

function info:deactivate()
  self.strobe = 0
  self.activated = 0
end  

function info:jump(keycode)
  if self.link.keymap then
    local src = self.link.keymap[string.char(keycode)]
    if src then
      if src:sub(1, 1) == "@" then
        self:jumpmain()
        self.cmd:emit(src:sub(2, -1));
        self:deactivate()        
      else
        path = info.infodir .. src .. ".lua"        
        local f, err = loadfile(path)
        if f then
          self.link = f()
          self:builddisplay()
          self.dojump:emit(self.link)        
        else          
          self.memo:addtext("Error\nJump" .. path .. "\n" .. err)  
        end        
        self:setactivated()        
      end
    end
  end
  return self
end

lpad = function(str, len, char)
  if char == nil then 
    char = ' '
  end
  return str .. string.rep(char, len - #str)
end

function info:builddisplay()
  if self.link then
    self.memo:clearall()      
    local titlewithwhitespaces = self.link.title:gsub(".", "%1 "):sub(1,-2)
    self.title:settext("< < <      " .. titlewithwhitespaces .. "      > > >")        
    if self.link.display then
      local lines = {""}
      local linecounter = 1
      local sectioncounter = 0
      for _, value  in pairs(self.link.display) do        
        if value.section then          
          lines[1] = lines[1] ..  value.section;          
          linecounter = 2
          sectioncounter = sectioncounter + 1
          lines[1] = lpad(lines[1], (sectioncounter)*30)
        else          
          if linecounter > #lines then
            lines[#lines + 1] = ""            
            lines[#lines] = lpad(lines[#lines], (sectioncounter - 1)*30)            
          end
          local space = ""
          if value.shortdesc  ~= nil and value.shortdesc ~= "" then
            space = lpad(space, 4)
          end
          lines[linecounter] = lines[linecounter] .. value.shortdesc .. space .. value.desc;          
          lines[linecounter] = lpad(lines[linecounter], sectioncounter*30)
          linecounter = linecounter + 1          
        end
      end    
      for i=1, #lines do
        self.memo:addtext(lines[i] .. "\n")
      end
    end
  end
end
      
return info

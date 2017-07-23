local player = require("psycle.player")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local image = require("psycle.ui.image")
local graphics = require("psycle.ui.graphics")

local rastergrid = { beatsperbar = 4, bgbeats = 4 }

function rastergrid:new(...)
  local m = {}
  setmetatable(m, self)
  self.__index = self    
  m:init(...)
  return m
end

function rastergrid:init()
  self.player = player:new()
end

function rastergrid:setview(view)
  self.view = view
end

function rastergrid:createbackground()  
  self.barbg = image:new():reset(self:bardimension())
  local g = graphics:new(self.barbg)
  self:drawbar(g)
  g:dispose()
end

function rastergrid:drawbar(g)
  if self.barbg then 
    local bardimension = self:bardimension()  
    g:setcolor(self.view.colors.rowcolor)
    g:fillrect(rect:new(point:new(), bardimension))
    self:drawkeylines(g, bardimension)     
    self:drawlines(g, bardimension)   
  end
end

function rastergrid:drawlines(g, bardimension)
  g:setcolor(self.view.colors.rastercolor) 
  local ticks = self.player:tpb() 
  for i=0, ticks * self.bgbeats do    
    local x = i * self.view:zoom():width() / ticks  
    g:drawline(point:new(x, 0), point:new(x, bardimension:height()))      
  end
end

function rastergrid:drawkeylines(g) 
end

function rastergrid:drawbars(g, screenrange, pattern, seqpos)
  self:drawimages(g, screenrange, pattern)
  self:drawbeatlines(g, screenrange, self:bardimension():height())
  self:drawbarendings(g, screenrange, seqpos)
  self:drawpatternend(g, self.view:zoom():beatwidth(pattern:numbeats())) 
end

function rastergrid:drawimages(g, screenrange, pattern)
  if self.barbg then
    local bgwidth = math.floor(self.bgbeats * self.view:zoom():width())
    local numbgs = pattern:numbeats() / self.bgbeats 
    local from = math.max(0, math.floor(screenrange.left / self.bgbeats))
    local to = math.min(numbgs, math.ceil(screenrange.right / self.bgbeats)) 
    for i=from, to do
      g:drawimage(self.barbg, point:new(i*bgwidth, 0 ))    
    end
  end
end

function rastergrid:drawbeatlines(g, screenrange, height)
  g:setcolor(self.view.colors.linebeatcolor)
  for i=0, screenrange.right do
    local x = i * self.view:zoom():width()    
    g:drawline(point:new(x, 0), point:new(x, height))      
  end
end

function rastergrid:drawbarendings(g, screenrange)
  local h = self:bardimension():height()
  g:setcolor(self.view.colors.linebarcolor)
  local start = 0 --math.floor(screenrange.left / self.beatsperbar) * self.beatsperbar
  for i=start, screenrange.right, self.beatsperbar do 
    local x = i*self.view:zoom():width() 
    g:drawline(point:new(x, 0), point:new(x, h))
  end
end

function rastergrid:drawpatternend(g, patternwidth, height)
  g:setcolor(self.view.colors.patternendcolor)  
  g:fillrect(rect:new(point:new(patternwidth - 5, 0), dimension:new(5, self:bardimension():height())))
end

function rastergrid:bardimension()
   return dimension:new(math.floor(self.beatsperbar * self.view:zoom():width()), 30)
end

function rastergrid:drawcursorbar(g, seqpos) 
  if self.view.cursor:seqpos() == seqpos then
    g:setcolor(self.view.colors.cursorbarcolor)
    self:rendercursorbar(g, self.view.cursor:position())   
  end             
end

function rastergrid:hittestselrect()
  return {}
end

function rastergrid:seteventcolor(g, event, seltrack, isplayed)   
  local isselectedtrack = event:track() == seltrack 
  local color = self.view.colors.eventdarkcolor 
  if isselectedtrack then             
   color = self.view.colors.eventcolor 
  end      
  if event:selected() then
   if self.view.selectionrect ~= nil then
     color = self.view.colors.eventblockcolor
   else
     color = self.view.colors.seleventcolor
   end
  end       
  if isplayed then              
   color = self.view.colors.eventplaycolor
  end
  g:setcolor(color)
end

function rastergrid:drawplaybar(g, playposition) 
  g:setcolor(self.view.colors.playbarcolor)
  self:renderplaybar(g, playposition)             
end

function rastergrid:renderplaybar(g, playposition)
  local x = playposition * self.view:zoom():width()
  g:drawline(point:new(x, - self.view:dy()), 
             point:new(x, self.view:position():height() - self.view:dy()))
end     

function rastergrid:rendercursorbar(g, position)
  local xpos = self.view:zoom():beatwidth(position)
  g:drawline(point:new(xpos, -self.view:dy()),
             point:new(xpos, self.view:position():height() - self.view:dy()))    
end        

function rastergrid:setclearcolor(g, playposition)
  if playposition % 1 == 0 then
    if playposition % self.player:tpb() == 0 then 
      g:setcolor(self.view.colors.line4beatcolor)
    else
      g:setcolor(self.view.colors.linebeatcolor)
    end
  else
    g:setcolor(self.view.colors.rastercolor)
  end
end

return rastergrid

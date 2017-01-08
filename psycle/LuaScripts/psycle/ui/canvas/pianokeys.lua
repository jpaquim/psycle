local item = require("psycle.ui.item")
local midi = require("psycle.midi")
local keymap = require("psycle.ui.canvas.pianokeymap")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")

local pianokeys = item:new()

-- wk = whitekey/ bk = black key
pianokeys.bk = {-1, 1, -1, 2, -1, -1, 3, -1, 4,  -1, 5, -1}
pianokeys.bkpos = {-0.1, 0, -0.3, 0, 0.3}
pianokeys.wkr = {0, 2, 4, 5, 7, 9, 11}

pianokeys.KEYDIRECTIONLEFT = 1
pianokeys.KEYDIRECTIONRIGHT = 2
pianokeys.KEYDIRECTIONTOP = 3
pianokeys.KEYDIRECTIONBOTTOM = 4

function pianokeys:new(parent)  
  local c = item:new(parent)  
  setmetatable(c, self)
  self.__index = self  
  c:init(shared)
  return c
end

function pianokeys:init()  
  self.keymap = keymap:new(12, 108)  
  self:initdefaultcolors()  
  self:viewpiano()  
  self:setevheight(10) 
  self:setkeydirection(pianokeys.KEYDIRECTIONTOP)  
end

function pianokeys:initdefaultcolors()
  self.keycolors = {
    whiteup = 0xFFFFFFFF,
    whitedown = 0xFF000000,
    blackup = 0xFF000000,
    blackdown = 0xFF0000AA,
    whiteseparator = 0xFF00000,
    blackseparator = 0xFF00000
  }
end

function pianokeys:viewlist() 
  self.viewpiano_ = false self:fls()
end

function pianokeys:viewpiano() 
  self.viewpiano_ = true self:fls()
end

function pianokeys:setkeydirection(keydirection)
  self.keydirection_ = keydirection
end

function pianokeys:setevheight(height)  
  self.evheight = height
  self.octpx = (12.0*height) / 7.0;  
  self.bksmaller = math.floor(self.octpx*0.25)    
  self.bkheight = self.octpx - 2*self.bksmaller  
  self.height = (self.keymap:range() + 1)*height  
end

function pianokeys:draw(g)       
  local d = {    
    first = self.keymap:first(),
    last = self.keymap:last(),
	  eh = self.evheight, -- self.shared.evheight:val(),
    keymap = self.keymap    
  }
  local x, y, w, h = 0, 0, 50, 500 -- self:pos() -- self:drawregion():bounds()  
  d.from = d.first -- math.max(math.min(d.last - math.floor((h+y) / d.eh), d.last), d.first)
  d.to = d.last -- math.max(d.last - math.floor((y) / d.eh), d.first)  
  if self.viewpiano_ then
    self:drawwhite(g, d)
    self:drawblack(g, d)
  else  
    self:drawkeymap(g, d)
  end
end

function pianokeys:setkeycolor(g, iswk, isdown)  
  if iswk then
    if (isdown) then
      g:setcolor(self.keycolors.whitedown)
    else
      g:setcolor(self.keycolors.whiteup)
    end
  else
    if (isdown) then
      g:setcolor(self.keycolors.blackdown)
    else
      g:setcolor(self.keycolors.blackup)
    end
  end      
end

function pianokeys:drawkeymap(g, d) 
  local x, y, w, h = self:pos()
  for note=d.from, d.to do               
    local yp = (d.last - note)*d.eh
    local isbk = pianokeys.bk[note%12+1] ~= -1    
    self:setkeycolor(g, not isbk, self.keydown == note)        
    g:fillrect(rect:new(point:new(0, yp), dimension:new(w, d.eh)))
    self:setkeycolor(g, isbk, self.keydown == note)      
    g:setcolor(self.keycolors.whiteup)
    g:drawstring(d.keymap:name(note), 0, yp)
  end   
end

function pianokeys:drawwhite(g, d)
  local w, h = self:position():width(), self:position():height()
  g:setcolor(self.keycolors.whiteup):fillrect(rect:new(point:new(), dimension:new(w, self.height)))  
  for note=d.from - d.from%12, d.to, 12 do
    local yos = (d.last - note + 1)*d.eh	
	  g:setcolor(self.keycolors.whiteseparator)
    for i=0, 6 do  -- loop octave separators for white keys
      local yp = yos - i*self.octpx
      if self.keydirection_ == pianokeys.KEYDIRECTIONLEFT or self.keydirection_ == pianokeys.KEYDIRECTIONRIGHT then
        g:drawline(point:new(0, yp), point:new(w, yp));
      elseif self.keydirection_ == pianokeys.KEYDIRECTIONTOP or self.keydirection_ == pianokeys.KEYDIRECTIONBOTTOM then        
        g:drawline(point:new(yp, 0), point:new(yp, h));
      end
      if self.keydown == note+pianokeys.wkr[i+1] then        
        g:setcolor(self.keycolors.whitedown)
        if self.keydirection_ == pianokeys.KEYDIRECTIONLEFT or self.keydirection_ == pianokeys.KEYDIRECTIONRIGHT then
          g:fillrect(point:new(0, yp-self.octpx), dimension:new(w, self.octpx))
        elseif self.keydirection_ == pianokeys.KEYDIRECTIONTOP or self.keydirection_ == pianokeys.KEYDIRECTIONBOTTOM then
          g:fillrect(point:new(yp-self.octpx, 0), dimension:new(self.octpx, 0))
        end
        g:setcolor(self.keycolors.whiteup)
      end      
	  end
    psycle.output("a"..d.keymap:name(note))
    g:setcolor(self.keycolors.blackup)
    if self.keydirection_ == pianokeys.KEYDIRECTIONLEFT then 
	    --g:drawstring(d.keymap:name(note), point:new(yos - d.eh))
    elseif self.keydirection_ == pianokeys.KEYDIRECTIONRIGHT then
     --local tw, th = g:textsize(d.keymap:name(note))
     --g:drawstring(d.keymap:name(note), w - tw, yos-d.eh)
    elseif self.keydirection_ == pianokeys.KEYDIRECTIONTOP then 
     --local tw, th = g:textsize(d.keymap:name(note))
     --g:drawstring(d.keymap:name(note), yos-d.eh, 0)    
    elseif self.keydirection_ == pianokeys.KEYDIRECTIONBOTTOM then
     --local th = g:textdimension(d.keymap:name(note)):height()
     --g:drawstring(d.keymap:name(note), yos-d.eh, h - th)
    end
  end    
end

function pianokeys:drawblack(g, d)
  local w, h = self:position():width(), self:position():height()
  for note=d.from, d.to do     
    local i = pianokeys.bk[note%12 + 1]
	  if (i~=-1) then      
	    local yp = (d.last-note)*d.eh --+ pianokeys.bkpos[i]*d.eh
	    g:setcolor(self.keycolors.blackseparator)
      --g:drawrect(rect:new(point:new(0, yp), dimension:new(self.bkwidth, self.bkheight)))
      self:setkeycolor(g, false, self.keydown == note)
      if self.keydirection_ == pianokeys.KEYDIRECTIONLEFT then
        g:fillrect(rect:new(point:new(0, yp), dimension:new(w * 0.75, self.bkheight)))
      elseif self.keydirection_ == pianokeys.KEYDIRECTIONRIGHT then
        g:fillrect(rect:new(point:new(w*0.25, yp), dimension:new(w*0.75, self.bkheight)))      
      elseif self.keydirection_ == pianokeys.KEYDIRECTIONTOP then 
        g:fillrect(rect:new(point:new(yp, 0), dimension:new(self.bkheight, h * 0.75)))
      elseif self.keydirection_ == pianokeys.KEYDIRECTIONBOTTOM then
        g:fillrect(rect:new(point:new(yp, h*0.25), dimension:new(self.bkheight, h*0.75)))      
      end
    end
  end    
end

function pianokeys:onnoteon(val)
  self.keydown = val
  self:fls()  
end

function pianokeys:onnoteoff()
  self.keydown = nil
  self:fls()
end

function pianokeys:setwhitekeyupcolor(color)
  self.keycolors.whiteup = color
end  

function pianokeys:setwhitekeydowncolor(color)
  self.keycolors.whitedown = color
end

function pianokeys:setblackkeyupcolor(color)
  self.keycolors.blackup = color
end  

function pianokeys:setblackkeydowncolor(color)
  self.keycolors.blackdown = color
end
    
function pianokeys:setwhitekeyseparator(color)
  self.keycolors.whiteseparator = color
end

function pianokeys:setblackkeyseparator(color)
  self.keycolors.blackseparator = color
end

return pianokeys
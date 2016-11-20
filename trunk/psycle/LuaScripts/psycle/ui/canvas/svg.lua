local slaxml = require 'psycle.slaxml'

local svg = {}

local endl = '\n'

local point = { 
  x, y  = 0, 0,
  new = function(self, o)
    o = o or {}   -- create object if user does not provide one
    setmetatable(o, self)
    self.__index = self
    self.x, self.y = 0, 0
    return o
  end,  
  set = function(self, x, y)  self.x, self.y = x, y end,
  offset = function(self, dx, dy)
    self.x, self.y  = self.x + dx, self.y + dy
  end
}

local pathcommands = {
  new = function(self)
    local c = {}
    setmetatable(c, self)  
    self.__index = self    
    c:init()    
    return c
  end,  
  init = function(self) 
    self.scalex, self.scaley  = 1, 1
    self.lastpoint, self.lastctrlpoint = point:new(), point:new()
    self.fill = nil
  end,  
  M  = { numargs = 2,
    drawcode = function(self, args)
      self.lastpoint:set(args[1], args[2])
      return "g:moveto(" .. self:createpointstr(args[1], args[2]) .. ")" .. endl        
    end
  },  
  L  = { numargs = 2,  
    drawcode = function(self, args)
      self.lastpoint:set(args[1], args[2])
      local ptstr = self:createpointstr(args[1], args[2])
      return "g:lineto(" .. ptstr .. ")" .. endl
    end  
  },
  H  = { numargs = 1,
    drawcode = function(self, args)
     self.lastpoint.x = args[1]        
     local ptstr = self:createpointstr(self.lastpoint.x, self.lastpoint.y)
     return "g:lineto(" .. ptstr .. ")" .. endl
   end  
  },         
  V  = { numargs = 1,
    drawcode = function(self, args)
      self.lastpoint.y = self.lastpoint.y + args[1]
      local ptstr = self:createpointstr(host.lastpoint.x, host.lastpoint.y)
      return "g:lineto(" .. ptstr .. ")" .. endl
    end    
  },             
  C  = { numargs = 6,
    drawcode = function(self, args)            
      self.lastpoint:set(args[5], args[6])
      self.lastctrlpoint:set(args[3], args[4])
      return "g:curveto(" .. self:createpointstr(args[1], args[2]) .. ", " 
                          .. self:createpointstr(args[3], args[4]) .. "," 
                          .. self:createpointstr(args[5], args[6]) .. ")" .. endl                   
    end
  },
  S  = { numargs = 4,    
    drawcode = function(self, args)       
      local ctrlpt1x = 2*self.lastpoint.x - self.lastctrlpoint.x
      local ctrlpt1y = 2*self.lastpoint.y - self.lastctrlpoint.y        
      self.lastctrlpoint:set(args[1], args[2])
      local ptstr1 = self:createpointstr(ctrlpt1x, ctrlpt1y)
      local ptstr2 = self:createpointstr(args[1], args[2])
      local ptstr3 = self:createpointstr(args[3], args[4])        
      self.lastpoint:set(args[3], args[4])
      return "g:curveto(" .. ptstr1 .. ", " .. ptstr2 .. "," .. ptstr3.. ")" .. endl      
    end
  },
  Z  = { numargs = 0,     
    drawcode = function(self, args)
      result = "g:endpath()" .. endl
      if self.fill and self.fill == "none" then
        result = result .. "g:drawpath()" .. endl
      else
        result = result .. "g:fillpath()" .. endl
      end
      return result
    end
  },  
  l  = { numargs = 2,
    drawcode = function(self, args)
       self.lastpoint:offset(args[1], args[2])
       local ptstr = self:createpointstr(self.lastpoint.x, self.lastpoint.y)
       return "g:lineto(" .. ptstr .. ")" .. endl        
     end  
  },
  c  = { numargs = 6,
    drawcode = function(self, args)
      host.lastctrlpoint = {} 
      local ptstr1 = self:createpointstr(args[1] + self.lastpoint.x, args[2] + self.lastpoint.y)
      local ptstr2 = self:createpointstr(args[3] + self.lastpoint.x, args[4] + self.lastpoint.y)
      local ptstr3 = self:createpointstr(args[5] + self.lastpoint.x, args[6] + self.lastpoint.y)        
      self.lastctrlpoint:offset(args[3], args[4])
      self.lastpoint:offset(args[5], args[6])
      return "g:curveto(" .. ptstr1 .. ", " .. ptstr2 .. "," .. ptstr3.. ")" .. endl  
    end
  },
  z  = { numargs = 0,
    drawcode = function(self, args)
      result = "g:endpath()" .. endl
      if self.fill and self.fill == "none" then
        result = result .. "g:drawpath()" .. endl
      else
        result = result .. "g:fillpath()" .. endl
      end
      return result
    end  
  },
  h  = { numargs = 1,
   drawcode = function(self, args)
     self.lastpoint.x = self.lastpoint.x + args[1]        
     local ptstr = self:createpointstr(self.lastpoint.x, self.lastpoint.y)
     return "g:lineto(" .. ptstr .. ")" .. endl
   end  
  },         
  v  = { numargs = 1,
    drawcode = function(self, args)
      self.lastpoint.y = args[2]
      local ptstr = self:createpointstr(self.lastpoint.x, args[1])
      return "g:lineto(" .. ptstr .. ")" .. endl
    end  
  },
  s  = { numargs = 4,    
    drawcode = function(self, args)      
      local ctrlpt1x = 2*self.lastpoint.x - self.lastctrlpoint.x
      local ctrlpt1y = 2*self.lastpoint.y - self.lastctrlpoint.y        
      self.lastctrlpoint:set(args[1] + self.lastpoint.x, args[2] + self.lastpoint.y)
      local ptstr1 = self:createpointstr(ctrlpt1x, ctrlpt1y)
      local ptstr2 = self:createpointstr(args[1] + self.lastpoint.x, args[2] + self.lastpoint.y)
      local ptstr3 = self:createpointstr(args[3] + self.lastpoint.x, args[4] + self.lastpoint.y)
      self.lastpoint:offset(args[3], args[4])      
      return "g:curveto(" .. ptstr1 .. ", " .. ptstr2 .. "," .. ptstr3.. ")" .. endl
    end  
  },
  createpointstr = function(self, arg1, arg2)  
    local ptstr = "point:new(" .. (arg1*self.scalex) .. "," 
                               .. (arg2*self.scaley)  ..")"
    return ptstr
  end
}

pathcommands.m = pathcommands.M
pathcommands.z = pathcommands.Z

function svg:new(filename)
  local m = {}
  setmetatable(m, self)
  self.__index = self  
  m:init(filename)
  return m
end

function svg:init(filename) 
  self.pathcommands = pathcommands:new()
  self.circle = { cx, cy, r = 0, 0, 0 }
  self.currentelement = ""    
  self.mode = ""  
  self.viewport = {x = 0, y = 0, width = 100, height = 100}
  self.viewbox = {x = 0, y = 0, width = 100, height = 100}  
  local that = self
  local myxml = io.open(filename):read('*all')
  -- Specify as many/few of these as you like
  self.parser = slaxml:parser{
    startElement = function(name,nsURI,nsPrefix)      
      that.currentelement = name      
    end, -- When "<foo" or <x:foo is seen
    attribute    = function(name,value,nsURI,nsPrefix)
      if that.currentelement == "svg" then
        if name == "width" then
          that.viewport.width = value
        elseif name == "height" then
          that.viewport.height = value
        elseif name == "viewBox" then                   
          local delim = {",", " "}  
          local p = "[^"..table.concat(delim).."]+"
          local i = 0          
          for w in value:gmatch(p) do         
            i = i + 1        
            if i == 1 then that.viewbox.x = w         
            elseif i == 2 then that.viewbox.y = w
            elseif i == 3 then that.viewbox.width = w
            elseif i == 4 then that.viewbox.height = w end       
          end     
        end          
      elseif that.currentelement == "path" then        
        if name == "d" then
          that.d = value          
        elseif name == "fill" then          
          that.pathcommands.fill = value
        end
      elseif that.currentelement == "circle" then
        if name == "cx" then
          that.circle.cx = value
        elseif name == "cy" then
          that.circle.cy = value
        elseif name == "r" then
          that.circle.r = value     
        elseif name == "fill" then          
          that.circle.fill = value
        elseif name == "stroke" then          
          that.circle.stroke = value
        end
      end
    end, -- attribute found on current element
    closeElement = function(name,nsURI)
      if name == "path" then      
        that.pathcommands.scalex = that.viewport.width / that.viewbox.width
        that.pathcommands.scaley = that.viewport.height / that.viewbox.height        
        that:transformpath(that.d)        
      elseif name == "circle" then
        local scalex = that.viewport.width / that.viewbox.width
        local scaley = that.viewport.height / that.viewbox.height        
        local ptstr = "point:new(" .. (that.circle.cx*scalex) .. "," 
                                   .. (that.circle.cy*scaley)  ..")"       
        if that.circle.fill and that.circle.fill ~= "none" then                                             
          that.drawcode = that.drawcode .. "g:setcolor(0xFF" .. that.circle.fill:sub(2) .. ")" .. endl
        end  
        if not that.circle.fill or that.circle.fill ~= "none" then
          that.drawcode = that.drawcode .. "g:fillcircle(" .. ptstr .. "," .. that.circle.r*scalex .. ")" .. endl
        end
        if that.circle.stroke and that.circle.stroke ~= "none" then                                             
          that.drawcode = that.drawcode .. "g:setcolor(0xFF" .. that.circle.stroke:sub(2) .. ")" .. endl
          that.drawcode = that.drawcode .. "g:drawcircle(" .. ptstr .. "," .. that.circle.r*scalex .. ")" .. endl          
        end          
      end
    end, -- When "</foo>" or </x:foo> or "/>" is seen
    text         = function(text)                      end, -- text and CDATA nodes
    comment      = function(content)                   end, -- comments
    pi           = function(target,content)            end, -- processing instructions e.g. "<?yes mon?>"
  }
  -- Ignore whitespace-only text nodes and strip leading/trailing whitespace from text
  -- (does not strip leading/trailing whitespace from CDATA)  
  self.drawcode = 'local point = require("psycle.ui.point")' .. endl ..
                  'local rect = require("psycle.ui.rect")' .. endl .. 
                  'local g = ...' .. endl
  self.parser:parse(myxml,{stripWhitespace=true})  
  psycle.output(self.drawcode)
  self.drawfunc = load(self.drawcode)     
end

function svg:transformpath(path)  
  if self.pathcommands.fill and self.pathcommands.fill:find("#") then    
    self.drawcode = self.drawcode .. "g:setcolor(0xFF" .. self.pathcommands.fill:sub(2) .. ")" .. endl
  end
  self.drawcode = self.drawcode .. "g:beginpath()" .. endl
  self:parsepath(path)      
end

function svg:parsepath(str)  
  local delimiters = "[MLSHVCZmlsczhv,%- \n]"
  local cmdcharclass = "[MLSHVCZsmhvlcz]"
  local st = 1
  for i = 1, #str do
    local c = str:sub(i,i)      
    if c:find(delimiters) then              
      local token = str:sub(st, i - 1)      
      if token ~= "" then          
        self:collectvalue(token)
        self:processcmd()
      end  
      if (c:find(cmdcharclass)) then         
        self:checkcmd(c)           
      end
      if c == "-" then
        st = i
      else
        st = i + 1      
      end
    end    
   end
end

function svg:checkcmd(token)  
  self.args = {}   
  self.command = pathcommands[token]            
  if self.command.numargs == 0 then    
    self:processcmd()     
  end  
end

function svg:collectvalue(token)
  if #self.args < self.command.numargs then
    self.args[#self.args + 1] = token
  end   
end  

function svg:processcmd()         
  if #self.args == self.command.numargs then
    self.drawcode = self.drawcode .. self.command.drawcode(self.pathcommands, self.args)    
  end
end

function svg:draw(g)  
  self.drawfunc(g)
end

return svg
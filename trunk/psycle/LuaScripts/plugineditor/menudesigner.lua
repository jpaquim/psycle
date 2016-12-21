-- psycle plugineditor (c) 2015 by psycledelics
-- File: menudesigner.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local canvas = require("psycle.ui.canvas")
local group = require("psycle.ui.group")
local item = require("psycle.ui.item")
local text = require("psycle.ui.text")
local ornamentfactory = require("psycle.ui.ornamentfactory"):new()
local tree = require("psycle.ui.treeview")
local node = require("psycle.node")
local button = require("psycle.ui.button")
local templateparser = require("templateparser")
local cfg = require("psycle.config"):new("PatternVisual")

local menudesigner = canvas:new()

function menudesigner:new(parent)
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()  
  return c
end

function menudesigner:init()  
  self:addornament(ornamentfactory:createfill(0xFF528A68))  
  self.helptext = text:new(self)
                      :setautosize(false, true)
                      :setalign(item.ALTOP)
                      :settext("CTRL + RIGHT create submenu") 
                      :addornament(ornamentfactory:createfill(0xFF528A68))    
  self.applybutton = button:new(self):settext("Apply"):setautosize(false, false):setalign(item.ALBOTTOM)
                           :setposition(rect:new(point:new(), dimension:new(0, 20)))  
  local that = self  
  function self.applybutton:onclick()    
    that:makecode()
  end  
  self.tree = tree:new(self):setautosize(false, false):setalign(item.ALCLIENT)
  self.tree:showbuttons():showlines()
  self.tree:setbackgroundcolor(0xFF2F2F2F)
  self.tree:setcolor(0xFFFFFF)
  function self.tree:onedited(node, text)
    self:selected():settext(text)    
  end
  self.rootnode = node:new() 
  local node = node:new():settext("PluginDesigner")
  self.rootnode:add(node)
  local emptynode = node:new():settext("empty")
  node:add(emptynode)  
  self.tree:setrootnode(self.rootnode)
  self.tree:selectnode(emptynode)  
  function self.tree:onkeydown(ev)    
    if ev:ctrlkey() then
      if ev:keycode() == ev.RIGHT then
        local node = self:selected()
        if node:size() == 0 then      
          local newnode = node:new():settext("newnode")          
          local node = self:selected()      
          node:add(newnode)
          that.tree:selectnode(newnode)
        else        
          that.tree:selectnode(node:at(1))
        end                                
        ev:preventdefault()
      end    
    elseif ev:keycode() == ev.RETURN then            
      local newnode = node:new():settext("newnode")      
      local node = self:selected()        
      if (node:level() == 2 and node:size() == 0) then
        node:add(newnode)
      else      
        node:parent():insertafter(newnode, node)
      end
      that.tree:selectnode(newnode)
               :editnode(newnode)
      ev:preventdefault()    
    elseif ev:keycode() == ev.DELETE then            
       that.tree:selected():parent():remove(that.tree:selected())
    end
  end    
end

function menudesigner:setmenu(rootnode)
  self.rootnode = self:copynode(rootnode)
  self.tree:setrootnode(self.rootnode)  
  self.tree:selectnode(self.rootnode:at(1):at(1))
end

function menudesigner:copynode(srcnode)
  local newnode = node:new():settext(srcnode:text())
  function newnode:onedited(node, text)
    node:settext(text)
    psycle.output("onedited")
  end
  local size = srcnode:size()   
  for i=1, size do    
    newnode:add(self:copynode(srcnode:at(i)))
  end
  return newnode
end

function menudesigner:generatemenucode(node)  
  local out = "  menu.node"..self.idx.." = node:new():settext(\""..node:text().."\")\n"
  local idxparent = self.idx
  self.idx = self.idx + 1
  local size = node:size()    
  for i=1, size do
    local subnode = node:at(i)
    local nodeidx = self.idx    
    out = out..self:generatemenucode(subnode).."\n"
    out = out .. "  menu.node"..idxparent..":add(menu.node"..nodeidx..")\n"    
  end
  return out
end

function menudesigner:makecode()
  if self.pluginname then
    self.idx = 1
    local output = self:generatemenucode(self.rootnode)
    local env = {
      nodes = output
    }     
    templateparser.work(cfg:luapath().."\\plugineditor\\mainmenu.lu$",
                        cfg:luapath().."\\"..self.pluginname.."\\mainmenu1.lua",
                        env)
  end
end                      

return menudesigner
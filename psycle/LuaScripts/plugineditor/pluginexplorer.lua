-- psycle plugineditor (c) 2015 by psycledelics
-- File: pluginexplorer.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local group = require("psycle.ui.canvas.group")
local tree = require("psycle.ui.canvas.treeview"):new()
local node = require("psycle.node")
local machine = require("psycle.machine"):new()
local settings = require("settings")
local signal = require("psycle.signal")
local serpent = require("psycle.serpent")
local filehelper = require("psycle.file")
local image = require("psycle.ui.image")
local images = require("psycle.ui.images")
local settings = require("settings")

local pluginexplorer = tree:new()


function node_iter(node)
  local i = {-1}
  local level_ = 1
  local n = node 
  return 
  { 
    level = function() return level_ - 1 end,
    next = function()
            i[level_] = i[level_] + 1
            if i[level_] == 0 then return n end
            if i[level_] <= n:size() then
              local child = n:at(i[level_])  
              if child:size() > 0 then
                level_ = level_ + 1
                i[level_] = 0
                n = child                
              end                 
              return child
            elseif level_ > 1 then
              level_ = level_ - 1              
              i[level_] = i[level_] + 1
              if i[level_] <= n:parent():size() then
                n = n:parent()
                return n:at(i[level_])
              end
            end
          end                                     
  }
end
    
function pluginexplorer:new(parent)  
  local c = tree:new(parent)  
  setmetatable(c, self)
  self.__index = self
  c:init()
  return c
end

function pluginexplorer:init()
  self.click = signal:new() 
  self:setpos(0, 0, 100, 0)
  self:showlines():showbuttons()
  self.text = image:new():load(settings.picdir.."gnome-mime-text.png")
  self.textlua = image:new():load(settings.picdir.."gnome-mime-text-x-lua.png")
  self.folder = image:new():load(settings.picdir.."folder.png")  
  self.images = images:new()
  self.images:add(self.text):add(self.folder):add(self.textlua)   
  self:setimages(self.images)
end

function pluginexplorer:dirLookup(directory)
  local filetree = filehelper.filetree(directory)
  if filetree then
    self.rootnode = filetree         
    for element in node_iter(filetree).next do            
      if element.isdirectory then
        element:setimageindex(1):setselectedimageindex(1)
      elseif element.extension == ".lua" then
        element:setimageindex(2):setselectedimageindex(2)
      end      
    end
  else
    self.rootnode = node:new()  
  end         
  self:setrootnode(self.rootnode)
end

function getpath(str,sep)
    sep=sep or'/'
    return str:match("(.*"..sep..")")
end

function scriptpath()
  local str = debug.getinfo(2, "S").source:sub(2)  
  return getpath(str, "\\")
end

function pluginexplorer:setfilepath(path) 
  --self:clear()
  self:dirLookup(path) 
end

function pluginexplorer:onclick(node)
  if not node.isdirectory then  
    local ev = { 
      sender = self, 
      path = node.path,
      filename = node.filename         
    }
    self.click:emit(ev)
  end
end

function pluginexplorer:onrightclick(node)
  psycle.alert("right click")
end

function pluginexplorer:onedited(node, text)
  node:settext(text)
end

return pluginexplorer
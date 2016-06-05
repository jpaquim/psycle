-- objectinspector.lua

local group = require("psycle.ui.canvas.group")
local listview = require("psycle.ui.canvas.listview")
local text = require("psycle.ui.canvas.text")
local item = require("psycle.ui.canvas.item")
local node = require("psycle.node")

local objectinspector = listview:new()

function objectinspector:new(parent)
  local m = listview:new(parent)
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function objectinspector:init()
  self:setautosize(false, false)
  self:viewreport()
  self:addcolumn("Name", 100)
  self:addcolumn("Value", 100)
  
  self.rootnode = node:new()
  local nameproperty = node:new():settext("name")
  self.rootnode:add(nameproperty)  
  self:setrootnode(self.rootnode)
end

function objectinspector:setobject(item)  
  self.rootnode = node:new()  
  if item.published then
    for k, v in pairs(item.published) do    
      local node = node:new():settext(k.."")
      self.rootnode:add(node)  
    end    
  end
  self:setrootnode(self.rootnode)
end

return objectinspector
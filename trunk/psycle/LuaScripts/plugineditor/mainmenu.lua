local node = require("psycle.node")
local image = require("psycle.ui.image")
local settings = require("settings")

local mainmenu = {}

function mainmenu:new()
  local m = canvas:new()  
  setmetatable(m, self)
  self.__index = self  
  m:init()
  return m
end

function mainmenu.menus()
   local menus = {}
   menus.node = node:new():settext("Plugineditor")   
   menus.filemenu = node:new():settext("File")
   menus.node:add(menus.filemenu)
   menus.newmenu = node:new():settext("New")
   menus.filemenu:add(menus.newmenu)
   menus.img_new = image:new():load(settings.picdir.."new.png")
   menus.newfile = node:new():settext("New File"):setimage(menus.img_new)
   menus.newmenu:add(menus.newfile)
   menus.openfile = node:new():settext("Open")
   menus.filemenu:add(menus.openfile) 
   -- tools
   menus.toolmenu = node:new():settext("Tools")
   menus.node:add(menus.toolmenu)
   menus.menudesigner = node:new():settext("Menu Designer")
   menus.toolmenu:add(menus.menudesigner)
   menus.separator = node:new():settext("-")
   menus.node:add(menus.separator)
   return menus
end

return mainmenu
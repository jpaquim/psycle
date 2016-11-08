-- psycle pluginexplorer (c) 2015 by psycledelics
-- File: settings.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- get skin from pattern view
local cfg = require("psycle.config"):new("PatternVisual")
local keys = {"background", "row", "row4beat", "rowbeat", "separator",
                "font", "cursor", "selection", "playbar"}    

local settings = {  
  picdir = cfg:luapath().."\\psycle\\ui\\icons\\",
  sci = {    
    lexer = {     
      font = {name = "consolas", size = 12} 
    },
    default = {foreground =  0xB0C8B1, background = 0xFF232323},    
    colors = {  
      STYLE_LINENUMBER = {foreground =  0x939393, background = 0x232323},
      STYLE_INDENTGUIDE = {foreground =  0x939393, background = -1}               
    },
    misc = {
      SCI_SETCARETFORE = {val1 = 0xFF939393, val2 = 0,  iscolor = true}, 
      SCI_SETSELALPHA = {val1=75, val2 = 0},
      SCI_SETSELFORE = {val1 = 1, val2 = 0xFF232323},
      --SCI_STYLESETBACK = {val1 = 32, val2 = 0x232323}
        
    }
  },    
  canvas = { 
    colors = {foreground = 0xFFCACACA, background = cfg:get("pvc_background")}
  } 
}
     
return settings
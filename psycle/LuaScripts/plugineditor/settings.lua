-- psycle pluginlexer (c) 2015 by psycledelics
-- File: settings.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local config = require("psycle.config")

local settings = { lexer = { colors = { }, font = { } }, canvas = { } }

local pv = { }
local cfg = config:new("PatternVisual")

settings.picdir = cfg:luapath().."\\psycle\\ui\\icons\\"
-- get skin from pattern view
local keys = {"background", "row", "row4beat", "rowbeat", "separator",
                "font", "cursor", "selection", "playbar"}    

settings.canvas.background = cfg:get("pvc_background")

settings.lexer.font.name = "consolas"
settings.lexer.font.size = 12
settings.lexer.colors.DEFAULT = 0x000000
settings.lexer.colors.COMMENT = 0x008000
settings.lexer.colors.COMMENTLINE = 0x008000
settings.lexer.colors.COMMENTDOC = 0x008000
settings.lexer.colors.COMMENTLINEDOC = 0x008000
settings.lexer.colors.COMMENTDOCKEYWORD = 0x008000
settings.lexer.colors.COMMENTDOCKEYWORDERROR = 0x008000
settings.lexer.colors.NUMBER = 0x008080
settings.lexer.colors.WORD = 0x000080
settings.lexer.colors.STRING = 0x800000
settings.lexer.colors.IDENTIFIER = 0x000000
settings.lexer.colors.PREPROCESSOR = 0x800000
settings.lexer.colors.OPERATOR = 0x808000

settings.lexer.keywords =   
     "and break do else elseif "..
     "end false for function if "..
     "in local nil not or "..
     "repeat return then true until while"

     
return settings
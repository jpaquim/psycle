-- psycle plugineditor (c) 2015 by psycledelics
-- File: settings.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local settings = { editor = { colors = { }, font = { }  } }

settings.editor.font.name = "consolas"
settings.editor.font.size = 12
settings.editor.colors.DEFAULT = 0x000000
settings.editor.colors.COMMENT = 0x008000
settings.editor.colors.COMMENTLINE = 0x008000
settings.editor.colors.COMMENTDOC = 0x008000
settings.editor.colors.COMMENTLINEDOC = 0x008000
settings.editor.colors.COMMENTDOCKEYWORD = 0x008000
settings.editor.colors.COMMENTDOCKEYWORDERROR = 0x008000
settings.editor.colors.NUMBER = 0x008080
settings.editor.colors.WORD = 0x000080
settings.editor.colors.STRING = 0x800000
settings.editor.colors.IDENTIFIER = 0x000000
settings.editor.colors.PREPROCESSOR = 0x800000
settings.editor.colors.OPERATOR = 0x808000

settings.editor.keywords =   
     "and break do else elseif "..
     "end false for function if "..
     "in local nil not or "..
     "repeat return then true until while"

return settings
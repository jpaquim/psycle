-- psycle plugineditor (c) 2016 by psycledelics
-- File: templateparser.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

local templateparser = {}

function templateparser.prep(file, env)
  local output = ""
  for line in file:lines() do
    if string.find(line, "^#") then       
    else
      output = output..templateparser.replaceline(line, env)
    end    
  end
  return output
end

function templateparser.replaceline(line, env)
  local output = ""
  local last = 1
  for text, expr, index in string.gmatch(line, "(.-)$(%b())()") do 
    last = index
    if text ~= "" then
      output = output..text
    end        
    local trimmedexp = expr:sub(2, -2)      
    local realvalue = env[trimmedexp]
    if not realvalue then
       output = output .. "#ERROR with ".."$("..expr..") and "..trimmedexp.."#ERROREND"
    else        
      output = output..realvalue
    end
  end    
  output = output..string.sub(line, last) .. "\n"      
  return output
end

function templateparser.work(templatepath, outputpath, env)   
  templateparser.write(outputpath,
                       templateparser.processtemplate(templatepath, env))
end

function templateparser.processtemplate(templatepath, env)
  local file = io.open(templatepath, "r")  
  local t = templateparser.prep(file, env)
  file:close()  
  return t
end

function templateparser.write(filepath, text)
   local file = io.open(filepath, "w")
   file:write(text)
   file:close()   
end

return templateparser
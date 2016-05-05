local templateparser = {}

function templateparser.prep(file, env)
  local output = ""
  for line in file:lines() do     
    local last = 1
    for text, expr, index in string.gmatch(line, "(.-)$(%b())()") do 
      last = index
      if text ~= "" then
        output = output .. text
      end  
        --table.insert(chunk, string.format('io.write %q ', text))
      --end
      --output = output .."EXP".. expr  
      local trimmedexp = expr:sub(2, -2)      
      local realvalue = env[trimmedexp]
      if not realvalue then
         output = output .. "#ERROR with ".."$("..expr..") and "..trimmedexp.."#ERROREND"
      else        
        output = output .. realvalue
      end
    end    
    output = output ..string.sub(line, last) .. "\n"
        --table.insert(chunk, string.format('io.write%s ', expr))    
      --output = output .."EXP".. expr
    --output = output .. string.sub(line, last)
      --table.insert(chunk, string.format('io.write %q\n',
      --                                 string.sub(line, last).."\n"))    
  end
  return output
end

return templateparser
local link = {
  title = "HELP LEVELS",  
  prompt = "ENTER Space OR NEW HELP LEVEL (0, 1, 2, OR 3)",  
  keymap = {},  
  display = {
    {section = ""},
    {shortdesc = "3", desc = "all menus and explanations displayed"},
    {shortdesc = "2", desc = "main editing menu (1-controll-char commands) surpressed"},
    {shortdesc = "1", desc = "prefix menus (2-character commands) also surpressed"},
    {shortdesc = "0", desc = "command explanations (including this) also surpressed"},    
  }
}
link.keymap["0"] = "@sethelplevel0"
link.keymap["1"] = "@sethelplevel1"
link.keymap["2"] = "@sethelplevel2"
link.keymap["3"] = "@sethelplevel3"

return link

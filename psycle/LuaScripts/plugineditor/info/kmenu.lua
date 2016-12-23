return {
  title = "BLOCK MENU",
  keymap = {
    S = "@saveresume",
    D = "@savedone",
    X = "@saveexit",
    Q = "@abandonfile",
    B = "@setblockbegin",
    K = "@setblockend",
    Y = "@deleteblock"
    --V = "@moveblock",
    --W = "@writeblock"
  },  
  display = {
    {section = "-Saving Files-"},
    {shortdesc = "S", desc = "Save & resume"},
    {shortdesc = "D", desc = "Save--done"},
    {shortdesc = "X", desc = "Save & exit"},
    {shortdesc = "Q", desc = "Abandon file"},
    {section = "-Block Operations-"},    
    {shortdesc = "B", desc = "Begin"},
    {shortdesc = "K", desc = "End"},
    --{shortdesc = "C", desc = "Copy"},
    {shortdesc = "Y", desc = "Delete"}
    --{shortdesc = "V", desc = "Move"},    
    --{shortdesc = "W", desc = "Write"}
  }
}

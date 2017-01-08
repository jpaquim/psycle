local setting = {
  { selector = "group",
    properties = {
      color = "@PVFONT"
    }
  },
  { selector = "edit",    
    properties = {
      color = "@PVFONT",
      backgroundcolor = "@PVROW"
    }
  },
  { selector = "listview",
    properties = {
      color = "@PVFONT",
      backgroundcolor = "@PVBACKGROUND"
    }
  },  
  { selector = "treeview",
    properties = {
      color = "@PVFONT",
      backgroundcolor = "@PVROWBEAT"
    }
  },
  { selector = "toolicon",
    properties = {
      color = "@PVFONT",
      backgroundcolor = "@PVBACKGROUND",
      hovercolor = "@PVFONT",
      hoverbackgroundcolor = "@PVROW4BEAT",
      activecolor = "@PVFONT",
      activebackgroundcolor = "@PVSELECTION"
    }
  },  
  { selector = "tabgroup",
    properties = {
      color = "@PVFONT",
      backgroundcolor = "@PVROW",
      tabbarcolor = "@PVROW",
      headercolor = "@PVFONT",
      headerbackgroundcolor = "@PVROW",
      headerhovercolor = "@PVFONT",
      headerhoverbackgroundcolor = "@PVROW4BEAT",
      headeractivecolor = "@PVFONT",
      headeractivebackgroundcolor = "@PVROW4BEAT",
      headerbordercolor = "@PVSELECTION",
      headerclosecolor = 0xffb0d8b1,
      headerclosehovercolor = 0xffffffff,
      headerclosehoverbackgroundcolor = 0xffa8444c
    }
  }
}


return setting
 
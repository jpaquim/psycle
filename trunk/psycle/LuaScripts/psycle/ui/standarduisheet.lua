local setting = {
  { selector = "group",
    properties = {
      color = "@PVFONT"
    }
  },
  { selector = "edit",    
    properties = {      
      backgroundcolor = "@PVROW"
    }
  },
  { selector = "listview",
    properties = {
      backgroundcolor = "@PVBACKGROUND"
    }
  },  
  { selector = "treeview",
    properties = {      
      backgroundcolor = "@PVROWBEAT"
    }
  },
  { selector = "toolicon",
    properties = {      
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
      headeractivebackgroundcolor = 0xFF444444,
      headerbordercolor = "@PVSELECTION",
      headerclosecolor = 0xffb0d8b1,
      headerclosehovercolor = 0xffffffff,
      headerclosehoverbackgroundcolor = 0xffa8444c
    }
  }
}


return setting
 
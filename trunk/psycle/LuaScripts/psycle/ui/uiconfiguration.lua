-- psycle (c) 2016 by psycledelics
-- File: uiconfiguration.lua
-- copyright 2016 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  


local uiconfiguration = {
  elements = {    
	edit = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = 17, type="stock"}
       }
	},
	text = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = { name="arial", size="12"}, type="font"}
       }
	},
	button = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = 17, type="stock"}
       }
	},
	radiobutton = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = 17, type="stock"}
       }
	},
	combobox = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = 17, type="stock"}
       }
	},
	checkbox = {
	   properties = {
         color = {value=0xFFFFFF, type="int"},
		 font = {value = 17, type="stock"}
       }
	}
  }
}

return uiconfiguration;
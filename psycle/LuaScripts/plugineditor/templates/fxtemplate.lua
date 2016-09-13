local fxtemplate = {
  name = "fx",
  label = "fx",
  filters = {{templatepath="plugin.lu$", outputpath="machine.lua"}},  
  properties = {
    {property = "pluginname", type="string", value="noname", edit = true},
	{property = "vendor", type="string", value="psycle", edit = true},
	{property = "machmode", type="machtype", value="fx", edit = false}
  }
}

return fxtemplate
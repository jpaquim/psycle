local generatortemplate = {
  name = "generator",
  label = "Generator",
  filters = {{templatepath="plugin.lu$", outputpath="machine.lua"}},   
  properties = {
    {property = "pluginname", type="string", value="noname", edit = true},
	{property = "vendor", type="string", value="psycle", edit = true},
	{property = "machmode", type="machtype", value="generator", edit = false}
  }  
}

return generatortemplate
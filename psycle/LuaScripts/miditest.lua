-- MidiTest
-- register file for psycle's lua plugin system


function psycle.info()
  local machinemodes = require("psycle.machinemodes")  
  return {
    vendor = "psycle",
    name = "miditest",
    mode = machinemodes.GENERATOR,
    version = 0,
    api = 0
  }
end

function psycle.start()
  mac = require("miditest.machine")  
  psycle.setmachine(mac)
end


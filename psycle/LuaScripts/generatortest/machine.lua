--[[
  psycle lua synthdemo
  file : machine.lua
]]

--require('mobdebug').start()

machine = require("psycle.machine"):new()

local array = require("psycle.array")
local dspmath = require("psycle.dsp.math")

local voices = {}
local channels = {}
-- require
local voice = require("generatortest.voice")
local param = require("psycle.parameter")
  
-- plugin info
function machine:info()
  return  {vendor="psycle", name="luaris", generator=1, version=0, api=0} 
end

function machine:help()
  return "01xx : slide up\n"..
         "02xx : slide down\n"..
		 "04xy : vibrato(frq,gain)\n"..
		 "0Cxx : volumne\n"..
         "C3xx : partomento"
end

local filtertypes = {"LowPass", "HighPass", "BandPass", "BandReject", "None", "ITLOWPASS"}

-- plugin constructor
function machine:init(samplerate)         
  voice.samplerate = samplerate        
  filter = require("psycle.dsp.filter")
  filtercurr = filter:new(filter.LOWPASS)
  -- setup voice independent parameters
  p = require("orderedtable"):new()
  p.mlb = param:newlabel("Master")
  p.vol = param:newknob("vol", "", 0, 1, 100, 0.5)
  p.flb = param:newlabel("Filter")
  p.ft = param:newknob("FilterType","",0,5,5,1):addlistener(self)
  function p.ft:display()
    return filtertypes[self:val()+1]
  end
  p.fc = param:newknob("VCF CutOff","",0,127,127,127):addlistener(self)
  p.fr = param:newknob("VCF Resonance","",0,127,127,0):addlistener(self)
  -- set parameters to the host
  self:addparameters(p)
  self:addparameters(voice.params)
  self:setnumcols(2)  
  -- create 3 polyphonic voices
  self.currvoice = 1 
  for i=1, 3 do voices[#voices+1] = voice:new()	end
  for i=0, 64 do channels[i] = 0 end  
end

function machine:work(num)     
  local n = #voices
  local c = 0
  for i = 1, n do   
    local v =voices[i]
    if v.isplaying or v.arp.playing then
     v:work(self:channel(0))
    end	
  end  
  self:channel(0):mul(p.vol:val())
  --vcfenv = filterenv:work(num):mul(vcfcg:val())
  filtercurr:work(self:channel(0)) --,vcfenv)
  self:channel(1):copy(self:channel(0))  
end

function machine:freevoice()
  local v = voices[self.currvoice];
  local c = 0
  while v.isplaying  or v.arp.playing do
    if self.currvoice == #voices then
      self.currvoice = 1
    else
      self.currvoice = self.currvoice + 1
    end
	c = c + 1
	if c > #voices then	      
	   voices[#voices+1] = voice:new()
       self.currvoice = #voices
	   break
	end
	v = voices[self.currvoice]
  end    
end

function machine:seqtick(channel, note, ins, cmd, val)    
  local curr = voices[channels[channel]]  
  if (note < 119) then 
    if cmd == 195 and curr~=nil and curr.isplaying then  -- portamento         
	  curr:glideto(note, 0.2)
    else	
      if curr~=nil and (curr.isplaying or curr.arp.playing) then
	    curr.arp.playing = false
	    curr:faststop()
	  end
	  self:freevoice()	  
	  channels[channel] = self.currvoice
	  curr = voices[self.currvoice];	
      curr:noteon(note)
    end   
  elseif curr~=nil and note==120 then
     voices[channels[channel]]:noteoff()	 
  end
  if curr~=nil then
    curr:initcmd(note, cmd, val);
  end
end

function machine:stop()
  local num = #voices
  for i = 1, num do    
	voices[i]:faststop()
	voices[i].arp.playing = false
  end	 
end

function machine:ontweaked(param)
  if param==p.ft then
     filtercurr:settype(param:val())
  elseif param==p.fc then
     filtercurr:setcutoff(param:val())
  elseif param==p.fr then
     filtercurr:setresonance(param:val())
  end	 
end

function machine:onsrchanged(rate)
  voice.samplerate = rate
end

return machine
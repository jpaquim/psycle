-- resamplerdemo/machine.lua
-- creates a wavetable with 10 saws at 261 hz and
-- a different set of harmonics
-- additional a slide up from the noton pitch 
-- is added

--require('mobdebug').start()

machine = require("psycle.machine"):new()

array = require("psycle.array");

function machine:info()
  return { vendor="psycle", name="resamplerdemo", generator=1, version=0, api=0 }
end

function machine.saw(num, maxharmonic)
  local data = array.new(num)  
  gain = 0.5
  local loop = array.arange(0, num, 1)
  for  h = 1, maxharmonic, 1 do
    amplitude = gain / h;
    to_angle = 2*math.pi / num * h;
    data:add(array.sin(math.pow(-1,h+1)*loop*to_angle)*amplitude)
  end
  return data
end

function machine.cwave(fh, sr)
   local f = 261.6255653005986346778499935233; -- C4
   local num = math.floor(sr/f + 0.5)         
   local hmax = math.floor(sr/2/fh)
   local data = machine.saw(num, hmax);
   wave = require("psycle.dsp.wavedata"):new()
   wave:copy(data)
   wave:setwavesamplerate(sr)
   wave:setloop(0, num)
   return wave
end

function machine:init(samplerate)  
   wavetable = {}   
   local flo = require("psycle.dsp.math").notetofreq(0)
   for i= 0, 10 do   
	 local fhi = flo*2
	 if i==0 then
	    flo = 0
	 end
	 local w = machine.cwave(fhi, samplerate)
     wavetable[#wavetable+1] = {w, flo, fhi}
	 flo = fhi
   end   
   resampler = require("psycle.dsp.resampler"):newwavetable(wavetable)
   resampler:setfrequency(263.1);   
   noteon = false
   env = require("psycle.envelope") 
end

function machine:work(num)
   if noteon then
     local a = envf:work(num);	 
     resampler:work(self:channel(0), self:channel(1), a)	 
	 --noteon = envf:isplaying()
   end
end

function machine:stop()
  resampler:noteoff()
  noteon = false
end

function machine:seqtick(channel, note, ins, cmd, val)    
   if note < 119 then      
      noteon = true
	  local f = require("psycle.dsp.math").notetofreq(note)
	  resampler:setfrequency(f);
	  resampler:start()
	  envf = env:new({{1, 900}},3);
   elseif note == 120 then
      resampler:noteoff()
	  noteon = false
   end	  
end

function machine:onsrchanged(rate)  
  self:init(rate)
end

return machine


-- todo
-- To eliminate these problems, we can gradually taper off the higher order partials by multiplying them by a raised cosine window. The raised cosine can be calculated as follows:
-- This can be calculated once for each table
-- double kGibbs = PI / (2 * numPartials);
-- This is calculated once for each partial in each table
-- double temp = cos((partial-1)*kGibbs);
-- double raisedCosine = temp * temp;; // Square it
-- Calculate windowed amplitude for Nth partial
-- ampl = raisedCosine / partial; 
-- aus dem buch audio anecdotes 2

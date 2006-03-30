/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "sampler.h"

char* Sampler::_psName = "Sampler";

Sampler::Sampler(int index)
{
  _macIndex = index;
  _numPars=0;
  _type = MACH_SAMPLER;
  _mode = MACHMODE_GENERATOR;
  sprintf(_editName, "Sampler");

  _resampler.SetQuality(dsp::R_LINEAR);
  for (int i=0; i<SAMPLER_MAX_POLYPHONY; i++)
  {
     _voices[i]._envelope._stage = ENV_OFF;
     _voices[i]._envelope._sustain = 0;
     _voices[i]._filterEnv._stage = ENV_OFF;
     _voices[i]._filterEnv._sustain = 0;
     _voices[i]._filter.Init();
     _voices[i]._cutoff = 0;
     _voices[i]._sampleCounter = 0;
     _voices[i]._triggerNoteOff = 0;
     _voices[i]._triggerNoteDelay = 0;
     _voices[i]._channel = -1;
     _voices[i]._wave._lVolCurr = 0;
     _voices[i]._wave._rVolCurr = 0;

     _voices[i].effCmd = SAMPLER_CMD_NONE;
   }

   for (int i = 0; i < MAX_TRACKS; i++)
   {
     lastInstrument[i]=255;
   }
}

Sampler::~ Sampler( )
{
}




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
#include "deserializer.h"

unsigned long DeSerializer::FourCC( const char *ChunkName)
{
   long retbuf = 0x20202020;   // four spaces (padding)
   char *p = ((char *)&retbuf);
   // Remember, this is Intel format!
   // The first character goes in the LSB
   for( int i(0) ; i < 4 && ChunkName[i]; ++i) *p++ = ChunkName[i];
   return retbuf;
}

uint32_t WaveDeSerializer::SamplingRate( ) const
{
  return wave_format.data.nSamplesPerSec;
}

uint16_t WaveDeSerializer::BitsPerSample( ) const
{
  return wave_format.data.nBitsPerSample;
}

uint16_t WaveDeSerializer::NumChannels( ) const
{
  return wave_format.data.nChannels;
}

uint32_t WaveDeSerializer::NumSamples( ) const
{
  return num_samples;
}

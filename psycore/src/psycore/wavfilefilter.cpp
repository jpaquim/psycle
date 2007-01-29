/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "wavfilefilter.h"
#include "sample.h"
#include "tr1stdint.h"
#include <vector>
#include <list>

namespace psy {
  namespace core {
        
    struct RiffHeader {
      char riff[4];
      psy::tr1::uint32_t fileLength;
      char wave[4];
    };

    struct WavFmt
    {
      // like WAVEFORMATEX structure
      char chunkID[4];
      psy::tr1::uint32_t chunkSize;
      psy::tr1::uint16_t wFormatTag;
      psy::tr1::uint16_t wChannels;
      psy::tr1::uint32_t dwSamplesPerSec;
      psy::tr1::uint32_t dwAvgBytesPerSec;
      psy::tr1::uint16_t wBlockAlign;
      psy::tr1::uint16_t wBitsPerSample;
    };

    WavFileFilter::WavFileFilter( Sample& sample )
      : SampleFileFilter( sample )
    {
    }

    WavFileFilter::~WavFileFilter()
    {
    }

    bool WavFileFilter::read( BinRead& in ) {
      // WAVE format starts with the RIFF header
      RiffHeader riffHeader;
      in.read( riffHeader.riff, 4 );
      if (!memcmp( riffHeader.riff, "RIFF", 4 )) return 0; 
      riffHeader.fileLength = in.readInt4LE();
      in.read( riffHeader.wave, 4 );
      if (!memcmp( riffHeader.wave, "WAVE", 4 )) return 0; 

      unsigned int chunk_count = 0;
      WavFmt fmt;
      std::list< std::vector<float> >::iterator channelItr = sample().channelBegin();
      while( !in.eof() ) {
        char chunkHeader[4];
        in.read(chunkHeader, 4);
        if ( memcmp( chunkHeader, "fmt", 4 )) {
          memcpy(fmt.chunkID,"fmt",4);
          fmt.chunkSize = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.wFormatTag = in.readUInt2LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.wChannels = in.readUInt2LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.dwSamplesPerSec = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.dwAvgBytesPerSec = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.wBlockAlign = in.readUInt2LE();
          if ( in.bad() || in.eof() ) return 0;
          fmt.wBitsPerSample= in.readUInt2LE();
          if ( in.bad() ) return 0;
          sample().addNewChannel( fmt.wChannels );          
          channelItr = sample().channelBegin();
        } else
        if ( memcmp( chunkHeader, "data", 4) ) {
          // For multi-channel data, samples are interleaved between channels
          // For stereo audio, channel 0 is the left channel and channel 1 is the right.
          unsigned int dataLength = in.readUInt4LE();
          switch (fmt.wBitsPerSample) {
            case 8: {
              // 8-bit samples are stored as unsigned bytes, ranging from 0 to 255
              char data;
              in.read( &data, 1);
              if ( in.bad() ) return 0;
              (*channelItr).push_back(static_cast<float>(static_cast<int>(data)/127));
            }
            break;
            case 16:
              // 16-bit samples are stored as 2's-complement signed integers, ranging from -32768 to 32767.
              (*channelItr).push_back(static_cast<float>( in.readInt2LE() / 32768));
              if ( in.bad() ) return 0;
            break;
            case 24:
            default: ;
          };
          ++channelItr;
          if ( channelItr == sample().channelEnd() ) {
            channelItr = sample().channelBegin();
          }
        }
        chunk_count++;
      }
      return true;
    }

  }
}

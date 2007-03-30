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
      // Offset  Length   Contents      
      RiffHeader riffHeader;
      // 0       4 bytes  'RIFF'
      in.read( riffHeader.riff, 4 );
      if (!memcmp( riffHeader.riff, "RIFF", 4 )) return 0; 
      // 4       4 bytes  <file length - 8>
      riffHeader.fileLength = in.readInt4LE();
      in.read( riffHeader.wave, 4 );
      // 8       4 bytes  'WAVE'
      if (!memcmp( riffHeader.wave, "WAVE", 4 )) return 0; 

      unsigned int chunk_count = 0;
      //Next, the fmt chunk describes the sample format:
      WavFmt fmt;
      // channel Iterator
      std::list< Channel >::iterator channelItr = sample().channels().begin();
      while( !in.eof() ) {
        char chunkHeader[4];
        in.read(chunkHeader, 4);
        if ( memcmp( chunkHeader, "fmt", 4 )) {                                                                                       
          // 12      4 bytes  'fmt ' , read above as riff header
          memcpy(fmt.chunkID,"fmt",4);
          // 16      4 bytes  0x00000010     // Length of the fmt data (16 bytes)
          fmt.chunkSize = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          // 20      2 bytes  0x0001         // Format tag: 1 = PCM
          fmt.wFormatTag = in.readUInt2LE();          
          if ( in.bad() || in.eof() ) return 0;
          // 22      2 bytes  <channels>     // Channels: 1 = mono, 2 = stereo
          fmt.wChannels = in.readUInt2LE();
          if ( in.bad() || in.eof() ) return 0;
          // 24      4 bytes  <sample rate>  // Samples per second: e.g., 44100
          fmt.dwSamplesPerSec = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          // 28      4 bytes  <bytes/second> // sample rate * block align
          fmt.dwAvgBytesPerSec = in.readUInt4LE();
          if ( in.bad() || in.eof() ) return 0;
          // 32      2 bytes  <block align>  // channels * bits/sample / 8
          fmt.wBlockAlign = in.readUInt2LE();
          if ( in.bad() || in.eof() ) return 0;
          // 34      2 bytes  <bits/sample>  // 8 or 16
          fmt.wBitsPerSample= in.readUInt2LE();
          if ( in.bad() ) return 0;
          sample().channels().addNewChannel( fmt.wChannels );                    
        } else if ( memcmp( chunkHeader, "data", 4) ) {
          // 4 bytes  <length of the data block>
          unsigned int frame = 0;
          unsigned int size = in.readUInt4LE();
          channelItr = sample().channels().begin();
          for ( ; channelItr != sample().channels().end(); channelItr++ ) {
            int frames = size / fmt.wBitsPerSample / fmt.wChannels;
            (*channelItr).createBuffer( frames );
          }
          channelItr = sample().channels().begin();
          while ( size ) {
            switch (fmt.wBitsPerSample) 
            {
            case 8: {
              // 8-bit samples are stored as unsigned bytes, ranging from 0 to 255
              char data;
              in.read( &data, 1);
              if ( in.bad() ) return 0;
              (*channelItr).buffer()[frame] = static_cast<float>(static_cast<int>(data)/127);
              size--;
            }
            break;
            case 16:
              // 16-bit samples are stored as 2's-complement signed integers, ranging from -32768 to 32767.
              (*channelItr).buffer()[frame] = static_cast<float>( in.readInt2LE() / 32768);
              if ( in.bad() ) return 0;
              size--;
              size--;
            break;
            case 24:
              return 0;
            break;
            default: 
              return 0;
            };
            // For multi-channel data, samples are interleaved between channels
            // sample 0 for channel 0  
            // sample 0 for channel 1  
            // sample 1 for channel 0
            // sample 1 for channel 1
            // ...
            // For stereo audio, channel 0 is the left channel and channel 1 is the right.
            ++channelItr; // increase channelIterator after each sample read
            if ( channelItr == sample().channels().end() ) {
              // after each channel is filled go back to the beginning of the channellist
              // and start writing the next sample
              channelItr = sample().channels().begin();
              frame++;
            }
          }
        }
        chunk_count++;
      }
      return true;
    }

  }
}

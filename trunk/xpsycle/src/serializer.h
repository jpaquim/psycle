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
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

#include <inttypes.h>


/// \todo add real detection of type size
typedef unsigned char byte;
/// \todo add real detection of type size
typedef unsigned short word;
/// \todo add real detection of type size
typedef unsigned long dword;


class WaveFormat_ChunkData
{
   public:
      /// Format category (PCM=1)
      uint16_t         wFormatTag;
      /// Number of channels (mono=1, stereo=2)
      uint16_t         nChannels;
      /// Sampling rate [Hz]
      uint32_t         nSamplesPerSec;
      uint32_t         nAvgBytesPerSec;
      uint16_t         nBlockAlign;
      uint16_t         nBitsPerSample;
      void Config
      (
        uint32_t NewSamplingRate = 44100,
        uint16_t NewBitsPerSample = 16,
        uint16_t NewNumChannels = 2
      )
      {
         nSamplesPerSec = NewSamplingRate;
         nChannels = NewNumChannels;
         nBitsPerSample = NewBitsPerSample;
         nAvgBytesPerSec = nChannels * nSamplesPerSec * nBitsPerSample / 8;
         nBlockAlign = nChannels * nBitsPerSample / 8;
      }
      WaveFormat_ChunkData()
      {
        wFormatTag = 1; // PCM
        Config();
      }
};

class ExtRiffChunkHeader
{
  public:
    /// Four-character chunk ID
    uint32_t    ckID;
    /// Length of data in chunk
    uint32_t    ckSize;
};

class WaveFormat_Chunk
{
  public:
     ExtRiffChunkHeader header;
     WaveFormat_ChunkData data;
     WaveFormat_Chunk()
     {
       header.ckID = FourCC("fmt");
       header.ckSize = sizeof(WaveFormat_ChunkData);
     }
     bool VerifyValidity()
     {
       return
          header.ckID == FourCC("fmt") &&
          (data.nChannels == 1 || data.nChannels == 2) &&
           data.nAvgBytesPerSec == data.nChannels * data.nSamplesPerSec * data.nBitsPerSample / 8 &&
           data.nBlockAlign == data.nChannels * data.nBitsPerSample / 8;
     }
    unsigned long FourCC( const char *ChunkName)
    {
      long retbuf = 0x20202020;   // four spaces (padding)
      char *p = ((char *)&retbuf);
      // Remember, this is Intel format!
      // The first character goes in the LSB
      for( int i(0) ; i < 4 && ChunkName[i]; ++i) *p++ = ChunkName[i];
      return retbuf;
    }

};


class Serializer
{
public:
    Serializer (std::string const & nameFile)
        : _stream (nameFile.c_str (), std::ios_base::out | std::ios_base::binary)
    {
        type = 0;
        if (!_stream.is_open ())
            throw "couldn't open file";
    }

    Serializer (std::string const & nameFile, uint32_t SamplingRate, uint16_t BitsPerSample, uint16_t NumChannels)
        : _stream (nameFile.c_str (), std::ios_base::out | std::ios_base::binary)
    {
        type = 1;
        if (!_stream.is_open ())
            throw "couldn't open file";

        ExtRiff_header.ckID = FourCC("RIFF");
        ExtRiff_header.ckSize = 0;

        extriff_data_offset = GetPos();
        PutPChar( (char*) &ExtRiff_header, sizeof(ExtRiff_header));

        if ((BitsPerSample != 8 && BitsPerSample != 16 && BitsPerSample != 24 && BitsPerSample != 32) || NumChannels < 1 || NumChannels > 2)
           throw "invalid call";

        data.Config(SamplingRate, BitsPerSample, NumChannels);

        PutPChar( "WAVE", 4 );
        PutPChar( (char*) &wave_format, sizeof(wave_format) );
        pcm_data.ckID = FourCC("data");
        pcm_data.ckSize = 0;
        pcm_data_offset = GetPos();
        PutPChar( (char*) &pcm_data, sizeof(pcm_data) );
    }

    ~Serializer() {
      close();
    }

    unsigned long FourCC( const char *ChunkName)
    {
      long retbuf = 0x20202020;   // four spaces (padding)
      char *p = ((char *)&retbuf);
      // Remember, this is Intel format!
      // The first character goes in the LSB
      for( int i(0) ; i < 4 && ChunkName[i]; ++i) *p++ = ChunkName[i];
      return retbuf;
    }


    std::size_t GetPos() {
      return  _stream.tellp();
    }

    void Seek(std::size_t pos) {
      _stream.seekp(pos);
    }

    void PutPChar (char* buf, int bytes)
    {
      ExtRiff_header.ckSize += bytes;
        _stream.write(buf, bytes);
        if (_stream.bad())
            throw "file write failed";
    }

    void PutFloat (float i)
    {
        _stream.write (reinterpret_cast<char *> (&i), sizeof (float));
        if (_stream.bad())
            throw "file write failed";
    }


    void PutInt (int i)
    {
        _stream.write (reinterpret_cast<char *> (&i), sizeof (int));
        if (_stream.bad())
            throw "file write failed";
    }

    void PutLong (long l)
    {
        _stream.write (reinterpret_cast<char *> (&l), sizeof (long));
        if (_stream.bad())
            throw "file write failed";
    }
    void PutDouble (double d)
    {
        _stream.write (reinterpret_cast<char *> (&d), sizeof (double));
        if (_stream.bad())
            throw "file write failed";
    }
    void PutString (std::string const & str)
    {
        int len = str.length ();
        _stream.write (str.data (), len);
        PutBool(0);
        if (_stream.bad())
            throw "file write failed";
    }
    void PutBool (bool b)
    {
        _stream.write (reinterpret_cast<char *> (&b), sizeof (bool));
        if (_stream.bad ())
            throw "file write failed";
    }

    void WriteMonoSample(float SampleData) {
      int d;
      if(SampleData > 32767.0f) SampleData = 32767.0f; else
      if(SampleData < -32768.0f) SampleData = -32768.0f;

      switch( data.nBitsPerSample)
      {
        case 8:
          pcm_data.ckSize += 1;
          d = int(SampleData/256.0f);
          d += 128;
          PutPChar ((char*) &d, 1 );
        break;
        case 16:
          pcm_data.ckSize += 2;
          d = int(SampleData);
          PutPChar ((char*) &d, 2 );
        break;
        case 24:
          pcm_data.ckSize += 3;
          d = int(SampleData * 256.0f);
          PutPChar ((char*) &d, 3 );
        break;
        case 32:
          pcm_data.ckSize += 4;
          d = int(SampleData * 65536.0f);
          PutPChar ((char*) &SampleData, 4 );
        break;
      }
    }

    void WriteStereoSample( float LeftSample, float RightSample ) {
       int l, r;
       if(LeftSample > 32767.0f) LeftSample = 32767.0f;
       else if (LeftSample < -32768.0f) LeftSample = -32768.0f;
       if(RightSample > 32767.0f) RightSample = 32767.0f;
       else if(RightSample < -32768.0f) RightSample = -32768.0f;
       switch( data.nBitsPerSample)
       {
         case 8:
           l = int(LeftSample/256.0f);
           r = int(RightSample/256.0f);
           l+= 128;
           r+= 128;
           PutPChar ( (char*) &l, 1 );
           PutPChar ( (char*) &r, 1);
           pcm_data.ckSize += 2;
         break;
         case 16:
           l = int(LeftSample);
           r = int(RightSample);
           PutPChar( (char*) &l, 2 );
           PutPChar( (char*) &r, 2); 
           pcm_data.ckSize += 4;
         break;
         case 24:
           l = int(LeftSample*256.0f);
           r = int(RightSample*256.0f);
           PutPChar((char*)&l, 3);
           PutPChar((char*)&r, 3);
           pcm_data.ckSize += 6;
         break;
         case 32:
           l = int(LeftSample*65536.0f);
           r = int(RightSample*65536.0f);
           PutPChar( (char*) &l, 4);
           PutPChar( (char*) &r, 4 );
           pcm_data.ckSize += 8;
         break;
         default:;

         }

    }

    int close() {
      if (type == 1) {
        Seek(pcm_data_offset);
        PutPChar((char*) &pcm_data, sizeof(pcm_data));
        Seek(extriff_data_offset);
        PutPChar((char*) &ExtRiff_header, sizeof(ExtRiff_header));
        _stream.seekp(0, std::ios::end);         // goto end of file
      }


      _stream.close();
    }

    WaveFormat_ChunkData data;

private:
    int type;

    std::ofstream _stream;
    ExtRiffChunkHeader    pcm_data;
    WaveFormat_Chunk   wave_format;
    ExtRiffChunkHeader ExtRiff_header;
    long               pcm_data_offset;
    long               extriff_data_offset;
};


#endif


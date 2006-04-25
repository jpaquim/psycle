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
#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "serializer.h"
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


class DeSerializer
{
public:
    DeSerializer (std::string const & nameFile) throw (const char *)
        : _stream (nameFile.c_str (), std::ios_base::in | std::ios_base::binary)
    {
       std::cout << nameFile.c_str() << std::endl;
       if (!_stream.is_open ()) throw "couldn't open file";
        _stream.seekg (0, std::ios::beg);
    }

    std::size_t fileSize() {
      std::size_t curPos = getPos();
      _stream.seekg(0, std::ios::end);         // goto end of file
      std::size_t fileSize = _stream.tellg();  // read the filesize
      _stream.seekg(curPos);                   // go back to begin of file
      return fileSize;
    }

    std::size_t getPos() {
     return  _stream.tellg();
    }

    bool readString(char* pData, long maxBytes)
    {
      if(maxBytes > 0) {
         memset(pData,0,maxBytes);
         char c;
         for(long index = 0; index < maxBytes; index++)
         {
           if ( (c=_stream.get())!=EOF)
           {
              pData[index] = c;
              if(c == '\0') return true;
           }
         }
         if (c==EOF) return true; else false;
       }
      return false;
    }

    bool checkVersion(int currentVer) {
       int version = getVersion();
       int size    = getSize();
       if (version > currentVer) {
         skip(size);
         return false;
       }
       return true;
    }

    void read(byte * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(reinterpret_cast<char*>(buf) ,bytes);
      if (_stream.bad())
            throw "file read failed";
    }

    void read(int * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(reinterpret_cast<char*>(buf) ,bytes);
      if (_stream.bad())
            throw "file read failed";
    }

    void read(short * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(reinterpret_cast<char*>(buf) ,bytes);
      if (_stream.bad())
            throw "file read failed";
    }


    void read(float * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(reinterpret_cast<char*>(buf) ,bytes);
      if (_stream.bad())
            throw "file read failed";
    }

    void read(bool * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(reinterpret_cast<char*>(buf),bytes);
      if (_stream.bad())
            throw "file read failed";
    }



    void read(char * buf, int bytes) {
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(buf,bytes);
      if (_stream.bad())
            throw "file read failed";
    }

    byte getByte() {
      char buf[1];
      if (_stream.eof())
            throw "unexpected end of file";
      _stream.read(buf,1);
      if (_stream.bad())
            throw "file read failed";
      return buf[0];
    }

    int getVersion() {
       return getInt();
    }

    int getSize() {
       return getInt();
    }

    bool eof() {
       return _stream.eof();
    }

    void skip(int offset) {
       _stream.seekg(offset, std::ios::cur);
    }

    std::string getHeader4()
    {
        if (_stream.eof())
            throw "unexpected end of file";
        char data4[4];
        _stream.read(data4, sizeof (data4));
        if (_stream.bad())
            throw "file read failed";
        return std::string(data4,4);
    }

    std::string getHeader8()
    {
        if (_stream.eof())
            throw "unexpected end of file";
        char data8[8];
        _stream.read (data8, sizeof (data8));
        if (_stream.bad())
            throw "file read failed";
        return std::string(data8,8);
    }

    int getInt ()
    {
        if (_stream.eof())
            throw "unexpected end of file";
        int i;
        _stream.read (reinterpret_cast<char *> (&i), sizeof (int));
        if (_stream.bad())
            throw "file read failed";
        return i;
    }

    int close() {
      _stream.close();
    }

    unsigned long FourCC( const char *ChunkName);

    private:
    std::ifstream _stream;
};

class WaveDeSerializer : public DeSerializer {

public:

  WaveDeSerializer (const std::string & nameFile) : DeSerializer(nameFile) {

    ExtRiff_header.ckID = FourCC("RIFF");
    ExtRiff_header.ckSize = 0;

    pcm_data.ckID = FourCC("data");
    pcm_data.ckSize = 0;
    num_samples = 0;

    // Try to read the ExtRiff header...
    try {
      read( (char*) &ExtRiff_header, sizeof(ExtRiff_header));
      read ( (char*) &wave_format.header, sizeof(wave_format.header) );

      while(wave_format.header.ckID != FourCC("fmt "))
      {
        //Skip (wave_format.header.ckSize);// read each block until we find the correct one
                                           // we didn't find our header, so move back and try again
        skip(1 - sizeof wave_format.header);
        read ((char*) &wave_format.header, sizeof(wave_format.header) );
      }

      read ( (char*) &wave_format.data, sizeof(wave_format.data) );

      if(!wave_format.VerifyValidity())
      {
           // This isn't standard PCM, so we don't know what it is!
           throw "DDC_FILE_ERROR";
      }
      pcm_data_offset = getPos();
      read ((char*) &pcm_data, sizeof(pcm_data) );
      while( pcm_data.ckID != FourCC("data") || pcm_data.ckSize == 0)
      {
          // Skip (pcm_data.ckSize);// read each block until we find the correct one
          // this is not our block, so move back and search for our header
          skip(1 - sizeof pcm_data);
          pcm_data_offset = getPos();
          read((char*)&pcm_data, sizeof pcm_data);
      }
      num_samples = pcm_data.ckSize;
        //num_samples = filelength(fileno(file)) - CurrentFilePosition();
      num_samples /= NumChannels();
      num_samples /= (BitsPerSample() / 8);
    } catch (const char* e) {
       throw "RFM_UNKNOWN";
    }
  }

  uint32_t  SamplingRate()   const;
  uint16_t  BitsPerSample()  const;
  uint16_t  NumChannels()    const;
  uint32_t  NumSamples()     const;

  void ReadData  ( uint8_t *data, uint32_t numData ) {
    read(data,numData );
  }

  void ReadData  ( int16_t *data, uint32_t numData ) {
    read(reinterpret_cast<char*>(data),numData*sizeof(int16_t) );
  }

private:

  ExtRiffChunkHeader   ExtRiff_header;
  WaveFormat_Chunk   wave_format;
  ExtRiffChunkHeader    pcm_data;
  /// offset of 'pcm_data' in output file
  long               pcm_data_offset;
  uint32_t           num_samples;

};


#endif

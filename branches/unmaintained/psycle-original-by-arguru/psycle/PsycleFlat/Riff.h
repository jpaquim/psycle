/*===========================================================================

      riff.h  -  Don Cross, April 1993.

      RIFF file format classes.
      See Chapter 8 of "Multimedia Programmer's Reference" in
      the Microsoft Windows SDK.

      See also:
          ..\source\riff.cpp
          ddc.h

===========================================================================*/

#include "ddc.h"

#ifndef __DDC_RIFF_H
#define __DDC_RIFF_H

#pragma pack(1)

unsigned long FourCC ( const char *ChunkName );


enum RiffFileMode
{
   RFM_UNKNOWN,      // undefined type (can use to mean "N/A" or "not open")
   RFM_WRITE,        // open for write
   RFM_READ          // open for read
};


struct RiffChunkHeader
{
   UINT32    ckID;       // Four-character chunk ID
   UINT32    ckSize;     // Length of data in chunk
};


class RiffFile
{
   RiffChunkHeader   riff_header;      // header for whole file

protected:
   RiffFileMode      fmode;            // current file I/O mode
   FILE             *file;             // I/O stream to use
   DDCRET  Seek ( long offset );

public:
   RiffFile();
   ~RiffFile();

   RiffFileMode CurrentFileMode() const   {return fmode;}

   DDCRET Open ( const char *Filename, RiffFileMode NewMode );
   DDCRET Write  ( const void *Data, unsigned NumBytes );
   DDCRET Read   (       void *Data, unsigned NumBytes );
   DDCRET Expect ( const void *Data, unsigned NumBytes );
   DDCRET Close();

   long    CurrentFilePosition() const;

   DDCRET  Backpatch ( long FileOffset,
                       const void *Data,
                       unsigned NumBytes );
};


struct WaveFormat_ChunkData
{
   UINT16         wFormatTag;       // Format category (PCM=1)
   UINT16         nChannels;        // Number of channels (mono=1, stereo=2)
   UINT32         nSamplesPerSec;   // Sampling rate [Hz]
   UINT32         nAvgBytesPerSec;
   UINT16         nBlockAlign;
   UINT16         nBitsPerSample;

   void Config ( UINT32   NewSamplingRate   = 44100,
                 UINT16   NewBitsPerSample  =    16,
                 UINT16   NewNumChannels    =     2 )
   {
      nSamplesPerSec  = NewSamplingRate;
      nChannels       = NewNumChannels;
      nBitsPerSample  = NewBitsPerSample;
      nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
      nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   }

   WaveFormat_ChunkData()
   {
      wFormatTag = 1;     // PCM
      Config();
   }
};


struct WaveFormat_Chunk
{
   RiffChunkHeader         header;
   WaveFormat_ChunkData    data;

   WaveFormat_Chunk()
   {
      header.ckID     =   FourCC("fmt");
      header.ckSize   =   sizeof ( WaveFormat_ChunkData );
   }

   dBOOLEAN VerifyValidity()
   {
      return header.ckID == FourCC("fmt") &&

             (data.nChannels == 1 || data.nChannels == 2) &&

             data.nAvgBytesPerSec == ( data.nChannels *
                                       data.nSamplesPerSec *
                                       data.nBitsPerSample    ) / 8   &&

             data.nBlockAlign == ( data.nChannels *
                                   data.nBitsPerSample ) / 8;
   }
};


#define  MAX_WAVE_CHANNELS   2


struct WaveFileSample
{
   INT16  chan [MAX_WAVE_CHANNELS];
};


class WaveFile: private RiffFile
{
   WaveFormat_Chunk   wave_format;
   RiffChunkHeader    pcm_data;
   long               pcm_data_offset;  // offset of 'pcm_data' in output file
   UINT32             num_samples;

public:
   WaveFile();

   DDCRET OpenForWrite ( const char  *Filename,
                         UINT32       SamplingRate   = 44100,
                         UINT16       BitsPerSample  =    16,
                         UINT16       NumChannels    =     2 );

   DDCRET OpenForRead ( const char *Filename );

   DDCRET ReadSample   ( INT16 Sample [MAX_WAVE_CHANNELS] );
   DDCRET WriteSample  ( const INT16 Sample [MAX_WAVE_CHANNELS] );
   DDCRET SeekToSample ( unsigned long SampleIndex );

   // The following work only with 16-bit audio
   DDCRET WriteData ( const INT16 *data, UINT32 numData );
   DDCRET ReadData  ( INT16 *data, UINT32 numData );

   // The following work only with 8-bit audio
   DDCRET WriteData ( const UINT8 *data, UINT32 numData );
   DDCRET ReadData  ( UINT8 *data, UINT32 numData );

   DDCRET ReadSamples  ( INT32 num, WaveFileSample[] );

   DDCRET WriteMonoSample    ( INT16 ChannelData );
   DDCRET WriteStereoSample  ( INT16 LeftChannelData, INT16 RightChannelData );

   DDCRET ReadMonoSample ( INT16 *ChannelData );
   DDCRET ReadStereoSample ( INT16 *LeftSampleData, INT16 *RightSampleData );

   DDCRET Close();

   UINT32   SamplingRate()   const;    // [Hz]
   UINT16   BitsPerSample()  const;
   UINT16   NumChannels()    const;
   UINT32   NumSamples()     const;

   // Open for write using another wave file's parameters...

   DDCRET OpenForWrite ( const char *Filename,
                         WaveFile &OtherWave )
   {
      return OpenForWrite ( Filename,
                            OtherWave.SamplingRate(),
                            OtherWave.BitsPerSample(),
                            OtherWave.NumChannels() );
   }

   long CurrentFilePosition() const
   {
      return RiffFile::CurrentFilePosition();
   }
};

#pragma pack()

#endif /* __DDC_RIFF_H */


/*--- end of file riff.h ---*/

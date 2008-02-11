/* -*- mode:c++, indent-tabs-mode:t -*- */
/**
	\file
	interface file for psycle::host::ExtRiffFile and psycle::host::WaveFile.
	\verbatim
		===========================================================================

			ExtRiff.h  -  Don Cross, April 1993.

			ExtRiff file format classes.
			See Chapter 8 of "Multimedia Programmer's Reference" in
			the Microsoft Windows SDK.

			See also:
				..\source\ExtRiff.cpp
				ddc.h

		===========================================================================
	\endverbatim
*/
#pragma once
#include "ddc.hpp"
namespace psycle
{
	namespace host
	{
		#pragma pack(push, 1)
		unsigned long FourCC(const char * ChunkName);

		enum ExtRiffFileMode
		{
			///< undefined type (can use to mean "N/A" or "not open")
			RFM_UNKNOWN, 
			///< open for write
			RFM_WRITE, 
			///< open for read
			RFM_READ 
		};

		class ExtRiffChunkHeader
		{
		public:
			/// Four-character chunk ID
			UINT32    ckID;       
			/// Length of data in chunk
			UINT32    ckSize;     
		};

		/// riff file format.
		class ExtRiffFile
		{
		private:
			/// header for whole file
			ExtRiffChunkHeader   ExtRiff_header;      
		protected:
			/// current file I/O mode
			ExtRiffFileMode      fmode;            
			/// I/O stream to use
			FILE             *file;             
			DDCRET  Seek ( long offset );
		public:
			ExtRiffFile();
			virtual ~ExtRiffFile();
			ExtRiffFileMode CurrentFileMode() const   {return fmode;}
			DDCRET Open ( const char *Filename, ExtRiffFileMode NewMode );
			DDCRET Write  ( const void *Data, unsigned NumBytes );
			DDCRET Read   (       void *Data, unsigned NumBytes );
			DDCRET Expect ( const void *Data, unsigned NumBytes );
			/// Added by [JAZ]
			DDCRET Skip   ( unsigned NumBytes );
			DDCRET Close();
			long    CurrentFilePosition() const;
			DDCRET  Backpatch ( long FileOffset,
								const void *Data,
								unsigned NumBytes );
		};

		class WaveFormat_ChunkData
		{
		public:
			/// Format category (PCM=1)
			UINT16         wFormatTag;       
			/// Number of channels (mono=1, stereo=2)
			UINT16         nChannels;        
			/// Sampling rate [Hz]
			UINT32         nSamplesPerSec;   
			UINT32         nAvgBytesPerSec;
			UINT16         nBlockAlign;
			UINT16         nBitsPerSample;
			void Config
				(
					UINT32 NewSamplingRate = 44100,
					UINT16 NewBitsPerSample = 16,
					UINT16 NewNumChannels = 2
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
			dBOOLEAN VerifyValidity()
			{
				return
					header.ckID == FourCC("fmt") &&
					(data.nChannels == 1 || data.nChannels == 2) &&
					data.nAvgBytesPerSec == data.nChannels * data.nSamplesPerSec * data.nBitsPerSample / 8 &&
					data.nBlockAlign == data.nChannels * data.nBitsPerSample / 8;
			}
		};

		#define  MAX_WAVE_CHANNELS   2

		class WaveFileSample
		{
		public:
			INT16 chan[MAX_WAVE_CHANNELS];
		};


		/// riff wave file format.
		/// MODIFIED BY [JAZ]. It was "private ExtRiffFile".
		class WaveFile: public ExtRiffFile
		{
			WaveFormat_Chunk   wave_format;
			ExtRiffChunkHeader    pcm_data;
			/// offset of 'pcm_data' in output file
			long               pcm_data_offset;
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

			/// work only with 16-bit audio
			DDCRET WriteData ( const INT16 *data, UINT32 numData );
			/// work only with 16-bit audio
			DDCRET ReadData  ( INT16 *data, UINT32 numData );

			/// work only with 8-bit audio
			DDCRET WriteData ( const UINT8 *data, UINT32 numData );
			/// work only with 8-bit audio
			DDCRET ReadData  ( UINT8 *data, UINT32 numData );

			DDCRET ReadSamples  ( INT32 num, WaveFileSample[] );

			DDCRET WriteMonoSample    ( float ChannelData );
			DDCRET WriteStereoSample  ( float LeftChannelData, float RightChannelData );

			DDCRET ReadMonoSample ( INT16 *ChannelData );
			DDCRET ReadStereoSample ( INT16 *LeftSampleData, INT16 *RightSampleData );

			DDCRET Close();

			/// [Hz]
			UINT32   SamplingRate()   const;
			UINT16   BitsPerSample()  const;
			UINT16   NumChannels()    const;
			UINT32   NumSamples()     const;

			/// Open for write using another wave file's parameters...
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
				return ExtRiffFile::CurrentFilePosition();
			}
		};

		#pragma pack(pop)
	}
}

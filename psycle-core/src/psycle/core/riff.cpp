/**
	\file
	implementation file for psy::core::ExtRiffFile and psy::core::WaveFile.
	\verbatim
		==========================================================================

			ExtRiff.cpp  -  Don Cross, April 1993.

			Contains code for reading/writing ExtRiff files, including .WAV files!

			See also:
				..\include\ExtRiff.h
				..\include\ddc.h

			Revision history:

		1993 April 12 [Don Cross]
			Started writing this code.

		1994 October 6 [Don Cross]
			Added WriteData, ReadData.
			Added const modifiers to some function parameters.

		1997 September 14 [Don Cross]
			Fixed a bug in WaveFile::Close.  It was calling Backpatch
			and getting confused when the the file had been opened for read.
			(Backpatch returns an error in that case, which prevented 
			WaveFile::Close from calling ExtRiffFile::Close.)

		==========================================================================
	\endverbatim
*/
#include <psycle/core/psycleCorePch.hpp>

#include "riff.h"

namespace psy
{
	namespace core
	{
		unsigned long FourCC( const char *ChunkName)
		{
			long retbuf = 0x20202020;   // four spaces (padding)
			char *p = ((char *)&retbuf);
			// Remember, this is Intel format!
			// The first character goes in the LSB
			for( int i(0) ; i < 4 && ChunkName[i]; ++i) *p++ = ChunkName[i];
			return retbuf;
		}

		ExtRiffFile::ExtRiffFile()
		{
			file = 0;
			fmode = RFM_UNKNOWN;
			ExtRiff_header.ckID = FourCC("RIFF");
			ExtRiff_header.ckSize = 0;
		}

		ExtRiffFile::~ExtRiffFile()
		{
			if( fmode != RFM_UNKNOWN) Close();
		}

		DDCRET ExtRiffFile::Open ( const char *Filename, ExtRiffFileMode NewMode )
		{
			DDCRET retcode = DDC_SUCCESS;
			if( fmode != RFM_UNKNOWN) retcode = Close();
			if( retcode == DDC_SUCCESS)
			{
				switch(NewMode)
				{
				case RFM_WRITE:
					file = fopen ( Filename, "wb" );
					if(file)
					{
						// Write the ExtRiff header...
						// We will have to come back later and patch it!
						ExtRiff_header.ckID = FourCC("RIFF");
						ExtRiff_header.ckSize = 0;
						if(fwrite( &ExtRiff_header, sizeof(ExtRiff_header), 1, file) != 1)
						{
							fclose(file);
							remove(Filename);
							fmode = RFM_UNKNOWN;
							file = 0;
						}
						else fmode = RFM_WRITE;
					}
					else
					{
						fmode = RFM_UNKNOWN;
						retcode = DDC_FILE_ERROR;
					}
					break;
				case RFM_READ:
					file = fopen ( Filename, "rb" );
					if(file)
					{
						// Try to read the ExtRiff header...
						if(fread( &ExtRiff_header, sizeof(ExtRiff_header), 1, file) != 1)
						{
							fclose(file);
							fmode = RFM_UNKNOWN;
							file = 0;
						}
						else fmode = RFM_READ;
					}
					else
					{
						fmode = RFM_UNKNOWN;
						retcode = DDC_FILE_ERROR;
					}
					break;
				default:
					retcode = DDC_INVALID_CALL;
				}
			}
			return retcode;
		}

		DDCRET ExtRiffFile::Write(const void *Data, unsigned NumBytes)
		{
			if(fmode != RFM_WRITE) return DDC_INVALID_CALL;
			if(fwrite(Data, NumBytes, 1, file) != 1) return DDC_FILE_ERROR;
			ExtRiff_header.ckSize += NumBytes;
			return DDC_SUCCESS;
		}

		DDCRET ExtRiffFile::Close()
		{
			DDCRET retcode = DDC_SUCCESS;
			switch(fmode)
			{
			case RFM_WRITE:
				if
						(
							fflush(file) ||
							fseek(file,0,SEEK_SET) ||
							fwrite(&ExtRiff_header, sizeof(ExtRiff_header), 1, file) != 1 ||
							fclose(file)
						)
				{
					retcode = DDC_FILE_ERROR;
				}
				break;
			case RFM_READ:
				fclose(file);
				break;
			}
			file = 0;
			fmode = RFM_UNKNOWN;
			return retcode;
		}

		long ExtRiffFile::CurrentFilePosition() const
		{
			return ftell ( file );
		}

		DDCRET ExtRiffFile::Seek(long offset)
		{
			fflush(file);
			DDCRET rc;
			if(fseek(file, offset, SEEK_SET)) rc = DDC_FILE_ERROR;
			else rc = DDC_SUCCESS;
			return rc;
		}


		DDCRET ExtRiffFile::Backpatch(long FileOffset, const void *Data, unsigned NumBytes)
		{
			if(!file || fmode != RFM_WRITE) return DDC_INVALID_CALL;
			if
				(
					fflush(file) ||
					fseek(file, FileOffset, SEEK_SET) ||
					fwrite(Data, NumBytes, 1, file) != 1
				)
				return DDC_FILE_ERROR;
			return DDC_SUCCESS;
		}

		DDCRET ExtRiffFile::Expect(const void *Data, unsigned NumBytes)
		{
			char *p = (char*) Data;
			while( NumBytes--) if(fgetc(file) != *p++) return DDC_FILE_ERROR;
			return DDC_SUCCESS;
		}

		DDCRET ExtRiffFile::Skip(unsigned NumBytes)
		{
			//fflush(file);
			DDCRET rc;
			if(fseek(file, NumBytes, SEEK_CUR)) rc = DDC_FILE_ERROR;
			else rc = DDC_SUCCESS;
			return rc;
		}

		DDCRET ExtRiffFile::Read(void *Data, unsigned NumBytes)
		{
			DDCRET retcode = DDC_SUCCESS;
			if(fread(Data,NumBytes,1,file) != 1) retcode = DDC_FILE_ERROR;
			return retcode;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// WaveFile

		WaveFile::WaveFile()
		{
			pcm_data.ckID = FourCC("data");
			pcm_data.ckSize = 0;
			num_samples = 0;
		}

		/// Modified by [JAZ]
		DDCRET WaveFile::OpenForRead(const char *Filename)
		{
			// Verify filename parameter as best we can...
			if(!Filename) return DDC_INVALID_CALL;
			DDCRET retcode = Open ( Filename, RFM_READ );
			if(retcode == DDC_SUCCESS)
			{
				retcode = Expect ( "WAVE", 4 );
				if( retcode == DDC_SUCCESS)
				{
					retcode = Read ( &wave_format.header, sizeof(wave_format.header) );
					while(wave_format.header.ckID != FourCC("fmt "))
					{ 
						//Skip (wave_format.header.ckSize);// read each block until we find the correct one
						// we didn't find our header, so move back and try again
						Skip(1 - sizeof wave_format.header);
							retcode = Read ( &wave_format.header, sizeof(wave_format.header) );
						if(retcode != DDC_SUCCESS) return retcode;
					}
					retcode = Read ( &wave_format.data, sizeof(wave_format.data) );
					if(!wave_format.VerifyValidity())
					{
						// This isn't standard PCM, so we don't know what it is!
						retcode = DDC_FILE_ERROR;
					}
					if( retcode == DDC_SUCCESS)
					{
						// Figure out number of samples from
						// file size, current file position, and
						// WAVE header.
						pcm_data_offset = CurrentFilePosition();
						retcode = Read ( &pcm_data, sizeof(pcm_data) );
						while( pcm_data.ckID != FourCC("data") || pcm_data.ckSize == 0)
						{
							// Skip (pcm_data.ckSize);// read each block until we find the correct one
							// this is not our block, so move back and search for our header
							Skip(1 - sizeof pcm_data);
							pcm_data_offset = CurrentFilePosition();
							retcode = Read(&pcm_data, sizeof pcm_data);
							if( retcode != DDC_SUCCESS) return retcode;
						}
						num_samples = pcm_data.ckSize;
						//num_samples = filelength(fileno(file)) - CurrentFilePosition();
						num_samples /= NumChannels();
						num_samples /= (BitsPerSample() / 8);
					}
				}
			}
			return retcode;
		}

		DDCRET WaveFile::OpenForWrite (const char * Filename, UINT32 SamplingRate, UINT16 BitsPerSample, UINT16 NumChannels)
		{
			// Verify parameters...
			if
				(
					!Filename ||
					(BitsPerSample != 8 && BitsPerSample != 16 && BitsPerSample != 24 && BitsPerSample != 32) ||
					NumChannels < 1 || NumChannels > 2
				)
				return DDC_INVALID_CALL;
			wave_format.data.Config(SamplingRate, BitsPerSample, NumChannels);
			DDCRET retcode = Open ( Filename, RFM_WRITE );
			if( retcode == DDC_SUCCESS)
			{
				retcode = Write ( "WAVE", 4 );
				if( retcode == DDC_SUCCESS)
				{
					retcode = Write ( &wave_format, sizeof(wave_format) );
					if( retcode == DDC_SUCCESS)
					{
						pcm_data.ckSize = 0;
						pcm_data_offset = CurrentFilePosition();
						retcode = Write ( &pcm_data, sizeof(pcm_data) );
					}
				}
			}
			return retcode;
		}

		DDCRET WaveFile::Close()
		{
			DDCRET rc = DDC_SUCCESS;
			if(fmode == RFM_WRITE)
				rc = Backpatch ( pcm_data_offset, &pcm_data, sizeof(pcm_data) );
			if( rc == DDC_SUCCESS)
			rc = ExtRiffFile::Close();
			return rc;
		}

		DDCRET WaveFile::WriteSample(const INT16 Sample[MAX_WAVE_CHANNELS])
		{
			DDCRET retcode = DDC_SUCCESS;
			switch(wave_format.data.nChannels)
			{
			case 1:
				switch( wave_format.data.nBitsPerSample)
				{
				case 8:
					pcm_data.ckSize += 1;
					retcode = Write ( &Sample[0], 1 );
					break;

				case 16:
					pcm_data.ckSize += 2;
					retcode = Write ( &Sample[0], 2 );
					break;

				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			case 2:
				switch( wave_format.data.nBitsPerSample)
				{
				case 8:
					retcode = Write ( &Sample[0], 1 );
					if( retcode == DDC_SUCCESS)
					{
						retcode = Write(&Sample[1], 1);
						if( retcode == DDC_SUCCESS) pcm_data.ckSize += 2;
					}
					break;
				case 16:
					retcode = Write ( &Sample[0], 2 );
					if(retcode == DDC_SUCCESS)
					{
						retcode = Write ( &Sample[1], 2 );
						if(retcode == DDC_SUCCESS) pcm_data.ckSize += 4;
					}
					break;
				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}

		DDCRET WaveFile::WriteMonoSample(float SampleData)
		{
			int d;
			if(SampleData > 32767.0f) SampleData = 32767.0f;
			else if(SampleData < -32768.0f) SampleData = -32768.0f;
			switch( wave_format.data.nBitsPerSample)
			{
			case 8:
				pcm_data.ckSize += 1;
				d = int(SampleData/256.0f);
				d += 128;
				return Write ( &d, 1 );
			case 16:
				pcm_data.ckSize += 2;
				d = int(SampleData);
				return Write ( &d, 2 );
			case 24:
				pcm_data.ckSize += 3;
				d = int(SampleData * 256.0f);
				return Write ( &d, 3 );
			case 32:
				pcm_data.ckSize += 4;
				d = int(SampleData * 65536.0f);
				return Write ( &SampleData, 4 );
			}
			return DDC_INVALID_CALL;
		}

		DDCRET WaveFile::WriteStereoSample(float LeftSample, float RightSample)
		{
			DDCRET retcode = DDC_SUCCESS;
			int l, r;
			if(LeftSample > 32767.0f) LeftSample = 32767.0f;
			else if (LeftSample < -32768.0f) LeftSample = -32768.0f;
			if(RightSample > 32767.0f) RightSample = 32767.0f;
			else if(RightSample < -32768.0f) RightSample = -32768.0f;
			switch( wave_format.data.nBitsPerSample)
			{
			case 8:
				l = int(LeftSample/256.0f);
				r = int(RightSample/256.0f);
				l+= 128;
				r+= 128;
				retcode = Write ( &l, 1 );
				if( retcode == DDC_SUCCESS)
				{
					retcode = Write(&r, 1);
					if(retcode == DDC_SUCCESS) pcm_data.ckSize += 2;
				}
				break;
			case 16:
				l = int(LeftSample);
				r = int(RightSample);
				retcode = Write ( &l, 2 );
				if( retcode == DDC_SUCCESS)
				{
					retcode = Write(&r, 2);
					if(retcode == DDC_SUCCESS) pcm_data.ckSize += 4;
				}
				break;
			case 24:
				l = int(LeftSample*256.0f);
				r = int(RightSample*256.0f);
				retcode = Write(&l, 3);
				if(retcode == DDC_SUCCESS)
				{
					retcode = Write(&r, 3);
					if(retcode == DDC_SUCCESS) pcm_data.ckSize += 6;
				}
				break;
			case 32:
				l = int(LeftSample*65536.0f);
				r = int(RightSample*65536.0f);
				retcode = Write(&l, 4);
				if(retcode == DDC_SUCCESS)
				{
					retcode = Write ( &r, 4 );
					if( retcode == DDC_SUCCESS) pcm_data.ckSize += 8;
				}
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}

		DDCRET WaveFile::ReadSample(INT16 Sample[MAX_WAVE_CHANNELS])
		{
			DDCRET retcode = DDC_SUCCESS;
			switch( wave_format.data.nChannels)
			{
			case 1:
				switch( wave_format.data.nBitsPerSample)
				{
				case 8:
					unsigned char x;
					retcode = Read ( &x, 1 );
					Sample[0] = INT16(x);
					break;

				case 16:
					retcode = Read ( &Sample[0], 2 );
					break;

				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			case 2:
				switch( wave_format.data.nBitsPerSample)
				{
				case 8:
					unsigned char  x[2];
					retcode = Read ( x, 2 );
					Sample[0] = INT16 ( x[0] );
					Sample[1] = INT16 ( x[1] );
					break;

				case 16:
					retcode = Read ( Sample, 4 );
					break;

				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}


		DDCRET WaveFile::ReadSamples ( INT32 num, WaveFileSample sarray[] )
		{
			DDCRET retcode = DDC_SUCCESS;
			INT32 i;
			switch( wave_format.data.nChannels )
			{
			case 1:
				switch( wave_format.data.nBitsPerSample )
				{
				case 8:
					for(i = 0 ; i < num && retcode == DDC_SUCCESS; ++i)
					{
						unsigned char x;
						retcode = Read ( &x, 1 );
						sarray[i].chan[0] = INT16(x);
					}
					break;

				case 16:
					for(i=0 ; i < num && retcode == DDC_SUCCESS; ++i)
					{
						retcode = Read ( &sarray[i].chan[0], 2 );
					}
					break;

				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			case 2:
				switch( wave_format.data.nBitsPerSample )
				{
				case 8:
					for(i = 0 ; i < num && retcode == DDC_SUCCESS; ++i)
					{
						unsigned char x[2];
						retcode = Read ( x, 2 );
						sarray[i].chan[0] = INT16 ( x[0] );
						sarray[i].chan[1] = INT16 ( x[1] );
					}
					break;

				case 16:
					retcode = Read ( sarray, 4 * num );
					break;

				default:
					retcode = DDC_INVALID_CALL;
				}
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}

		DDCRET WaveFile::ReadMonoSample ( INT16 *Sample )
		{
			DDCRET retcode = DDC_SUCCESS;
			switch ( wave_format.data.nBitsPerSample )
			{
			case 8:
				unsigned char x;
				retcode = Read ( &x, 1 );
				*Sample = INT16(x);
				break;
			case 16:
				retcode = Read ( Sample, 2 );
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}

		DDCRET WaveFile::ReadStereoSample ( INT16 *L, INT16 *R )
		{
			DDCRET retcode = DDC_SUCCESS;
			UINT8 x[2];
			INT16 y[2];
			switch( wave_format.data.nBitsPerSample )
			{
			case 8:
				retcode = Read ( x, 2 );
				*L = INT16 ( x[0] );
				*R = INT16 ( x[1] );
				break;
			case 16:
				retcode = Read ( y, 4 );
				*L = INT16 ( y[0] );
				*R = INT16 ( y[1] );
				break;
			default:
				retcode = DDC_INVALID_CALL;
			}
			return retcode;
		}

		DDCRET WaveFile::SeekToSample ( unsigned long SampleIndex )
		{
			if( SampleIndex >= NumSamples()) return DDC_INVALID_CALL;
			unsigned SampleSize = (BitsPerSample() + 7) / 8;
			DDCRET rc = Seek ( pcm_data_offset + sizeof(pcm_data) + SampleSize * NumChannels() * SampleIndex );
			return rc;
		}

		UINT32 WaveFile::SamplingRate() const
		{
			return wave_format.data.nSamplesPerSec;
		}

		UINT16 WaveFile::BitsPerSample() const
		{
			return wave_format.data.nBitsPerSample;
		}

		UINT16 WaveFile::NumChannels() const
		{
			return wave_format.data.nChannels;
		}

		UINT32 WaveFile::NumSamples() const
		{
			return num_samples;
		}

		DDCRET WaveFile::WriteData ( const INT16 *data, UINT32 numData )
		{
			UINT32 extraBytes = numData * sizeof(INT16);
			pcm_data.ckSize += extraBytes;
			return ExtRiffFile::Write ( data, extraBytes );
		}

		DDCRET WaveFile::WriteData ( const UINT8 *data, UINT32 numData )
		{
			pcm_data.ckSize += numData;
			return ExtRiffFile::Write ( data, numData );
		}

		DDCRET WaveFile::ReadData ( INT16 *data, UINT32 numData )
		{
			return ExtRiffFile::Read ( data, numData * sizeof(INT16) );
		}

		DDCRET WaveFile::ReadData ( UINT8 *data, UINT32 numData )
		{
			return ExtRiffFile::Read ( data, numData );
		}
	}
}

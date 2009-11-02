
/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#pragma once
#include <cstdio>
#include <cstdint>
namespace psycle { namespace host {

enum DDCRET {
	DDC_SUCCESS,           ///< operation succeded
	DDC_FAILURE,           ///< operation failed for unspecified reasons
	DDC_OUT_OF_MEMORY,     ///< operation failed due to running out of memory
	DDC_FILE_ERROR,        ///< operation encountered file I/O error
	DDC_INVALID_CALL,      ///< operation was called with invalid parameters
	DDC_USER_ABORT,        ///< operation was aborted by the user
	DDC_INVALID_FILE       ///< file format does not match
};

std::uint32_t FourCC(const char * ChunkName);

enum ExtRiffFileMode {
	RFM_UNKNOWN, ///< undefined type (can use to mean "N/A" or "not open")
	RFM_WRITE, ///< open for write
	RFM_READ ///< open for read
};

class ExtRiffChunkHeader {
	public:
		/// Four-character chunk ID
		std::uint32_t ckID;       
		/// Length of data in chunk
		std::uint32_t ckSize;     
};

/// riff file format.
//UNIVERSALIS__COMPILER__DEPRECATED("c++ iostream for the better")
class ExtRiffFile {
	private:
		/// header for whole file
		ExtRiffChunkHeader   ExtRiff_header;      
	protected:
		/// current file I/O mode
		ExtRiffFileMode      fmode;
		/// I/O stream to use
		FILE             *file;
		DDCRET Seek(std::int32_t offset);
	public:
		ExtRiffFile();
		~ExtRiffFile();
		ExtRiffFileMode CurrentFileMode() const { return fmode; }
		DDCRET Open(const char * Filename, ExtRiffFileMode NewMode);
		DDCRET Close();

		std::int32_t CurrentFilePosition() const;

		template<typename X>
		DDCRET inline Read(X & x) { return Read(&x, sizeof x); }
		template<typename X>
		DDCRET inline Write(X const & x) { return Write(&x, sizeof x); }

		DDCRET Read(void *, std::uint32_t bytes); // Remember to fix endian if needed when you call this
		DDCRET Write(void const *, std::uint32_t bytes); // Remember to fix endian if needed when you call this
		DDCRET Expect(void const *, std::uint32_t bytes); // Remember to fix endian if needed when you call this

		/// Added by [JAZ]
		DDCRET Skip(std::int32_t NumBytes);

		DDCRET Backpatch(std::int32_t FileOffset, const void * Data, std::uint32_t NumBytes); // Remember to fix endian if needed when you call this
};

class WaveFormat_ChunkData {
	public:
		/// Format category (PCM=1)
		std::uint16_t wFormatTag;       
		/// Number of channels (mono=1, stereo=2)
		std::uint16_t nChannels;        
		/// Sampling rate [Hz]
		std::uint32_t nSamplesPerSec;   
		std::uint32_t nAvgBytesPerSec;
		std::uint16_t nBlockAlign;
		std::uint16_t nBitsPerSample;
		void Config(std::uint32_t NewSamplingRate = 44100, std::uint16_t NewBitsPerSample = 16, std::uint16_t NewNumChannels = 2) {
			nSamplesPerSec = NewSamplingRate;
			nChannels = NewNumChannels;
			nBitsPerSample = NewBitsPerSample;
			nAvgBytesPerSec = nChannels * nSamplesPerSec * nBitsPerSample / 8;
			nBlockAlign = nChannels * nBitsPerSample / 8;
		}
		WaveFormat_ChunkData() {
			wFormatTag = 1; // PCM
			Config();
		}
};


class WaveFormat_Chunk {
	public:
		ExtRiffChunkHeader header;
		WaveFormat_ChunkData data;
		WaveFormat_Chunk() {
			header.ckID = FourCC("fmt");
			header.ckSize = sizeof(WaveFormat_ChunkData);
		}
		bool VerifyValidity() {
			return
				header.ckID == FourCC("fmt") &&
				(data.nChannels == 1 || data.nChannels == 2) &&
				data.nAvgBytesPerSec == data.nChannels * data.nSamplesPerSec * data.nBitsPerSample / 8 &&
				data.nBlockAlign == data.nChannels * data.nBitsPerSample / 8;
		}
};

std::uint16_t const MAX_WAVE_CHANNELS = 2;

class WaveFileSample {
	public:
		std::int16_t chan[MAX_WAVE_CHANNELS];
};


/// riff wave file format.
/// MODIFIED BY [JAZ]. It was "private ExtRiffFile".
//UNIVERSALIS__COMPILER__DEPRECATED("c++ iostream for the better")
class WaveFile: public ExtRiffFile {
	WaveFormat_Chunk wave_format;
	ExtRiffChunkHeader pcm_data;
	/// offset of 'pcm_data' in output file
	std::uint32_t pcm_data_offset;
	std::uint32_t num_samples;

	public:
	WaveFile();

	DDCRET OpenForWrite(const char * Filename,
		std::uint32_t SamplingRate = 44100,
		std::uint16_t BitsPerSample = 16,
		std::uint16_t NumChannels = 2
	);

	DDCRET OpenForRead(const char * Filename);

	DDCRET ReadSample(std::int16_t Sample[MAX_WAVE_CHANNELS]);
	DDCRET WriteSample(const std::int16_t Sample[MAX_WAVE_CHANNELS]);
	DDCRET SeekToSample(std::uint32_t SampleIndex);

	/// work only with 16-bit audio
	DDCRET WriteData(const std::int16_t * data, std::uint32_t numData);
	/// work only with 16-bit audio
	DDCRET ReadData(std::int16_t * data, std::uint32_t numData);

	/// work only with unsigned 8-bit audio
	DDCRET WriteData(const std::uint8_t * data, std::uint32_t numData);
	/// work only with unsigned 8-bit audio
	DDCRET ReadData(std::uint8_t * data, std::uint32_t numData);

	DDCRET ReadSamples(std::uint32_t num, WaveFileSample[]);

	DDCRET WriteMonoSample(float ChannelData);
	DDCRET WriteStereoSample(float LeftChannelData, float RightChannelData);

	DDCRET ReadMonoSample(std::int16_t * ChannelData);
	DDCRET ReadStereoSample(std::int16_t * LeftSampleData, std::int16_t * RightSampleData);

	DDCRET Close();

	/// [Hz]
	std::uint32_t SamplingRate() const;
	std::uint16_t BitsPerSample() const;
	std::uint16_t NumChannels() const;
	std::uint32_t NumSamples() const;

	/// Open for write using another wave file's parameters...
	DDCRET OpenForWrite(const char * Filename, WaveFile & OtherWave) {
		return OpenForWrite(Filename, OtherWave.SamplingRate(), OtherWave.BitsPerSample(), OtherWave.NumChannels());
	}

	std::int32_t CurrentFilePosition() const {
		return ExtRiffFile::CurrentFilePosition();
	}
};

}}

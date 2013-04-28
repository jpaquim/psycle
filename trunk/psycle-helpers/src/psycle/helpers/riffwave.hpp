#pragma once
#include "msriff.hpp"
namespace psycle { namespace helpers {
	class WaveFormat_Data;

	/// the riff WAVE/fmt chunk.
	template<typename long_type, typename short_type>
	class RiffWaveFmtChunk  : public RiffChunkHeader<long_type>
	{
	public:
		static const uint16_t FORMAT_PCM=1;
		static const uint16_t FORMAT_FLOAT=3;
		static const uint16_t FORMAT_EXTENSIBLE=0xFFFEU;
		short_type wFormatTag;
		/// Number of channels (mono=1, stereo=2)
		short_type wChannels;
		/// Sampling rate [Hz]
		long_type dwSamplesPerSec;
		/// Indication of the amount of bytes required for one second of audio
		long_type dwAvgBytesPerSec;
		/// block align (i.e. bytes of one frame).
		short_type wBlockAlign;
		/// bits per sample (i.e. bits of one frame for a single channel)
		short_type wBitsPerSample;
	public:
		RiffWaveFmtChunk();
		RiffWaveFmtChunk(const WaveFormat_Data& config);
		virtual ~RiffWaveFmtChunk();
		virtual uint32_t length() const;
	};

	//todo: WAVEFORMAT_EXTENSIBLE
	template<typename long_type, typename short_type>
	class RiffWaveFmtChunkExtensible : RiffWaveFmtChunk<long_type, short_type>
	{
	public:
		RiffWaveFmtChunkExtensible();
		RiffWaveFmtChunkExtensible(const WaveFormat_Data& config);
		virtual ~RiffWaveFmtChunkExtensible();

	};

	class WaveFormat_Data {
	public:
		bool isfloat;
		uint16_t nChannels;
		uint32_t nSamplesPerSec;
		uint16_t nBitsPerSample;
		//TODO: WAVEFORMAT_EXTENSIBLE
	public:
		WaveFormat_Data() {
			Config();
		}
		WaveFormat_Data(uint32_t NewSamplingRate, uint16_t NewBitsPerSample, uint16_t NewNumChannels, bool isFloat) {
			Config(NewSamplingRate, NewBitsPerSample, NewNumChannels, isFloat);
		}

		bool operator==(const WaveFormat_Data& other) {
			return isfloat == other.isfloat
				&& nChannels == other.nChannels
				&& nSamplesPerSec == other.nSamplesPerSec
				&& nBitsPerSample == other.nBitsPerSample;
		}

		void Config(uint32_t NewSamplingRate = 44100, uint16_t NewBitsPerSample = 16, uint16_t NewNumChannels = 2, bool isFloat = false) {
			isfloat = isFloat;
			nSamplesPerSec = NewSamplingRate;
			nChannels = NewNumChannels;
			nBitsPerSample = NewBitsPerSample;
		}
		template<typename long_type, typename short_type>
		void Config(const RiffWaveFmtChunk<long_type,short_type>& chunk);

	};

/// usage: integerconverter<Long24BE,Long24LE,assignconverter<Long24BE, Long24LE>(bla,bla2,10);
template<typename in_type, typename out_type>
inline out_type assign24converter(in_type a) { out_type b(a.unsignedValue()); return b; }


/*********  IFF file reader comforming to IBM/Microsoft WaveRIFF specifications ****/
/* This one only supports up to 32bits (4GB) samples, plus support for reading 64bit RF64 */
class RiffWave : public MsRiff {
	RiffWave();
	virtual ~RiffWave();

	virtual void Open(const std::string& fname);
	virtual void Create(const std::string& fname, bool const overwrite, 
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		bool const littleEndian=true
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		bool const littleEndian=false
#endif
		);
	virtual void close();
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
	virtual const RiffChunkHeader<LongLE>& findChunk(const IffChunkId& id, bool allowWrap=false);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
	virtual const RiffChunkHeader<LongBE>& findChunk(const IffChunkId& id, bool allowWrap=false);
#endif

	const WaveFormat_Data& format() const {return formatdata; }
	uint32_t numSamples() const { return numsamples; }

	void SeekToSample(uint32_t sampleIndex);
	void addFormat(uint32_t SamplingRate, uint16_t BitsPerSample, uint16_t NumChannels, bool isFloat);
	
	void readInterleavedSamples(void* pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo=NULL);
	void readDeInterleavedSamples(void** pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo=NULL);

	void writeFromInterleavedSamples(void* pSamps, uint32_t maxSamples, WaveFormat_Data* convertFrom=NULL);
	void writeFromDeInterleavedSamples(void** pSamps, uint32_t maxSamples, WaveFormat_Data* convertFrom=NULL);
	
	static const IffChunkId RF64;
	static const IffChunkId WAVE;
	static const IffChunkId ds64;
	static const IffChunkId fmt;
	static const IffChunkId data;
protected:
	void ReadFormat();
	void readMonoSamples(void* pSamp, uint32_t samples);
	void readMonoConvertTo16(int16_t* pSamp, uint32_t samples);
	void readDeintMultichanSamples(void** pSamps, uint32_t samples);
	void readDeintMultichanConvertTo16(int16_t** pSamps, uint32_t samples);

	void writeMonoSamples(void* pSamp, uint32_t samples);
	void writeMonoConvertFrom16(int16_t* pSamp, uint32_t samples);
	void writeDeintConvertFrom16(int16_t** pSampL, void* pSampR, uint32_t samples);

	uint32_t calcSamplesFromBytes(uint32_t length);



	template<typename sample_type>
	inline void readDeinterleaveSamples(void** pSamps, uint16_t chans, uint32_t samples);
	template<typename sample_type>
	inline void readDeinterleaveSamples24(void** pSamps, uint16_t chans, uint32_t samples);

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void integerconverter(out_type* out, uint32_t samples);
	//Same as above, for multichan
	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void multichanintegerconverter(out_type** out, uint16_t chans, uint32_t samples);

	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void integer24converterA(out_type* out, uint32_t samples);
	//Same as above, but requiring endiannes conversion.
	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void integer24converterB(out_type* out, uint32_t samples);

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void floatconverter(out_type* out, uint32_t numsamples, double multi);

	WaveFormat_Data formatdata;

	std::streampos ds64_pos;
	std::streampos pcmdata_pos;
	uint32_t numsamples;

};
}}

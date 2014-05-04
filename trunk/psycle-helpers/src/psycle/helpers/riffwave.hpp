#pragma once
#include "msriff.hpp"
namespace psycle { namespace helpers {

	class WaveFormat_Data;

	/// the riff WAVE/fmt chunk.
	class RiffWaveFmtChunk
	{
	public:
		RiffWaveFmtChunk();
		RiffWaveFmtChunk(const WaveFormat_Data& config);
		virtual ~RiffWaveFmtChunk() {};
	public:
		static const IffChunkId fmt;
		static const uint16_t FORMAT_PCM;
		static const uint16_t FORMAT_FLOAT;
		static const uint16_t FORMAT_EXTENSIBLE;
		
		static const std::size_t SIZEOF;

		uint16_t wFormatTag;
		/// Number of channels (mono=1, stereo=2)
		uint16_t wChannels;
		/// Sampling rate [Hz]
		uint32_t dwSamplesPerSec;
		/// Indication of the amount of bytes required for one second of audio
		uint32_t dwAvgBytesPerSec;
		/// block align (i.e. bytes of one frame).
		uint16_t wBlockAlign;
		/// bits per sample (i.e. bits of one frame for a single channel)
		uint16_t wBitsPerSample;
	};

	//WAVEFORMAT_EXTENSIBLE
	typedef struct {
		uint32_t data1;
		uint16_t data2;
		uint16_t data3;
		char data4[8];
	} SubFormatTag_t;

	class RiffWaveFmtChunkExtensible : public RiffWaveFmtChunk
	{
	public:
		RiffWaveFmtChunkExtensible();
		RiffWaveFmtChunkExtensible(const RiffWaveFmtChunk& copy);
		RiffWaveFmtChunkExtensible(const WaveFormat_Data& config);
		virtual ~RiffWaveFmtChunkExtensible() {}

	public:
		static const std::size_t SIZEOF;

		uint16_t extensionSize;
		uint16_t numberOfValidBits;
		uint32_t speakerPositionMask;
		SubFormatTag_t subFormatTag;
	};

	class WaveFormat_Data {
	public:
		bool isfloat;
		uint16_t nChannels;
		uint32_t nSamplesPerSec;
		uint16_t nBitsPerSample;
		uint16_t nUsableBitsPerSample;
		//TODO: WAVEFORMAT_EXTENSIBLE
	public:
		WaveFormat_Data() {
			Config();
		}
		WaveFormat_Data(uint32_t NewSamplingRate, uint16_t NewBitsPerSample, uint16_t NewNumChannels, bool isFloat) {
			Config(NewSamplingRate, NewBitsPerSample, NewNumChannels, isFloat);
		}

		inline bool operator==(const WaveFormat_Data& other) const;

		void Config(uint32_t NewSamplingRate = 44100, uint16_t NewBitsPerSample = 16, uint16_t NewNumChannels = 2, bool useFloat = false);
		void Config(const RiffWaveFmtChunk& chunk);
		void Config(const RiffWaveFmtChunkExtensible& chunk);

	};
	bool WaveFormat_Data::operator==(const WaveFormat_Data& other) const {
		return isfloat == other.isfloat && nChannels == other.nChannels
			&& nSamplesPerSec == other.nSamplesPerSec
			&& nBitsPerSample == other.nBitsPerSample
			&& nUsableBitsPerSample == other.nUsableBitsPerSample;
	}



/*********  IFF file reader comforming to IBM/Microsoft WaveRIFF specifications ****/
/* This one only supports up to 32bits (4GB) samples, plus support for reading 64bit RF64 that have less than 2^32 sampleframes*/
class RiffWave : public MsRiff {
public:
	RiffWave();
	virtual ~RiffWave();

	virtual void Open(const std::string& fname);
	virtual void Create(const std::string& fname, bool overwrite, 
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		bool littleEndian=true
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		bool littleEndian=false
#endif
		);
	virtual void close();
	virtual const BaseChunkHeader& findChunk(const IffChunkId& id, bool allowWrap=false);

	const WaveFormat_Data& format() const {return formatdata; }
	uint32_t numSamples() const { return numsamples; }

	void SeekToSample(uint32_t sampleIndex);
	void addFormat(uint32_t SamplingRate, uint16_t BitsPerSample, uint16_t NumChannels, bool isFloat);
	
	void readInterleavedSamples(void* pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo=NULL);
	void readDeInterleavedSamples(void** pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo=NULL);

	void writeFromInterleavedSamples(void* pSamps, uint32_t samples, WaveFormat_Data* convertFrom=NULL);
	void writeFromDeInterleavedSamples(void** pSamps, uint32_t samples, WaveFormat_Data* convertFrom=NULL);
	
	static const IffChunkId RF64;
	static const IffChunkId WAVE;
	static const IffChunkId ds64;
	static const IffChunkId data;
	static const IffChunkId fact;
protected:
	void ReadFormat();
	void readMonoSamples(void* pSamp, uint32_t samples);
	void readDeintMultichanSamples(void** pSamps, uint32_t samples);

	template<typename out_type, out_type (*uint8conv)(uint8_t), out_type (*int16conv)(int16_t), out_type (*int24conv)(int32_t)
		, out_type (*int32conv)(int32_t), out_type (*floatconv)(float, double), out_type (*doubleconv)(double, double)>
	void readMonoConvertToInteger(out_type* pSamp, uint32_t samples, double multi);

	template<typename out_type, out_type (*uint8conv)(uint8_t), out_type (*int16conv)(int16_t), out_type (*int24conv)(int32_t)
		, out_type (*int32conv)(int32_t), out_type (*floatconv)(float, double), out_type (*doubleconv)(double, double)>
	void readDeintMultichanConvertToInteger(out_type** pSamps, uint32_t samples, double multi);


	void writeMonoSamples(void* pSamp, uint32_t samples);
	void writeDeintMultichanSamples(void** pSamps, uint32_t samples);

	uint32_t calcSamplesFromBytes(uint32_t length);



	template<typename sample_type>
	inline void readDeinterleaveSamples(void** pSamps, uint16_t chans, uint32_t samples);
	template<typename file_type, typename platform_type>
	inline void readDeinterleaveSamplesendian(void** pSamps, uint16_t chans, uint32_t samples);

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void ReadWithintegerconverter(out_type* out, uint32_t samples);
	//Same as above, for multichan
	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void ReadWithmultichanintegerconverter(out_type** out, uint16_t chans, uint32_t samples);
	//Same as above, but for endiantypes.
	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void ReadWithinteger24converter(out_type* out, uint32_t samples);
	//Same as above, for multichan
	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void ReadWithmultichaninteger24converter(out_type** out, uint16_t chans, uint32_t samples);


	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void ReadWithfloatconverter(out_type* out, uint32_t numsamples, double multi);
	//Same as above, for multichan
	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void ReadWithmultichanfloatconverter(out_type** out, uint16_t chans, uint32_t numsamples, double multi);


	WaveFormat_Data formatdata;

	std::streampos ds64_pos;
	std::streampos pcmdata_pos;
	uint32_t numsamples;

};

	//Templates to use with RiffWave class.
	///////////////////////////////////////
	template<typename out_type, out_type (*uint8conv)(uint8_t), out_type (*int16conv)(int16_t), out_type (*int24conv)(int32_t)
		, out_type (*int32conv)(int32_t), out_type (*floatconv)(float, double), out_type (*doubleconv)(double, double)>
	void RiffWave::readMonoConvertToInteger(out_type* pSamp, uint32_t samples, double multi)
	{
		switch(formatdata.nBitsPerSample)
		{
			case 8: ReadWithintegerconverter<uint8_t,out_type,uint8conv>(pSamp, samples);break;
			case 16: ReadWithintegerconverter<int16_t,out_type,int16conv>(pSamp, samples);break;
			case 24: {
				if (isLittleEndian) {
					ReadWithinteger24converter<Long24LE, out_type, int24conv>(pSamp,samples);
				}
				else {
					ReadWithinteger24converter<Long24BE, out_type, int24conv>(pSamp,samples);
				}
				break;
				}
			case 32: {
				if (formatdata.isfloat) {
					ReadWithfloatconverter<float,out_type,floatconv>(pSamp, samples,multi);
				}
				else {
					//TODO: Verify that this works for 24-in-32bits (WAVERFORMAT_EXTENSIBLE)
					ReadWithintegerconverter<int32_t,out_type,int32conv>(pSamp, samples);
				}
				break;
			}
			case 64: ReadWithfloatconverter<double,out_type,doubleconv>(pSamp, samples,multi); break;
			default: break; ///< \todo should throw an exception
		}
	}

	template<typename out_type, out_type (*uint8conv)(uint8_t), out_type (*int16conv)(int16_t), out_type (*int24conv)(int32_t)
		, out_type (*int32conv)(int32_t), out_type (*floatconv)(float, double), out_type (*doubleconv)(double, double)>
	void RiffWave::readDeintMultichanConvertToInteger(out_type** pSamps, uint32_t samples, double multi)
	{
		switch(formatdata.nBitsPerSample)
		{
			case 8:
				ReadWithmultichanintegerconverter<uint8_t,out_type,uint8conv>(pSamps, formatdata.nChannels, samples);
				break;
			case 16:
				ReadWithmultichanintegerconverter<int16_t,out_type,int16conv>(pSamps, formatdata.nChannels, samples);
				break;
			case 24:
				if (isLittleEndian) {
					ReadWithmultichaninteger24converter<Long24LE, out_type, int24conv>(pSamps,formatdata.nChannels,samples);
				}
				else {
					ReadWithmultichaninteger24converter<Long24BE, out_type, int24conv>(pSamps,formatdata.nChannels,samples);
				}
				break;
			case 32: {
				if (formatdata.isfloat) {
					ReadWithmultichanfloatconverter<float,out_type,floatconv>(pSamps, formatdata.nChannels, samples,multi);
				}
				else {
					//TODO: Verify that this works for 24-in-32bits (WAVERFORMAT_EXTENSIBLE)
					ReadWithmultichanintegerconverter<int32_t,out_type,int32conv>(pSamps, formatdata.nChannels, samples);
				}
				break;
			}
			case 64: ReadWithmultichanfloatconverter<double,out_type,doubleconv>(pSamps, formatdata.nChannels, samples,multi); break;
			default: break; ///< \todo should throw an exception
		}
	}



	template<typename sample_type>
	inline void RiffWave::readDeinterleaveSamples(void** pSamps, uint16_t chans, uint32_t samples) {
		sample_type** samps = reinterpret_cast<sample_type**>(pSamps);
		ReadWithmultichanintegerconverter<sample_type, sample_type,assignconverter<sample_type,sample_type> >(samps, chans, samples);
	}
	template<typename file_type, typename platform_type>
	inline void RiffWave::readDeinterleaveSamplesendian(void** pSamps, uint16_t chans, uint32_t samples) {
		platform_type** samps = reinterpret_cast<platform_type**>(pSamps);
		ReadWithmultichanintegerconverter<file_type, platform_type,endianessconverter<file_type,platform_type> >(samps, chans, samples);
	}

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void RiffWave::ReadWithintegerconverter(out_type* out, uint32_t samples)
	{
		in_type samps[32768];
		std::size_t amount=0;
		for(std::size_t io = 0; io < samples; io+=amount) {
			amount = std::min(static_cast<std::size_t>(32768U),samples-io);
			ReadArray(samps,amount);
			in_type* psamps = samps;
			for(std::size_t b = 0 ; b < amount; ++b) {
				*out=converter_func(*psamps);
				out++;
				psamps++;
			}
		}
	}
	//Same as above, for multichan, deinterlaced
	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void RiffWave::ReadWithmultichanintegerconverter(out_type** out, uint16_t chans, uint32_t samples)
	{
		in_type samps[32768];
		std::size_t amount=0;
		for(uint32_t io = 0 ; io < samples ; io+=amount)
		{
			amount = std::min(static_cast<std::size_t>(32768U)/chans,samples-io);
			ReadArray(samps, amount*chans);
			in_type* psamps = samps;
			for (int a=0; a < amount; a++) {
				for (int b=0; b < chans; b++) {
					out[b][io+a]=converter_func(*psamps);
					psamps++;
				}
			}
		}
	}

	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void RiffWave::ReadWithinteger24converter(out_type* out, uint32_t samples)
	{
		in_type samps[32768];
		std::size_t amount=0;
		for(uint32_t io = 0 ; io < samples ; io+=amount)
		{
			amount = std::min(static_cast<std::size_t>(32768U),samples-io);
			ReadArray(samps, amount);
			in_type* psamps = samps;
			for(std::size_t b = 0 ; b < amount; ++b) {
				*out=converter_func(psamps->signedValue());
				out++;
				psamps++;
			}
		}
	}

	//Same as above, but for multichan, deinterlaced
	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void RiffWave::ReadWithmultichaninteger24converter(out_type** out, uint16_t chans, uint32_t samples)
	{
		in_type samps[32768];
		std::size_t amount=0;
		for(uint32_t io = 0 ; io < samples ; io+=amount)
		{
			amount = std::min(static_cast<std::size_t>(32768U)/chans,samples-io);
			ReadArray(samps, amount*chans);
			in_type* psamps = samps;
			for (int a=0; a < amount; a++) {
				for (int b=0; b < chans; b++) {
					out[b][io+a]=converter_func(psamps->signedValue());
					psamps++;
				}
			}
		}
	}



	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void RiffWave::ReadWithfloatconverter(out_type* out, uint32_t numsamples, double multi) {
		in_type samps[32768];
		std::size_t amount=0;
		for(std::size_t io = 0; io < numsamples; io+=amount) {
			amount = std::min(static_cast<std::size_t>(32768U),numsamples-io);
			ReadArray(samps,amount);
			in_type* psamps = samps;
			for(std::size_t b = 0 ; b < amount; ++b) {
				*out=converter_func(*psamps, multi);
				out++;
				psamps++;
			}
		}
	}

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void RiffWave::ReadWithmultichanfloatconverter(out_type** out, uint16_t chans, uint32_t numsamples, double multi) {
		in_type samps[32768];
		std::size_t amount=0;
		for(std::size_t io = 0; io < numsamples; io+=amount) {
			amount = std::min(static_cast<std::size_t>(32768U)/chans,numsamples-io);
			ReadArray(samps,amount*chans);
			in_type* psamps = samps;
			for (int a=0; a < amount; a++) {
				for (int b=0; b < chans; b++) {
					out[b][io+a]=converter_func(*psamps, multi);
					psamps++;
				}
			}
		}
	}

}}

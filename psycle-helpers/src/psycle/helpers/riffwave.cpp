#include "riffwave.hpp"
#include "sampleconverter.hpp"
namespace psycle { namespace helpers {

	const IffChunkId RiffWave::RF64 = {'R','F','6','4'};
	const IffChunkId RiffWave::WAVE = {'W','A','V','E'};
	const IffChunkId RiffWave::ds64 = {'d','s','6','4'};
	const IffChunkId RiffWave::fmt = {'f','m','t',' '};
	const IffChunkId RiffWave::data = {'d','a','t','a'};


	// TODO No idea how to do this in a nicer way with the gnu toolchain
    namespace {
		constexpr uint16_t FORMAT_PCM = 1;
		constexpr uint16_t FORMAT_FLOAT = 3;
		constexpr uint16_t FORMAT_EXTENSIBLE = 0xFFFEU;
    }
    template class RiffWaveFmtChunk<LongBE, ShortBE>;
    template<> const uint16_t RiffWaveFmtChunk<LongBE, ShortBE>::FORMAT_PCM = FORMAT_PCM;
    template<> const uint16_t RiffWaveFmtChunk<LongBE, ShortBE>::FORMAT_FLOAT = FORMAT_FLOAT;
    template<> const uint16_t RiffWaveFmtChunk<LongBE, ShortBE>::FORMAT_EXTENSIBLE = FORMAT_EXTENSIBLE;
    template class RiffWaveFmtChunk<LongLE, ShortLE>;
    template<> const uint16_t RiffWaveFmtChunk<LongLE, ShortLE>::FORMAT_PCM = FORMAT_PCM;
    template<> const uint16_t RiffWaveFmtChunk<LongLE, ShortLE>::FORMAT_FLOAT = FORMAT_FLOAT;
    template<> const uint16_t RiffWaveFmtChunk<LongLE, ShortLE>::FORMAT_EXTENSIBLE = FORMAT_EXTENSIBLE;

	//////////////////////////////////////////////

	RiffWave::RiffWave() : ds64_pos(0),pcmdata_pos(0),numsamples(0) {
	}

	RiffWave::~RiffWave() {
	}
	void RiffWave::Open(const std::string& fname) {
		MsRiff::Open(fname);
		if (isValid) {
			if (Expect(WAVE)) {
				isValid=true;
				ReadFormat();
			}
			else {
				isValid=false;
			}
		}
		else {
			MsRiff::close();
			AbstractIff::Open(fname);
			const BaseChunkHeader& header = readHeader();
			if (header.matches(RF64)) {
				isLittleEndian=true;
				if (Expect(WAVE)) {
					isValid=true;
					findChunk(ds64);
					ds64_pos = headerPosition;
					Skip(16); // skip riff and data sizes
					Long64LE sample;
					Read(sample);
					numsamples=sample.unsignedValue();
					skipThisChunk();
					ReadFormat();
				}
			}
		}
	}
    void RiffWave::Create(const std::string& fname, bool const  overwrite, bool const /*littleEndian*/) {
		MsRiff::Create(fname, overwrite);
		Write(WAVE);
		ds64_pos = 0; pcmdata_pos = 0; numsamples = 0;
	}
	void RiffWave::close() { 
		if (isWriteMode()) {
			std::streamsize size;
			if (currentHeader.matches(data)) {
				size = GetPos() - headerPosition;
			}
			else {
				//TODO: would need a mechanism to find the chunk and determine where the next one is.
				size=0;
			}
			UpdateFormSize(0,size-sizeof(currentHeader));
		}
		MsRiff::close();
	}

#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
	const RiffChunkHeader<LongLE>& RiffWave::findChunk(const IffChunkId& id, bool allowWrap)
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
	const RiffChunkHeader<LongBE>& RiffWave::findChunk(const IffChunkId& id, bool allowWrap)
#endif
	{
		do {
			readHeader();
			if (currentHeader.matches(data)) {
				pcmdata_pos = headerPosition;
				if (ds64_pos == std::streampos(0) ) {
					numsamples = calcSamplesFromBytes(currentHeader.length());
				}
			}
			if (currentHeader.matches(id)) {
				return currentHeader;
			}
			Skip(currentHeader.length());
			if (currentHeader.length()%2)Skip(1);
		}while(!Eof());
		if (allowWrap && Eof()) {
			Seek(12);//Skip RIFF/WAVE header.
			return findChunk(id,false);
		}
		throw std::runtime_error(std::string("couldn't find chunk ") + id);
	}

	void RiffWave::ReadFormat()
	{
		findChunk(fmt);
		if (isLittleEndian) {
			RiffWaveFmtChunk<LongLE,ShortLE> chunk;
			ReadRaw(&chunk,sizeof(RiffWaveFmtChunk<LongLE,ShortLE>));
			formatdata.Config(chunk);
			skipThisChunk();
		}
		else {
			RiffWaveFmtChunk<LongBE,ShortBE> chunk;
			ReadRaw(&chunk,sizeof(RiffWaveFmtChunk<LongBE,ShortBE>));
			formatdata.Config(chunk);
			skipThisChunk();
		}
		//TODO: waveformat_Extensible
		/*
 if (wav->formatTag == WAVE_FORMAT_EXTENSIBLE)
    {
      uint16_t extensionSize;
      uint16_t numberOfValidBits;
      uint32_t speakerPositionMask;
      uint16_t subFormatTag;
      uint8_t dummyByte;
      int i;

      if (wFmtSize < 18)
      {
        lsx_fail_errno(ft,SOX_EHDR,"WAVE file fmt chunk is too short");
        return SOX_EOF;
      }
      lsx_readw(ft, &extensionSize);
      len -= 2;
      if (extensionSize < 22)
      {
        lsx_fail_errno(ft,SOX_EHDR,"WAVE file fmt chunk is too short");
        return SOX_EOF;
      }
      lsx_readw(ft, &numberOfValidBits);
      lsx_readdw(ft, &speakerPositionMask);
      lsx_readw(ft, &subFormatTag);
      for (i = 0; i < 14; ++i) lsx_readb(ft, &dummyByte);
      len -= 22;
      if (numberOfValidBits != wBitsPerSample)
      {
        lsx_fail_errno(ft,SOX_EHDR,"WAVE file fmt with padded samples is not supported yet");
        return SOX_EOF;
      }
      wav->formatTag = subFormatTag;
      lsx_report("EXTENSIBLE");
    }
		*/
	}
	void RiffWave::addFormat(uint32_t SamplingRate, uint16_t BitsPerSample, uint16_t NumChannels, bool isFloat)
	{
		formatdata.Config(SamplingRate, BitsPerSample, NumChannels, isFloat);
		pcmdata_pos = GetPos();
		if (isLittleEndian) {
			RiffChunkHeader<LongLE> header(fmt,sizeof(RiffWaveFmtChunk<LongLE,ShortLE>));
			addNewChunk(header);
			RiffWaveFmtChunk<LongLE,ShortLE> chunk(formatdata);
			WriteRaw(&chunk,sizeof(RiffWaveFmtChunk<LongLE,ShortLE>));
		}
		else {
			RiffChunkHeader<LongBE> header(fmt,sizeof(RiffWaveFmtChunk<LongBE,ShortBE>));
			addNewChunk(header);
			RiffWaveFmtChunk<LongBE,ShortBE> chunk(formatdata);
			WriteRaw(&chunk,sizeof(RiffWaveFmtChunk<LongBE,ShortBE>));
		}
	}

	void RiffWave::SeekToSample(uint32_t sampleIndex)
	{
		if (sampleIndex < numsamples ) {
			std::streampos size = formatdata.nChannels * ((formatdata.nBitsPerSample + 7) / 8);
			Seek(pcmdata_pos + std::streamoff(sizeof(currentHeader) + size * sampleIndex));
		}
	}
	uint32_t RiffWave::calcSamplesFromBytes(uint32_t length)
	{
		return length/( formatdata.nChannels* ((formatdata.nBitsPerSample + 7) / 8));
	}


	void RiffWave::readInterleavedSamples(void* pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo)
	{
		if (!currentHeader.matches(data)) {
			skipThisChunk();
			findChunk(data, true);
		}
		//TODO: max take into account the offset into data chunk.
		uint32_t max = (maxSamples>0) ? maxSamples : numsamples;
		if (convertTo == NULL || *convertTo == formatdata)
		{
			readMonoSamples(pSamps, max*formatdata.nChannels);
		}
		else if (convertTo->nChannels == formatdata.nChannels) {
			//TODO
			switch(convertTo->nBitsPerSample) 
			{
			case 8:break;
			case 16:readMonoConvertTo16(static_cast<int16_t*>(pSamps),max*formatdata.nChannels);break;
			case 24:break;
			case 32:break;
			default:break;
			}
		}
		else if (convertTo->nChannels == 1) {
			//TODO
		}
		else {
			//TODO
		}
	}
	void RiffWave::readDeInterleavedSamples(void** pSamps, uint32_t maxSamples, WaveFormat_Data* convertTo)
	{
		if (!currentHeader.matches(data)) {
			skipThisChunk();
			findChunk(data, true);
		}
		//TODO: max take into account the offset into data chunk.
		uint32_t max = (maxSamples>0) ? maxSamples : numsamples;
		if (convertTo == NULL || *convertTo == formatdata)
		{
			if (formatdata.nChannels==1) {
				readMonoSamples(pSamps[0],max);
			}
			else {
				readDeintMultichanSamples(pSamps, max);
			}
		}
		else if (convertTo->nChannels == formatdata.nChannels) {
			//TODO
			switch(convertTo->nBitsPerSample) 
			{
			case 8:break;
			case 16:readDeintMultichanConvertTo16(reinterpret_cast<int16_t**>(pSamps),max);break;
			case 24:break;
			case 32:break;
			default:break;
			}
		}
		else if (convertTo->nChannels == 1) {
			//TODO
		}
		else {
			//TODO
		}
	}


	void RiffWave::readMonoSamples(void* pSamp, uint32_t samples)
	{
		switch(formatdata.nBitsPerSample)
		{
			case 8: {
				uint8_t* smp8 = static_cast<uint8_t*>(pSamp);
				ReadArray(smp8, samples);
				break;
			}
			case 16: {
				int16_t* smp16 = static_cast<int16_t*>(pSamp);
				ReadArray(smp16, samples);
				break;
			}
			case 24: {
				if (isLittleEndian) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
					Long24LE* samps = static_cast<Long24LE*>(pSamp);
					ReadArray(samps,samples);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
					Long24BE* samps = static_cast<Long24BE*>(pSamp);
					integerconverter<Long24LE,Long24BE,assign24converter<Long24LE,Long24BE> >(samps, samples);
#endif
				}
				else {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
					Long24LE* samps = static_cast<Long24LE*>(pSamp);
					//There's no need to use the integer24converter here because we can use the operator=().
					integerconverter<Long24BE,Long24LE,assign24converter<Long24BE,Long24LE> >(samps, samples);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
					Long24BE* samps = static_cast<Long24BE*>(pSamp);
					ReadArray(samps,samples);
#endif
				}
				break;
			}
			case 32: {
				//TODO: WAVERFORMAT_EXTENSIBLE for 24bit samples
				if (formatdata.isfloat) {
					float* smp32 = static_cast<float*>(pSamp);
					ReadArray(smp32, samples);
				}
				else {
					int32_t* smp32 = static_cast<int32_t*>(pSamp);
					ReadArray(smp32, samples);
				}
				break;
			}
			case 64: {
				double* smp32 = static_cast<double*>(pSamp);
				ReadArray(smp32, samples);
				break;
			}
			default:
				break;
		}
	}

	void RiffWave::readMonoConvertTo16(int16_t* pSamp, uint32_t samples)
	{
		switch(formatdata.nBitsPerSample)
		{
			case 8: integerconverter<uint8_t,int16_t,uint8toint16>(pSamp, samples);break;
			case 16: ReadArray(pSamp, samples); break;
			case 24: {
				if (isLittleEndian) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
					integer24converterA<Long24LE, int16_t, int24toint16>(pSamp,samples);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
					integer24converterB<Long24LE, int16_t, int24toint16>(pSamp,samples);
#endif
				}
				else {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
					integer24converterB<Long24BE, int16_t, int24toint16>(pSamp,samples);
#elif defined DIVERSALIS__CPU__ENDIAN__LITTLE
					integer24converterA<Long24BE, int16_t, int24toint16>(pSamp,samples);
#endif
				}
				break;
				}
			case 32: {
				//TODO: WAVERFORMAT_EXTENSIBLE for 24bit samples
				if (formatdata.isfloat) {
					//TODO: float range
					floatconverter<float,int16_t,floattoint16>(pSamp, samples, 1.f);
				}
				else {
					integerconverter<int32_t,int16_t,int32toint16>(pSamp, samples);
				}
				break;
			}
			case 64: //TODO:
				break;
			default:
				break;
		}
	}

	void RiffWave::readDeintMultichanSamples(void** pSamps, uint32_t samples)
	{
		switch (formatdata.nBitsPerSample) {
			case  8: readDeinterleaveSamples<uint8_t>(pSamps,formatdata.nChannels, samples);break;
			case 16: readDeinterleaveSamples<int16_t>(pSamps,formatdata.nChannels, samples);break;
			case 24: {
				if (isLittleEndian) {
					readDeinterleaveSamples24<Long24LE>(pSamps,formatdata.nChannels, samples);
				}
				else {
					readDeinterleaveSamples24<Long24BE>(pSamps,formatdata.nChannels, samples);
				}
				break;
			}
			case 32: {
				//TODO: WAVEFORMAT_EXTENSIBLE
				if (formatdata.isfloat) {
					readDeinterleaveSamples<float>(pSamps,formatdata.nChannels, samples);
				}
				else {
					readDeinterleaveSamples<int32_t>(pSamps,formatdata.nChannels, samples);
				}
				break;
			}
		}
	}
	void RiffWave::readDeintMultichanConvertTo16(int16_t** pSamps, uint32_t samples)
	{
		switch(formatdata.nBitsPerSample)
		{
			case 8:
				multichanintegerconverter<uint8_t,int16_t,uint8toint16>(pSamps, formatdata.nChannels, samples);
				break;
			case 16:
				readDeinterleaveSamples<int16_t>(reinterpret_cast<void**>(pSamps),formatdata.nChannels, samples);
				break;
			case 24:
				//TODO
				if (isLittleEndian) {
				}
				else {
				}
				break;
			case 32: {
				//TODO: WAVEFORMAT_EXTENSIBLE
				if (formatdata.isfloat) {
				}
				else {
					multichanintegerconverter<int32_t,int16_t,int32toint16>(pSamps, formatdata.nChannels, samples);
				}
				break;
			}
			default:
				break; ///< \todo should throw an exception
		}
	}


    void RiffWave::writeFromInterleavedSamples(void* /*pSamps*/, uint32_t /*maxSamples*/, WaveFormat_Data* /*convertFrom*/)
	{
		if (!currentHeader.matches(data)) {
			pcmdata_pos = GetPos();
			if (isLittleEndian) {
				RiffChunkHeader<LongLE> header(data,0);
				addNewChunk(header);
			}
			else {
				RiffChunkHeader<LongBE> header(data,0);
				addNewChunk(header);
			}
		}
	}
    void RiffWave::writeFromDeInterleavedSamples(void** /*pSamps*/, uint32_t /*maxSamples*/, WaveFormat_Data* /*convertFrom*/)
	{
		if (!currentHeader.matches(data)) {
			pcmdata_pos = GetPos();
			if (isLittleEndian) {
				RiffChunkHeader<LongLE> header(data,0);
				addNewChunk(header);
			}
			else {
				RiffChunkHeader<LongBE> header(data,0);
				addNewChunk(header);
			}
		}
	}



    void RiffWave::writeMonoSamples(void* /*pSamp*/, uint32_t /*samples*/) {
	}
    void RiffWave::writeMonoConvertFrom16(int16_t* /*pSamp*/, uint32_t /*samples*/) {
	}
    void RiffWave::writeDeintConvertFrom16(int16_t** /*pSampL*/, void* /*pSampR*/, uint32_t /*samples*/) {
	}


#if 0

	{
if(hd.ckID == FourCC("smpl"))
				{
					char pl(0);
					file.Skip(28);
					file.Read(static_cast<void*>(&pl), 1);
					if(pl == 1)
					{
						file.Skip(15);
						std::uint32_t ls(0);
						std::uint32_t le(0);
						file.Read(static_cast<void*>(&ls), 4);
						file.Read(static_cast<void*>(&le), 4);
						if (ls >= 0 && ls <= le && le < Datalen){
							wave.WaveLoopStart(ls);
							wave.WaveLoopEnd(le);
							wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
						}
					}
					file.Skip(9);
	}
#endif


		//Templates to use with RiffWave class.

	template<typename sample_type>
	inline void RiffWave::readDeinterleaveSamples(void** pSamps, uint16_t chans, uint32_t samples) {
		sample_type** samps = reinterpret_cast<sample_type**>(pSamps);
		multichanintegerconverter<sample_type, sample_type,assignconverter<sample_type,sample_type> >(samps, chans, samples);
	}
	template<typename sample_type>
	inline void RiffWave::readDeinterleaveSamples24(void** pSamps, uint16_t chans, uint32_t samples) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		Long24LE** samps = reinterpret_cast<Long24LE**>(pSamps);
		multichanintegerconverter<sample_type, Long24LE,assign24converter<sample_type,Long24LE> >(samps, chans, samples);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		Long24BE** samps = reinterpret_cast<Long24BE**>(pSamps);
		multichanintegerconverter<sample_type, Long24BE,assign24converter<sample_type,Long24BE> >(samps, chans, samples);
#endif
	}

	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void RiffWave::integerconverter(out_type* out, uint32_t samples)
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
	//Same as above, for multichan
	//Same as above, for multichan, deinterlaced
	template<typename in_type, typename out_type, out_type (*converter_func)(in_type)>
	void RiffWave::multichanintegerconverter(out_type** out, uint16_t chans, uint32_t samples)
	{
		in_type samps[32768];
		uint32_t amount=0;
		for(uint32_t io = 0 ; io < samples ; io+=amount)
		{
			//truncate 1024 to amount of chans.
			amount = std::min(32768U/chans,samples-io);
			ReadArray(samps, amount*chans);
			in_type* psamps = samps;
			for (int a=0; a < amount; a++) {
				for (int b=0; b < chans; b++) {
					out[b][a]=converter_func(*psamps);
					psamps++;
				}
			}
		}
	}

	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void RiffWave::integer24converterA(out_type* out, uint32_t samples)
	{
		in_type bufpacked[4];
		int32_t bufunpacked[4];
		uint32_t io = 0;
		for(; io+4 < samples; io+=4)
		{
			ReadArray(bufpacked,4);
			unpackint24(reinterpret_cast<int32_t*>(bufpacked),bufunpacked);
			out[0]=converter_func(bufunpacked[0]);
			out[1]=converter_func(bufunpacked[1]);
			out[2]=converter_func(bufunpacked[2]);
			out[3]=converter_func(bufunpacked[3]);
			out+=4;
		}
		if (io < samples) {
			ReadArray(bufpacked,samples-io);
			unpackint24(reinterpret_cast<int32_t*>(bufpacked),bufunpacked);
			for(int i=0; io < samples; i++, io++) {
				out[i]=converter_func(bufunpacked[i]);
			}
		}
	}
	//Same as above, but requiring endiannes conversion.
	template<typename in_type, typename out_type, out_type (*converter_func)(int32_t)>
	void RiffWave::integer24converterB(out_type* out, uint32_t samples)
	{
		in_type bufpacked[4];
		uint32_t io = 0;
		for(; io+4 < samples; io+=4)
		{
			ReadArray(bufpacked,4);
			out[0]=converter_func(bufpacked[0].signedValue());
			out[1]=converter_func(bufpacked[1].signedValue());
			out[2]=converter_func(bufpacked[2].signedValue());
			out[3]=converter_func(bufpacked[3].signedValue());
			out+=4;
		}
		if (io < samples) {
			ReadArray(bufpacked,(samples-io));
			for(int i=0; io < samples; i++, io++) {
				out[i]=converter_func(bufpacked[i].signedValue());
			}
		}
	}


	template<typename in_type, typename out_type, out_type (*converter_func)(in_type, double)>
	void RiffWave::floatconverter(out_type* out, uint32_t numsamples, double multi) {
		in_type samps[1024];
		std::size_t amount=0;
		for(std::size_t io = 0; io < numsamples; io+=amount) {
			amount = std::min(static_cast<std::size_t>(1024U),numsamples-io);
			ReadArray(samps,amount);
			in_type* psamps = samps;
			for(std::size_t b = 0 ; b < amount; ++b) {
				*out=converter_func(*psamps, multi);
				out++;
				psamps++;
			}
		}
	}


}}

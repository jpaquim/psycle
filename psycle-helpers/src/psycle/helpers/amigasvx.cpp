#include "amigasvx.hpp"

namespace psycle
{
	namespace helpers
	{
		AmigaSvx::AmigaSvx() {
		}
		AmigaSvx::~AmigaSvx() {
		}


#if 0
		//TODO
		{
			//In big endian:
			typedef struct
			{	unsigned int	oneShotHiSamples, repeatHiSamples, samplesPerHiCycle ;
				unsigned short	samplesPerSec ;
				unsigned char	octave, compression ;
				unsigned int	volume ;
			} VHDR_CHUNK ;

			RiffFile file;
			RiffChunkHeader hd;
			ULONG data;
			ULONGINV tmp;
			int bits = 0;
			// opens the file and reads the "FORM" header.
			if(!file.Open(const_cast<char*>(str)))
			{
				return 0;
			}
			XMInstrument::WaveData wave;
			unsigned int Datalen=0;
			file.Read(&data,4);
			if( data == file.FourCC("16SV")) bits = 16;
			else if(data == file.FourCC("8SVX")) bits = 8;
			file.Read(&hd,sizeof hd);
			if(hd._id == file.FourCC("NAME"))
			{
				char tmp[23];
				file.Read(tmp, 22); ///\todo should be hd._size instead of "22", but it is incorrectly read.
				tmp[22]='\0';
				wave.WaveName(tmp);
				if (instrument < PREV_WAV_INS){
					strncpy(_pInstrument[instrument]->_sName,str,31);
					_pInstrument[instrument]->_sName[31]='\0';
				}
				file.Read(&hd,sizeof hd);
			}
			if ( hd._id == file.FourCC("VHDR"))
			{
				unsigned int ls, le;
				file.Read(&tmp,sizeof tmp);
				Datalen = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(&tmp,sizeof tmp);
				ls = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				file.Read(&tmp,sizeof tmp);
				le = (tmp.hihi << 24) + (tmp.hilo << 16) + (tmp.lohi << 8) + tmp.lolo;
				if(bits == 16)
				{
					Datalen >>= 1;
					ls >>= 1;
					le >>= 1;
				}
				if(ls != le)
				{
					wave.WaveLoopStart(ls);
					wave.WaveLoopEnd(ls + le);
					wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
				}
				unsigned short srbe, sr;
				file.Read(&srbe,sizeof srbe);
				sr = (srbe&0xFF)<<8 | ((srbe&0xFF00) >> 8);
				wave.WaveSampleRate(sr);
 				file.Skip(6); // Skipping octave, compression and volume
				file.Read(&hd,sizeof hd);
			}

			if(hd._id == file.FourCC("BODY"))
			{
				std::int16_t * csamples;
				wave.AllocWaveData(Datalen,false);
				csamples = const_cast<std::int16_t*>(wave.pWaveDataL());
				if(bits == 16)
				{
					for(unsigned int smp(0) ; smp < Datalen; ++smp)
					{
						file.Read(&tmp, 2);
						*csamples = tmp.hilo * 256 + tmp.hihi;
						++csamples;
					}
				}
				else
				{
					for(unsigned int smp(0) ; smp < Datalen; ++smp)
					{
						file.Read(&tmp, 1);
						*csamples = tmp.hihi * 256 + tmp.hihi;
						++csamples;
					}
				}
			}
		}
#endif
	}
}


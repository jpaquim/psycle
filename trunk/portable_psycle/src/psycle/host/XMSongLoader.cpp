/** @file 
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#include <project.private.hpp>
/*#include "NewMachine.h"
#include "MainFrm.h"
#if defined WTL
	#include "PsycleWTLView.h"
#else
	#include "ChildView.h"
#endif
*/
#include "ProgressDialog.hpp"

//	extern CPsycleApp theApp;
#include "Song.hpp"
//#include "IPsySongLoader.h"
//#include "Instrument.h"
#include "Machine.hpp" // It wouldn't be needed, since it is already included in "song.h"
//#include "Sampler.h"
#include "XMInstrument.hpp"
#include "XMSampler.hpp"
#include "XMSongLoader.hpp"
//#include "Plugin.h"
//#include "VSTHost.h"
//#include "DataCompression.h"

//#include <sstream>

#ifdef CONVERT_INTERNAL_MACHINES
	#include "convert_internal_machines.hpp" // conversion
#endif

#include "Riff.hpp"	 // For Wave file loading.


namespace psycle{
namespace host{

	static const char * XM_HEADER = "extended module: ";
	
	static const UCHAR VOLCMD_EXG_TABLE[16] = 
	{
		0,
		XMSampler::CMD::VOLUME,
		XMSampler::CMD::VOLUME,
		XMSampler::CMD::VOLUME,
		XMSampler::CMD::VOLUME,
		XMSampler::CMD::VOLUME,
		XMSampler::CMD::VOLSLIDEDOWN,
		XMSampler::CMD::VOLSLIDEUP,
		XMSampler::CMD::FINEVOLDOWN,
		XMSampler::CMD::FINEVOLUP,
		XMSampler::CMD::VIBRATO_SPEED,
		XMSampler::CMD::VIBRATO_DEPTH,
		XMSampler::CMD::PANNING,
		XMSampler::CMD::PANSLIDELEFT,
		XMSampler::CMD::PANSLIDERIGHT,
		XMSampler::CMD::PORTA2NOTE
	};

	XMSongLoader::XMSongLoader(void)
	{
	
	}

	XMSongLoader::~XMSongLoader(void)
	{

	}
	
	void XMSongLoader::Load(Song& song,const bool fullopen)
	{

		// check validity
		if(!IsValid()){
			return;
		}

		// clear existing data
		song.DeleteAllPatterns();

		song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, _T(""),0);
		song.InsertConnection(0,MASTER_INDEX,0.5f);
		song.seqBus=0;
		// build sampler
		m_pSampler = (XMSampler *)(song._pMachine[0]);

		LONG iInstrStart = LoadPatterns(song);

		m_pSampler->BPM(m_Header.tempo);
		if(m_Header.speed == 0){
			m_pSampler->TicksPerRow(6);
		} else {
			m_pSampler->TicksPerRow(m_Header.speed);
		}
		m_pSampler->IsLinearFreq((m_Header.flags & XMSampler::LINEAR_FREQUECY) == 1);

		LoadInstruments(*m_pSampler,iInstrStart);

	}

	const bool XMSongLoader::IsValid()
	{			

		bool bIsValid = false;
		
		// get header
		char * pID = AllocReadStr(17,0);

		// tracker name	
		char * pTrackerName = AllocReadStr(20,38);
		
		// get tracker version
		char * pTrackerVer = AllocReadStr(2,58);	

		// process
		TRACE(_T("Header: %s\n"),pID);
		TRACE(_T("Tracker: %s v%i.%i\n"),pTrackerName,int(pTrackerVer[1]),int(pTrackerVer[0]));	

		// check header
		bIsValid = (!stricmp(pID,XM_HEADER));


		// cleanup
		delete[] pID;
		delete[] pTrackerName;
		delete[] pTrackerVer;

		return bIsValid;
	}

	const long XMSongLoader::LoadPatterns(Song & song)
	{

		// get song name
		
		char * pSongName = AllocReadStr(20,17);
		
		if(pSongName==NULL)
			return 0;
		strcpy(song.Name,pSongName);	
		strcpy(song.Author,"");
		strcpy(song.Comment,"Imported from Fasttracker II module.");
		zapArray(pSongName);

		// get data
		Seek(60);
		Read(&m_Header,sizeof(XMFILEHEADER));
/*
		int iHeaderLen = ReadInt4(60);
		short iSongLen = ReadInt2();
		short iRestartPos = ReadInt2();
		short iNoChannels = ReadInt2();
		short iNoPatterns = ReadInt2();
		short iNoInstruments = ReadInt2();
		short iFlags = ReadInt2();		// ignored*/

/*		m_iTempoTicks = ReadInt2();
		m_iTempoBPM = ReadInt2();
		
		// get pattern order	
		unsigned char playOrder[256];
		m_File.Read(playOrder,256);*/

		for(int i = 0;i < MAX_SONG_POSITIONS && i < 256;i++)
		{
			if ( m_Header.order[i] < MAX_PATTERNS ){
				song.playOrder[i]=m_Header.order[i];
			} else { 
				song.playOrder[i]=0;
			}
		}

		if ( m_Header.norder > MAX_SONG_POSITIONS ){
			song.playLength=MAX_SONG_POSITIONS;
		} else {
			song.playLength=m_Header.norder;
		}

		song.SONGTRACKS=m_Header.channels;
		
		// tempo
		// BPM = 6 * iTempoBPM / iTempoTicks;
		// 
		song.BeatsPerMin(6 * m_Header.tempo / ((m_Header.speed == 0)?6:m_Header.speed) );
		
		// instr count
		m_iInstrCnt = m_Header.instruments;

		// get pattern data
		int nextPatStart = m_Header.size + 60;
		for(int j = 0;j < m_Header.patterns && nextPatStart > 0;j++){
			nextPatStart = LoadSinglePattern(song,nextPatStart,j,m_Header.channels);
		}
		
		return nextPatStart;
	}

	// Load instruments
	const bool XMSongLoader::LoadInstruments(XMSampler & sampler, LONG iInstrStart)
	{	
		for(int i = 1;i <= m_iInstrCnt;i++){
			iInstrStart = LoadInstrument(sampler,iInstrStart,i);
			TRACE2("%d %s\n",i,sampler.Instrument(i).Name().c_str());
		}

		return true;
	}

	char * XMSongLoader::AllocReadStr(const LONG size, const LONG start)
	{
		// allocate space
		char *pData = new char[size + 1];
		if(pData==NULL)
			return NULL;

		// null terminate
		pData[size]=0;
		
		// go to offset
		if(start>=0)
			Seek(start);

		// read data
		if(Read(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}





	// return address of next pattern, 0 for invalid
	const LONG XMSongLoader::LoadSinglePattern(Song & song, const LONG start,const int patIdx,const int iTracks)
	{

		int iHeaderLen = ReadInt4(start);
		char iPackingType = ReadInt1();
		short iNumRows = ReadInt2();
		short iPackedSize = ReadInt2();

		if(patIdx < MAX_PATTERNS)
			song.patternLines[patIdx]=iNumRows;

		PatternEntry e;

		if(iPackedSize == 0)
		{
			// build empty PatternEntry
			e._note = 255;
			e._inst = 255;
			e._mach = 255;
			e._cmd=0;
			e._parameter=0;
//			e._volume = 0;
//			e._volcmd = 0;

			// build empty pattern
			for(int row=0;row<iNumRows;row++)
				for(int col=0;col<iTracks;col++)
					WritePatternEntry(song,patIdx,row,col,e);	
		}	
		else
		{
			// get next values
			for(int row = 0;row < iNumRows;row++)
			{
				for(int col=0;col<iTracks;col++)
				{	
					// reset
					char note=-1;
					char instr=-1;
					char vol=0;
					char type=0;
					char param=0;

					// read note
					note = ReadInt1();

					// is compression bit set?
					if(note & 0x80)
					{
						char bReadNote = note&0x01;
						char bReadInstr = note&0x02;
						char bReadVol = note&0x04;
						char bReadType = note&0x08;
						char bReadParam  = note&0x10;

						note = 0;
						if(bReadNote) note = ReadInt1(); 
						if(bReadInstr) instr = ReadInt1();
						if(bReadVol) vol = ReadInt1();
						if(bReadType) type = ReadInt1();
						if(bReadParam) param = ReadInt1();
					}
					else
					{
						// read all values
						instr = ReadInt1();
						vol = ReadInt1();
						type = ReadInt1();
						param = ReadInt1();				
					}								

					// translate
					e._inst = instr;	
					e._mach = 0;
//					e._volume = 0;
//					e._volcmd = 0;

					
					
					// volume/command
					if(vol >= 0x10 && vol <= 0x50)
					{
						// translate volume
						type=0x0C;
						param=vol-0x10;
						
/*						if(vol == 0x50) 
						{ e._volume = 255;
						} else {
							e._volume = 4 * (vol - 0x10);
						}
						e._volcmd = XMSampler::CMD::VOLUME;
*/
					} else {

//						if(vol >= 0x60){						
/*						e._volcmd = VOLCMD_EXG_TABLE[(vol & 0xf0) >> 4];
						e._volume = vol & 0xf;

						switch(e._volcmd){

							case XMSampler::CMD::PORTA2NOTE:
							case XMSampler::CMD::PANNING:
								e._volume *= 16;
								break;
							case XMSampler::CMD::FINEVOLDOWN:
							case XMSampler::CMD::FINEVOLUP:
							case XMSampler::CMD::VOLSLIDEUP:
							case XMSampler::CMD::VOLSLIDEDOWN:
							case XMSampler::CMD::PANSLIDELEFT:
							case XMSampler::CMD::PANSLIDERIGHT:
								e._volume *=  4;
								break;
						}
*/

					}
	//				else
					e._cmd = type;
					switch(type){
						case 0:
							if(param != 0){
								e._cmd = XMSampler::CMD::ARPEGGIO;
								e._parameter = param;
							} else {
								e._parameter = param;
							}
							break;
						case XMSampler::CMD::VOLUME:
							if (param== 64)
								{e._parameter=255;
								}
							else e._parameter = param * 4;
							break;
						case XMSampler::CMD::VIBRATO_SPEED:
						case XMSampler::CMD::PORTADOWN:
						case XMSampler::CMD::PORTAUP:
							e._parameter = param * 4;
							break;
						case XMSampler::CMD::VOLUMESLIDE:
							// ***** [bohan] iso-(10)646 encoding only please! *****
							if(param & 0xf){
								e._cmd = XMSampler::CMD::VOLSLIDEDOWN;
								e._parameter = (param & 0x0f) << 2;
							} else if(param & 0xf0){
								e._cmd = XMSampler::CMD::VOLSLIDEUP;
								e._parameter = (param & 0xf0) >> 2;
							}
							break;
						case XMSampler::CMD::PANNINGSLIDE:
							// ***** [bohan] iso-(10)646 encoding only please! *****
							if(param & 0xf){
								e._cmd = XMSampler::CMD::PANSLIDERIGHT;
								e._parameter = (param & 0xf) << 2;
							} else if(param & 0xf0){
								e._cmd = XMSampler::CMD::PANSLIDELEFT;
								e._parameter = (param & 0xf0) >> 2;
							}
							break;
						case XMSampler::CMD::EXTENDED:
							// ***** [bohan] iso-(10)646 encoding only please! *****
							switch(param & 0xf0){
							case 0x10:
								e._cmd = XMSampler::CMD::FINE_PORTAMENTO_UP;
								e._parameter = param & 0xf;
								break;
							case 0x20:
								e._cmd = XMSampler::CMD::FINE_PORTAMENTO_DOWN;
								e._parameter = param & 0xf;
								break;
							case 0x30:
								e._cmd = XMSampler::CMD::GRISSANDO;
								e._parameter = param & 0xf;
								break;
							case 0x40:
								e._cmd = XMSampler::CMD::VIBRATO_TYPE;
								e._parameter = param & 0xf;
								break;
							case 0x50:
								e._cmd = XMSampler::CMD::SET_FINE_TUNE;
								e._parameter = param & 0xf;
								break;
							case 0x60:
								e._cmd = XMSampler::CMD::PATTERN_LOOP;
								e._parameter = param & 0xf;
								break;
							case 0x70:
								e._cmd = XMSampler::CMD::TREMOLO_TYPE;
								e._parameter = param & 0xf;
								break;
							case 0x80:
								e._cmd = XMSampler::CMD::SET_PANNING;
								e._parameter = param & 0xf;
								break;
							case 0x90:
								e._cmd = XMSampler::CMD::RETRIGGER_NOTE;
								e._parameter = param & 0xf;
								break;
							case 0xA0:
								e._cmd = XMSampler::CMD::FINE_VOLSLIDE_UP;
								e._parameter = param & 0xf;
								break;
							case 0xB0:
								e._cmd = XMSampler::CMD::FINE_VOLSLIDE_DOWN;
								e._parameter = param & 0xf;
								break;
							case 0xc0:
								e._cmd = XMSampler::CMD::NOTE_CUT;
								e._parameter = param & 0xf;
								break;
							case 0xd0:
								e._cmd = XMSampler::CMD::NOTE_DELAY;
								e._parameter = param & 0xf;
								break;
							case 0xe0:
								e._cmd = XMSampler::CMD::PATTERN_DELAY;
								e._parameter = param & 0xf;
								break;
							case 0xf0:
								e._cmd = XMSampler::CMD::SET_ACTIVE_MACRO;
								e._parameter = param & 0xf;
								break;

							}

							break;
						case XMSampler::CMD::EXTEND_XM_EFFECTS:
							switch(param & 0xf0){
							case 0x10:
								// Extra Fine Portamento Down
								e._cmd = XMSampler::CMD::EXTRA_FINE_PORTAMENTO_DOWN;
								e._parameter = param & 0xf;
								break;
							case 0x20:
								// Extra Fine Portamento Up
								e._cmd = XMSampler::CMD::FINE_PORTAMENTO_UP;
								e._parameter = param & 0xf;
								break;
							case 0x80:
								switch ( param & 0xf){
								case 0:
									// Surround OFF
									e._cmd = XMSampler::CMD::SURROUND_OFF;
									e._parameter = 0;
									break;
								case 1:
									// Surround ON
									e._cmd = XMSampler::CMD::SURROUND_ON;
									e._parameter = 0;
									break;
								case 8:
									// Reverb Off
									e._cmd = XMSampler::CMD::REVERB_OFF;
									e._parameter = 0;
									break;
								case 9:
									// Reverb On
									e._cmd = XMSampler::CMD::REVERB_ON;
									e._parameter = 0;
									break;
								case 0xa:
									// Center Surround
									e._cmd = XMSampler::CMD::CENTER_SURROUND;
									e._parameter = 0;
									break;
								case 0xb:
									// Quad Surround
									e._cmd = XMSampler::CMD::QUOAD_SURROUND;
									e._parameter = 0;
									break;
								case 0xc:
									// Global Filters
									e._cmd = XMSampler::CMD::GROBAL_FILTERS;
									e._parameter = 0;
									break;
								case 0xd:
									// Local Filters
									e._cmd = XMSampler::CMD::LOCAL_FILTERS;
									e._parameter = 0;
									break;
								case 0xe:
									// Play Forward
									e._cmd = XMSampler::CMD::PLAY_FORWARD;
									e._parameter = 0;
									break;
								case 0xf:
									// Play Backward
									e._cmd = XMSampler::CMD::PLAY_BACKWARD;
									e._parameter = 0;
									break;
								}
								break;
							case 0xA0:
								e._cmd = XMSampler::CMD::SET_HIGH_OFFSET;
								e._parameter = param & 0xf;
								break;

							}
							break;
						default:
							e._parameter = param;
							break;
					}

					// instrument/note
					note = note & 0x7f;
					switch(note)
					{
						case 0x00: 
							e._note = 255;
							break;// no note

						case 0x61:
							e._note = 120;
							e._inst = 255;
							e._mach = 0;
							break;// noteoff		
						
						default: 
							if(note >= 96 || note < 0)
								TRACE(_T("invalid note\n"));
							e._note  = note-1; // Note 0 means noteoff, whereas in psycle it is C-0.
				/*		

							// force note into range
							if (note >= 30)
								e._note = note - 30;
							else if (note>=30-12)
								e._note = note - (30-12);
							else if (note>=30-24)
								e._note = note - (30-24);
							else if (note>=30-36)
								e._note = note - (30-36);

					*/
							break;	// transpose
					}

//					if ((e._note == 255) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255) && (e._volume == 0) && (e._volcmd == 0))
					if ((e._note == 255) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255))
					{
						e._mach = 255;
					}
					WritePatternEntry(song,patIdx,row,col,e);	
				}
			}
		}

		//int z = ftell(_file);
		return start + iPackedSize + iHeaderLen;

	}


	const BOOL XMSongLoader::WritePatternEntry(Song & song,
		const int patIdx, const int row, const int col,PatternEntry &e)
	{
		// don't overflow song buffer 
		if(patIdx>=MAX_PATTERNS) return false;

		PatternEntry* pData = (PatternEntry*) song._ptrackline(patIdx,col,row);

		*pData = e;

		return true;
	}	

	const LONG XMSongLoader::LoadInstrument(XMSampler & sampler, LONG iStart, const int idx)
	{
		// read header
		Seek(iStart);
		
		int iInstrSize = ReadInt4();
		ASSERT(iInstrSize==0x107||iInstrSize==0x21);
		TCHAR sInstrName[23];
		ZeroMemory(sInstrName,sizeof(sInstrName) * sizeof(TCHAR));
		Read(sInstrName,22);

		int iInstrType = ReadInt1();
		int iSampleCount = ReadInt2();

		if(iSampleCount>1)
 			TRACE(_T("ssmple count = %d\n"),iSampleCount);

		// store instrument name
		//std::string& _tmp1 = 
		sampler.Instrument(idx).Name(sInstrName);

		//strcpy(song.pInstrument(idx)->_sName,sInstrName);

		//int iSampleHeader = ReadInt4();
		//ATLASSERT(iSampleHeader==0x28);
		
		if( iSampleCount > 0) {
			XMSAMPLEHEADER _samph;
			ZeroMemory(&_samph,sizeof(XMSAMPLEHEADER));
			Read(&_samph,sizeof(XMSAMPLEHEADER));
			
			sampler.Instrument(idx).AutoVibratoDepth(_samph.vibdepth);
			sampler.Instrument(idx).AutoVibratoRate(_samph.vibrate);
			sampler.Instrument(idx).AutoVibratoSweep(_samph.vibsweep);
			sampler.Instrument(idx).AutoVibratoType(_samph.vibtype);
			
			XMInstrument::NotePair npair;
			npair.second=_samph.snum[0];
			for(int i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=i;
				if (i< 12){
					//npair.second=_samph.snum[0]; implicit.
					sampler.Instrument(idx).NoteToSample(i,npair);
				} else if(i < 108){
					npair.second=_samph.snum[i-12];
					sampler.Instrument(idx).NoteToSample(i,npair);
				} else {
					//npair.second=_samph.snum[95]; implicit.
					sampler.Instrument(idx).NoteToSample(i,npair);
				}
			}

			ReadEnvelopes(sampler.Instrument(idx),_samph);
		}

		iStart += iInstrSize;
		
		if(iSampleCount==0)
			return iStart;

		// read instrument data	
		
		// load individual samples
		int i;
		for(i=0;i<iSampleCount;i++)
			iStart = LoadSampleHeader(sampler,iStart,idx,i);
		for(i=0;i<iSampleCount;i++)
			iStart = LoadSampleData(sampler,iStart,idx,i);

		sampler.Instrument(idx).IsEnabled(true);
		return iStart;
	}

	const LONG XMSongLoader::LoadSampleHeader(XMSampler & sampler, LONG iStart, const int iInstrIdx, const int iSampleIdx)
	{
		// get sample header
		Seek(iStart);
		int iLen = ReadInt4();

		// loop data
		int iLoopStart = ReadInt4();
		int iLoopLength = ReadInt4();

		// params
		char iVol = ReadInt1();
		char iFineTune = ReadInt1();
		char iFlags = ReadInt1();
		unsigned char iPanning = ReadInt1();
		char iRelativeNote = ReadInt1();
		char iReserved = ReadInt1();	

		// sample name
		char * sName = AllocReadStr(22);
		
		// parse
		BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
		BOOL bPingPong = iFlags & 0x02;
		BOOL b16Bit = iFlags & 0x10;
	
		// alloc wave memory

		ASSERT(iLen < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.

	
		XMInstrument::WaveData& _wave = sampler.Instrument(iInstrIdx).rWaveLayer(iSampleIdx);
		
		_wave.Init();
		_wave.AllocWaveData(b16Bit?iLen / 2:iLen,false);
		_wave.WaveLength(b16Bit?iLen / 2:iLen);
		_wave.PanEnabled(true);
		_wave.PanFactor(iPanning/256.0f);
//		XMInstrument::WaveData& _data = sampler.Instrument(iInstrIdx).rWaveData(0).
//		sampler.Instrument(iInstrIdx).rWaveData()..Name() = sName;
		
		delete[] sName;

		if(bLoop)
		{
			if((iFlags & 0x1) == XMInstrument::WaveData::LoopType::NORMAL){
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
			} else if((iFlags & 0x2) == XMInstrument::WaveData::LoopType::BIDI){
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::BIDI);
			}
		

			if(b16Bit)
			{
				_wave.WaveLoopStart(iLoopStart / 2);
				_wave.WaveLoopEnd((iLoopLength  + iLoopStart )/ 2);
			}
			else
			{
				_wave.WaveLoopStart(iLoopStart);
				_wave.WaveLoopEnd(iLoopLength + iLoopStart);
			}
			
//			TRACE2("l:%x s:%x e:%x \n",_wave.WaveLength(),_wave.WaveLoopStart(),_wave.WaveLoopEnd()); 

		} else {
			_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
		}


		_wave.WaveVolume(((int)iVol * 100) / 64);
		_wave.WaveTune(iRelativeNote);
		_wave.WaveFineTune(iFineTune*2); // WaveFineTune has double range.

		smpLen[iSampleIdx] = iLen;
		smpFlags[iSampleIdx] = iFlags;

		return iStart + 40;

	}

	const LONG XMSongLoader::LoadSampleData(XMSampler & sampler, LONG iStart,const int iInstrIdx,const int iSampleIdx)
	{
		// parse
		
		BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
		XMInstrument::WaveData& _wave =  sampler.Instrument(iInstrIdx).rWaveLayer(iSampleIdx);
		short wNew=0;

		// cache sample data
		Seek(iStart);
		char * smpbuf = new char[smpLen[iSampleIdx]];
		Read(smpbuf,smpLen[iSampleIdx]);

		int sampleCnt = smpLen[iSampleIdx];

		// unpack sample data
		if(b16Bit)
		{				
			// 16 bit mono sample, delta
			int out=0;
			for(int j=0;j<sampleCnt;j+=2)
			{
				wNew += 0xFF & smpbuf[j] | smpbuf[j+1]<<8;				
				*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
				out++;
			}   
		}
		else
		{
			// 8 bit mono sample
			for(int j=0;j<sampleCnt;j++)
			{			
				wNew += (smpbuf[j]<<8);// | char(rand())); // scale + dither
				*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew;
			}
		}

		// cleanup
		delete[] smpbuf;

		// complete			
		iStart += smpLen[iSampleIdx];
		return iStart;
	}

	
	void XMSongLoader::ReadEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader)
	{
		/*
			
		*/
		const int _tickPerWave = 44100  /* samples/sec */ * 60 /* sec */  / (4 /*row */ * 6 /* tick */ * m_pSampler->BPM());

		inst.VolumeFadeSpeed
			((((float)sampleHeader.volfade) * 2) / ((float)_tickPerWave * 65536.0f));
		// volume envelope
		inst.AmpEnvelope()->Init();
		if(sampleHeader.vtype & 1){// enable volume envelope
			inst.AmpEnvelope()->IsEnabled(true);
			
			if(sampleHeader.vtype & 2){
				inst.AmpEnvelope()->SustainBegin(sampleHeader.vsustain);
				inst.AmpEnvelope()->SustainEnd(sampleHeader.vsustain);
			}

			
			if(sampleHeader.vtype & 4){
				if(sampleHeader.vloops < sampleHeader.vloope){
					inst.AmpEnvelope()->LoopStart(sampleHeader.vloops);
					inst.AmpEnvelope()->LoopEnd(sampleHeader.vloope);
				} else {
					inst.AmpEnvelope()->LoopStart(sampleHeader.vloope);
					inst.AmpEnvelope()->LoopEnd(sampleHeader.vloops);
				}
			}

            int envelope_point_num = sampleHeader.vnum;
			
			//inst.AmpEnvelope()->NumOfPoints(envelope_point_num);

			if(envelope_point_num > 12){ // Max number of envelope points in Fasttracker format is 12.
				envelope_point_num = 12;
			}
			
			// Format of FastTracker points is :
			// Point : frame number. (frame= line*(24/TPB), samplepos= frame*(samplespertick*TPB/24))
			// Value : 0..64. , divide by 64 to use it as a multiplier.
			for(int i = 0; i < envelope_point_num;i++){
//				inst.AmpEnvelope()->Point(i,((int)sampleHeader.venv[i * 2]) * _tickPerWave);
//				inst.AmpEnvelope()->Value(i,(float)sampleHeader.venv[i * 2 + 1] / 64.0f);
				inst.AmpEnvelope()->Append((int)sampleHeader.venv[i * 2] * _tickPerWave,(float)sampleHeader.venv[i * 2 + 1] / 64.0f);
			}

		} else {
			inst.AmpEnvelope()->IsEnabled(false);
		}

		// pan envelope
		inst.PanEnvelope()->Init();
		if(sampleHeader.ptype & 1){// enable volume envelope
			
			inst.PanEnvelope()->IsEnabled(true);
			
			if(sampleHeader.ptype & 2){
				inst.PanEnvelope()->SustainBegin(sampleHeader.psustain);
				inst.PanEnvelope()->SustainEnd(sampleHeader.psustain);
			}

			
			if(sampleHeader.ptype & 4){
				if(sampleHeader.ploops < sampleHeader.ploope){
					inst.PanEnvelope()->LoopStart(sampleHeader.ploops);
					inst.PanEnvelope()->LoopEnd(sampleHeader.ploope);
				} else {
					inst.PanEnvelope()->LoopStart(sampleHeader.ploope);
					inst.PanEnvelope()->LoopEnd(sampleHeader.ploops);
				}
			}
			int envelope_point_num = sampleHeader.pnum;

			//inst.PanEnvelope()->NumOfPoints(envelope_point_num);
			
			if(envelope_point_num > 12){
				envelope_point_num = 12;
			}

			for(int i = 0; i < envelope_point_num;i++){
//				inst.PanEnvelope()->Point(i,(int)sampleHeader.penv[i * 2] * _tickPerWave);
//				inst.PanEnvelope()->Value(i,(float)sampleHeader.penv[i * 2 + 1] / 64.0f);
				inst.PanEnvelope()->Append((int)sampleHeader.penv[i * 2] * _tickPerWave,(float)sampleHeader.penv[i * 2 + 1] / 64.0f);
			}

		} else {
			inst.PanEnvelope()->IsEnabled(false);
		}
		//inst.

	};		
}
}

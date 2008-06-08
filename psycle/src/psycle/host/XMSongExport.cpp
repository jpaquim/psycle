// -*- mode:c++; indent-tabs-mode:t -*-
/** @file 
 *  @brief implementation file
 *  $Date: 2008-02-12 00:44:11 +0100 (mar, 12 feb 2008) $
 *  $Revision: 6303 $
 */
#include <psycle/project.private.hpp>
#include "XMSongExport.hpp"
//#include "ProgressDialog.hpp"
#include "Song.hpp"
#include "Machine.hpp" // It wouldn't be needed, since it is already included in "song.h"
//#include "SongStructs.hpp"
//#include "configuration_options.hpp"
//#include "resources/resources.hpp"
//#include <algorithm>
//#include <cstring>

namespace psycle{
namespace host{


	XMSongExport::XMSongExport(void)
	{
	}

	XMSongExport::~XMSongExport(void)
	{

	}

	void XMSongExport::exportsong(Song& song,const bool fullopen)
	{

		writeSongHeader(song);

		SavePatterns(song);
		//WriteInstruments(*m_pSampler,iInstrStart);

	}

	void XMSongExport::writeSongHeader(Song &song)
	{
		Write(XM_HEADER, 17);//ID text
		std::string name = "PE:" + song.name.substr(0,17);
		Write(name.c_str(), 20);//Module name
		std::uint16_t temp = 0x1A;
		Write(&temp, 1);							
		Write("FastTracker v2.00   ", 20);//Tracker name
		temp = 0x0104;
		Write(&temp, 2);//Version number

		int patternCount = 0;
		for (int i = 0; i < MAX_PATTERNS; i++)
		{			
			if (song.IsPatternUsed(i))
				patternCount++;
		}

		memset(&m_Header,0,sizeof(m_Header));
		m_Header.size = sizeof(m_Header);
		m_Header.norder = song.playLength;
		m_Header.restartpos = 0;
		m_Header.channels = song.SONGTRACKS;
		m_Header.patterns = patternCount;
		m_Header.instruments = 
		m_Header.flags = 0x0001; //Linear frequency.
		m_Header.speed = 24/song.LinesPerBeat();
		m_Header.tempo =  song.BeatsPerMin();

		//Pattern order table
		for (int i = 0; i < song.playLength; i++) {
			m_Header.order[i] =  song.playOrder[i];
		}
		Write(&m_Header,m_Header.size);
	}

	void XMSongExport::SavePatterns(Song & song)
	{
		//We find the last index of machine, to use as first index of instruments
		lastMachine=63;
		while (song._pMachine[lastMachine] == 0 && lastMachine > 0) lastMachine--;
		lastMachine++;

		for (int i=0; i<lastMachine; i++) {
			if (song._pMachine[i]->_type == MACH_SAMPLER ) {
				isSampler[i] = 1;
//				isSampler.insert(i,1);
			}
			else {
				isSampler[i] = 0;
//				isSampler.insert(i,0);
			}
		}

		for (int i = 0; i < MAX_PATTERNS; i++)
		{
			SaveSinglePattern(song,i);
		}
	}
/*
	// Load instruments
	const bool XMSongExport::SaveInstruments(XMSampler & sampler, std::int32_t iInstrStart)
	{	
		int currentSample=0;
		for(int i = 1;i <= m_iInstrCnt;i++){
			iInstrStart = LoadInstrument(sampler,iInstrStart,i,currentSample);
			TRACE2("%d %s\n",i,sampler.rInstrument(i).Name().c_str());
		}

		return true;
	}
	*/

	// return address of next pattern, 0 for invalid
	void XMSongExport::SaveSinglePattern(Song & song, const int patIdx)
	{
		XMPATTERNHEADER ptHeader;
		memset(&ptHeader,0,sizeof(ptHeader));
		ptHeader.size = sizeof(ptHeader);
		//ptHeader.packingtype = 0; implicit from memset.
		ptHeader.rows = std::min(256,song.patternLines[patIdx]);
		//ptHeader.packedsize = 0; implicit from memset.

		std::size_t currentpos = GetPos();
		Write(&ptHeader,sizeof(ptHeader));


		// check every pattern for validity
		if (song.IsPatternUsed(patIdx))
		{
			for (int j = 0; j < ptHeader.rows; j++) {
				for (int i = 0; i < song.SONGTRACKS; i++) {
					
					PatternEntry* pData = (PatternEntry*) song._ptrackline(patIdx,i,j);
					
					
					unsigned char note;
					if (pData->_note <= notecommands::b9) {
						if (pData->_note >= 12 && pData->_note < 108 ) {
							note = pData->_note - 11;
						} else {
							note = 0x00;
						}
					}
					else if (pData->_note == notecommands::release) {
						note = 0x61;
					} else {
						note = 0x00;
					}
					
					unsigned char instr=0;
					
					//Very simple method for now:
					if (isSampler[pData->_mach] != 0) instr = lastMachine +  pData->_inst +1;
					else instr = pData->_mach + 1;

					unsigned char vol=0;
					unsigned char type=0;
					unsigned char param=0;


				//Putting just a few commands for now.
					switch(pData->_cmd) {
						case 0x0C:
							vol = 10 + (pData->_parameter/4);
							break;
						case PatternCmd::SET_TEMPO:
							if (pData->_parameter > 32) {
								type = XMCMD::SETSPEED;
								param = pData->_parameter;
							}
							break;
						case PatternCmd::EXTENDED:
							switch(pData->_parameter&0xF0) {
							case PatternCmd::SET_LINESPERBEAT0:
							case PatternCmd::SET_LINESPERBEAT1:
								type = XMCMD::SETSPEED;
								param = pData->_parameter;
								break;
							case PatternCmd::PATTERN_LOOP:
								type = XMCMD::EXTENDED;
								param = XMCMD_E::E_PATTERN_LOOP + (pData->_parameter & 0x0F);
								break;
							case PatternCmd::PATTERN_DELAY:
								type = XMCMD::EXTENDED;
								param = XMCMD_E::E_PATTERN_DELAY + (pData->_parameter & 0x0F);
								break;
							default:
								break;
							}
							break;							
						case PatternCmd::BREAK_TO_LINE:
							type = XMCMD::PATTERN_BREAK;
							param = (pData->_parameter/10)<<4 + (pData->_parameter%10);
							break;
						case PatternCmd::SET_VOLUME:
							type = XMCMD::SET_GLOBAL_VOLUME;
							param = param/2;
							break;
						default:
							break;
					}


					unsigned char bWriteNote = note!=0;
					unsigned char bWriteInstr = instr!=0;
					unsigned char bWriteVol = vol!=0;
					unsigned char bWriteType = type!=0;
					unsigned char bWriteParam  = param!=0;

					char compressed = 0x80 + bWriteNote + (bWriteInstr <<0x2) + (bWriteVol << 0x4) +
										+ (bWriteType << 0x8) + ( bWriteType << 0x10);
					Write(&compressed,1);
					if (bWriteNote) Write(&note,1);
					if (bWriteInstr) Write(&instr,1);
					if (bWriteVol) Write(&vol,1);
					if (bWriteType) Write(&type,1);
					if (bWriteParam) Write(&param,1);
					
				}
			}
			ptHeader.packedsize = static_cast<std::uint16_t>((GetPos() - currentpos) & 0xFFFF);
			Seek(currentpos);
			Write(&ptHeader,sizeof(ptHeader));
			Skip(ptHeader.packedsize);
		}
		else {
			Write(&ptHeader,sizeof(ptHeader));
		}
	}
/*

	const std::int32_t XMSongExport::LoadInstrument(XMSampler & sampler, std::int32_t iStart, const int idx,int &curSample)
	{
		Seek(iStart);

		// read header
		int iInstrSize = ReadInt4();
		//assert(iInstrSize==0x107||iInstrSize==0x21); // Skale Tracker (or MadTracker or who knows which more) don't have the "reserved[20]" parameter in the XMSAMPLEHEADER
		char sInstrName[23] = {0}; ///\todo it's probably useless to zero-initialise the array content
		Read(sInstrName,22);
		sInstrName[22]= 0;

		Skip(1); //int iInstrType = ReadInt1();
		int iSampleCount = ReadInt2();

		if(iSampleCount>1)
 			TRACE(_T("ssmple count = %d\n"),iSampleCount);

		sampler.rInstrument(idx).Name(sInstrName);
		iStart += iInstrSize;

		if(iSampleCount==0)
			return iStart;

        
		XMSAMPLEHEADER _samph;
		std::memset(&_samph, 0, sizeof _samph);
		Read(&_samph,sizeof(XMSAMPLEHEADER));
		
		int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
			XMInstrument::WaveData::WaveForms::SQUARE,
			XMInstrument::WaveData::WaveForms::SAWDOWN,
			XMInstrument::WaveData::WaveForms::SAWUP
		};		

		SetEnvelopes(sampler.rInstrument(idx),_samph);

		unsigned char *sRemap = new unsigned char[iSampleCount];
		int i;
		// read instrument data	
		for(i=0;i<iSampleCount;i++)
		{
			iStart = LoadSampleHeader(sampler,iStart,idx,curSample);
			 // Only get REAL samples.
			if ( smpLen[curSample] > 0 && curSample < MAX_INSTRUMENTS-2 ) {	sRemap[i]=curSample++; }
			else { sRemap[i]=MAX_INSTRUMENTS-1; }
		}
		// load individual samples
		for(i=0;i<iSampleCount;i++)
		{
			if ( sRemap[i] < MAX_INSTRUMENTS-1)
			{
				sampler.rInstrument(idx).IsEnabled(true);
				iStart = LoadSampleData(sampler,iStart,idx,sRemap[i]);

				//\todo : Improve autovibrato. (correct depth? fix for sweep?)
				XMInstrument::WaveData& _wave = sampler.SampleData(sRemap[i]);
				_wave.VibratoAttack(_samph.vibsweep!=0?255/_samph.vibsweep:255);
				_wave.VibratoDepth(_samph.vibdepth<<1);
				_wave.VibratoSpeed(_samph.vibrate);
				_wave.VibratoType(exchwave[_samph.vibtype&3]);
			}
		}

		XMInstrument::NotePair npair;
		if ( _samph.snum[0] < iSampleCount) npair.second=sRemap[_samph.snum[0]];
		else npair.second=0;
		for(int i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
			npair.first=i;
			if (i< 12){
				//npair.second=_samph.snum[0]; implicit.
				sampler.rInstrument(idx).NoteToSample(i,npair);
			} else if(i < 108){
				if ( _samph.snum[i] < iSampleCount) npair.second=sRemap[_samph.snum[i-12]];
				else npair.second=curSample-1;
				sampler.rInstrument(idx).NoteToSample(i,npair);
			} else {
				//npair.second=_samph.snum[95]; implicit.
				sampler.rInstrument(idx).NoteToSample(i,npair);
			}
		}
		delete[] sRemap;
		return iStart;
	}

	const std::int32_t XMSongExport::LoadSampleHeader(XMSampler & sampler, std::int32_t iStart, const int iInstrIdx, const int iSampleIdx)
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
		Skip(1);//char iReserved = ReadInt1();

		// sample name
		char * cName = AllocReadStr(22);

		// parse
		BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
		BOOL bPingPong = iFlags & 0x02;
		BOOL b16Bit = iFlags & 0x10;
	
		// alloc wave memory

		ASSERT(iLen < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.

		XMInstrument::WaveData& _wave = sampler.SampleData(iSampleIdx);
		
		_wave.Init();
		if ( iLen > 0 ) // Sounds Stupid, but it isn't. Some modules save sample header when there is no sample.
		{
			_wave.AllocWaveData(b16Bit?iLen / 2:iLen,false);
			_wave.WaveLength(b16Bit?iLen / 2:iLen);
		}
		else _wave.WaveLength(0);
		_wave.PanEnabled(true);
		_wave.PanFactor(iPanning/255.0f);
//		XMInstrument::WaveData& _data = sampler.Instrument(iInstrIdx).rWaveData(0).
//		sampler.Instrument(iInstrIdx).rWaveData()..Name() = sName;
		
		if(bLoop)
		{
			if(bPingPong){
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::BIDI);
			}else {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
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


		_wave.WaveVolume(iVol * 2);
		_wave.WaveTune(iRelativeNote);
		_wave.WaveFineTune(iFineTune*2); // WaveFineTune has double range.
		std::string sName = cName;
		_wave.WaveName(sName);
		delete[] cName;

		smpLen[iSampleIdx] = iLen;
		smpFlags[iSampleIdx] = iFlags;

		return iStart + 40;

	}

	const std::int32_t XMSongExport::LoadSampleData(XMSampler & sampler, std::int32_t iStart,const int iInstrIdx,const int iSampleIdx)
	{
		// parse
		
		BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
		XMInstrument::WaveData& _wave =  sampler.SampleData(iSampleIdx);
		short wNew=0;

		// cache sample data
		Seek(iStart);
		char * smpbuf = new char[smpLen[iSampleIdx]];
		memset(smpbuf,0,smpLen[iSampleIdx]);
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

	
	void XMSongExport::SetEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader)
	{
		// volume envelope
		inst.AmpEnvelope()->Init();
		if(sampleHeader.vtype & 1){// enable volume envelope
			inst.AmpEnvelope()->IsEnabled(true);
			// In FastTracker, the volume fade only works if the envelope is activated, so we only calculate
			// volumefadespeed in this case, so that a check during playback time is not needed.
			inst.VolumeFadeSpeed
				((float)sampleHeader.volfade / 32768.0f);
			
			int envelope_point_num = sampleHeader.vnum;
			if(envelope_point_num > 12){ // Max number of envelope points in Fasttracker format is 12.
				envelope_point_num = 12;
			}

			// Format of FastTracker points is :
			// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
			// Value : 0..64. , divide by 64 to use it as a multiplier.
			inst.AmpEnvelope()->Append((int)sampleHeader.venv[0] ,(float)sampleHeader.venv[1] / 64.0f);
			for(int i = 1; i < envelope_point_num;i++){
				if ( sampleHeader.venv[i*2] > sampleHeader.venv[(i-1)*2] )// Some rare modules have erroneous points. This tries to solve that.
					inst.AmpEnvelope()->Append((int)sampleHeader.venv[i * 2] ,(float)sampleHeader.venv[i * 2 + 1] / 64.0f);
			}

			if(sampleHeader.vtype & 2){
				inst.AmpEnvelope()->SustainBegin(sampleHeader.vsustain);
				inst.AmpEnvelope()->SustainEnd(sampleHeader.vsustain);
			}
			else
			{
				// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
				// IT also sets the Sustain points to the end of the envelope, but i can't see a reason for this to be needed.
//				inst.AmpEnvelope()->SustainBegin(inst.AmpEnvelope()->NumOfPoints()-1);
//				inst.AmpEnvelope()->SustainEnd(inst.AmpEnvelope()->NumOfPoints()-1);
			}

			
			if(sampleHeader.vtype & 4){
				if(sampleHeader.vloops < sampleHeader.vloope){
					inst.AmpEnvelope()->LoopStart(sampleHeader.vloops);
					inst.AmpEnvelope()->LoopEnd(sampleHeader.vloope);
				}
				// if loopstart >= loopend, Fasttracker ignores the loop!.
				// We can't ignore them because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
				else {
//					inst.AmpEnvelope()->LoopStart(XMInstrument::Envelope::INVALID);
//					inst.AmpEnvelope()->LoopEnd(XMInstrument::Envelope::INVALID);
					inst.AmpEnvelope()->LoopStart(inst.AmpEnvelope()->NumOfPoints()-1);
					inst.AmpEnvelope()->LoopEnd(inst.AmpEnvelope()->NumOfPoints()-1);
				}
			}
			else
			{
				// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
				inst.AmpEnvelope()->LoopStart(inst.AmpEnvelope()->NumOfPoints()-1);
				inst.AmpEnvelope()->LoopEnd(inst.AmpEnvelope()->NumOfPoints()-1);
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
			if(envelope_point_num > 12){
				envelope_point_num = 12;
			}

			for(int i = 0; i < envelope_point_num;i++){
				inst.PanEnvelope()->Append((int)sampleHeader.penv[i * 2] ,(float)(sampleHeader.penv[i * 2 + 1]-32.0f) / 32.0f);
			}

		} else {
			inst.PanEnvelope()->IsEnabled(false);
		}
		//inst.

	}	

*/

}
}

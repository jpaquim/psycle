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

	void XMSongExport::exportsong(Song& song)
	{

		writeSongHeader(song);

		SavePatterns(song);
		SaveInstruments(song);

	}

	void XMSongExport::writeSongHeader(Song &song)
	{
		//We find the last index of machine, to use as first index of instruments
		lastMachine=63;
		while (song._pMachine[lastMachine] == 0 && lastMachine > 0) lastMachine--;
		lastMachine++;

		for (int i=0; i<lastMachine; i++) {
			if (song._pMachine[i] != 0 && 
				song._pMachine[i]->_type == MACH_SAMPLER ) {
					isSampler[i] = 1;
			}
			else {
				isSampler[i] = 0;
			}
		}

		Write(XM_HEADER, 17);//ID text
		std::string name = "PE:" + song.name.substr(0,17);
		Write(name.c_str(), 20);//Module name
		std::uint16_t temp = 0x1A;
		Write(&temp, 1);							
		Write("FastTracker v2.00   ", 20);//Tracker name
		temp = 0x0104;
		Write(&temp, 2);//Version number

		memset(&m_Header,0,sizeof(m_Header));
		m_Header.size = sizeof(m_Header);
		m_Header.norder = song.playLength;
		m_Header.restartpos = 0;
		m_Header.channels = song.SONGTRACKS;
		m_Header.patterns = song.GetHighestPatternIndexInSequence()+1;
		m_Header.instruments = std::min(128,lastMachine + song.GetHighestInstrumentIndex()+1);
		m_Header.flags = 0x0001; //Linear frequency.
		m_Header.speed = 24/song.LinesPerBeat();
		m_Header.tempo =  song.BeatsPerMin();

		//Pattern order table
		for (int i = 0; i < song.playLength; i++) {
			m_Header.order[i] =  song.playOrder[i];
		}
		Write(&m_Header,sizeof(m_Header));
	}

	void XMSongExport::SavePatterns(Song & song)
	{
		for (int i = 0; i < m_Header.patterns ; i++)
		{
			SaveSinglePattern(song,i);
		}
	}

	// Load instruments
	void XMSongExport::SaveInstruments(Song& song)
	{
		for (int i = 0; i < lastMachine ; i++ ) {
			if ( song._pMachine[i] != 0 ) {
				SaveEmptyInstrument(song._pMachine[i]->_editName);
			}
			else {
				SaveEmptyInstrument("");
			}
		}
		for (int i = 0; i < m_Header.instruments; i++ ){
			SaveInstrument(song,i);
		}
	}


	// return address of next pattern, 0 for invalid
	void XMSongExport::SaveSinglePattern(Song & song, const int patIdx)
	{
		XMPATTERNHEADER ptHeader;
		memset(&ptHeader,0,sizeof(ptHeader));
		ptHeader.size = sizeof(ptHeader);
		//ptHeader.packingtype = 0; implicit from memset.
		ptHeader.rows = std::min(256,song.patternLines[patIdx]);
		//ptHeader.packedsize = 0; implicit from memset.

		Write(&ptHeader,sizeof(ptHeader));
		std::size_t currentpos = GetPos();


		// check every pattern for validity
		if (song.IsPatternUsed(patIdx))
		{
			for (int j = 0; j < ptHeader.rows && j < 256; j++) {
				for (int i = 0; i < song.SONGTRACKS; i++) {
					
					PatternEntry* pData = (PatternEntry*) song._ptrackline(patIdx,i,j);
					
					
					unsigned char note;
					if (pData->_note <= notecommands::b9) {
						if (pData->_note >= 12 && pData->_note < 108 ) {
							if (pData->_mach < MAX_MACHINES && song._pMachine[pData->_mach] != 0 
								&& isSampler[pData->_mach] != 0)
							{ // The sampler machine uses C-4 as middle C.
								note = pData->_note +1;
							} else {
								note = pData->_note - 11;
							}
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
					if (pData->_mach < MAX_MACHINES && song._pMachine[pData->_mach] != 0 ) {
						if (isSampler[pData->_mach] != 0 && pData->_inst != 0xFF) instr = lastMachine +  pData->_inst +1;
						else instr = pData->_mach + 1;
					}

					unsigned char vol=0;
					unsigned char type=0;
					unsigned char param=0;


				//Putting just a few commands for now.
			
					bool foundEffect = true;
					int singleEffectCharacter = (pData->_cmd & 0xF0);					

					if (singleEffectCharacter == 0xE0) { //slide up
						int slideAmount = (pData->_cmd & 0x0F);
						type = XMCMD::PORTAUP;
						param = pData->_parameter;
					}
					else if (singleEffectCharacter == 0xD0) { //slide down
						int slideAmount = (pData->_cmd & 0x0F);
						type = XMCMD::PORTADOWN;
						param = pData->_parameter;
					}
					else {
						switch(pData->_cmd) {
							case 0xC3:
								type = XMCMD::PORTA2NOTE;
								param = pData->_parameter;
								break;
							case 0x0C:
								vol = 0x10 + (pData->_parameter/4);
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
								foundEffect = false;
								break;
						}
					}

					if ((foundEffect == false) & (pData->_cmd > 0)) {
						type = XMCMD::ARPEGGIO;
						param = pData->_cmd;
					}

					unsigned char bWriteNote = note!=0;
					unsigned char bWriteInstr = instr!=0;
					unsigned char bWriteVol = vol!=0;
					unsigned char bWriteType = type!=0;
					unsigned char bWriteParam  = param!=0;

					char compressed = 0x80 + bWriteNote + (bWriteInstr << 1) + (bWriteVol << 2)
										+ (bWriteType << 3) + ( bWriteParam << 4);

					if (compressed !=  0x9F ) Write(&compressed,1); // 0x9F means to write everything.
					if (bWriteNote) Write(&note,1);
					if (bWriteInstr) Write(&instr,1);
					if (bWriteVol) Write(&vol,1);
					if (bWriteType) Write(&type,1);
					if (bWriteParam) Write(&param,1);
					
				}
			}
			ptHeader.packedsize = static_cast<std::uint16_t>((GetPos() - currentpos) & 0xFFFF);
			Seek(currentpos-sizeof(ptHeader));
			Write(&ptHeader,sizeof(ptHeader));
			Skip(ptHeader.packedsize);
		}
		else {
			Write(&ptHeader,sizeof(ptHeader));
		}
	}
	void XMSongExport::SaveEmptyInstrument(std::string name)
	{
		XMINSTRUMENTHEADER insHeader;
		memset(&insHeader,0,sizeof(insHeader));
		//insHeader.type = 0; Implicit by memset
		insHeader.size = sizeof(insHeader);
		strncpy(insHeader.name,name.c_str(),21);
		//insHeader.samples = 0; Implicit by memset
		Write(&insHeader,sizeof(insHeader));
	}


	void XMSongExport::SaveInstrument(Song& song, int instIdx)
	{
		XMINSTRUMENTHEADER insHeader;
		memset(&insHeader,0,sizeof(insHeader));
		strncpy(insHeader.name,song._pInstrument[instIdx]->_sName,21);
		//insHeader.type = 0; Implicit by memset

		//If no samples for this instrument, write it and exit.
		if (song._pInstrument[instIdx]->Empty()) {
			insHeader.size = sizeof(insHeader);
			//insHeader.samples = 0; Implicit by memset
			Write(&insHeader,sizeof(insHeader));
			return;
		}
		
		insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
		// sampler just has one sample per instrument.
		insHeader.samples = 1;
		Write(&insHeader,sizeof(insHeader));

		XMSAMPLEHEADER _samph;
		std::memset(&_samph, 0, sizeof(_samph));
		//For now, everything zeroed. Later on we can convert the ADSR curves to envelopes.
		//SetEnvelopes(instIdx,_samph);
		int filepos = GetPos();
		Write(&_samph,sizeof(_samph));

		SaveSampleHeader(song, instIdx);
		_samph.shsize = static_cast<std::uint32_t>(GetPos() - filepos);
		Seek(filepos);
		Write(&_samph,sizeof(_samph));
		Skip(_samph.shsize-sizeof(_samph));

		SaveSampleData(song, instIdx);

	}

	void XMSongExport::SaveSampleHeader(Song& song, int instIdx)
	{
		Instrument& instr = *song._pInstrument[instIdx];

		XMSAMPLESTRUCT stheader;
		memset(&stheader,0,sizeof(stheader));
		// stheader.name Implicitely set at zero by memset
		// stheader.res Implicitely set at zero by memset

		//All samples are 16bits in Psycle.
		stheader.samplen = instr.waveLength *2;
		stheader.loopstart = instr.waveLoopStart * 2;
		stheader.looplen = (instr.waveLoopEnd - instr.waveLoopStart) * 2;
		stheader.vol = std::min(64,instr.waveVolume*64/100);
		stheader.finetune = ((instr.waveFinetune/2) + 82) & 0xFF ;
		stheader.type = instr._loop?1:0 + 0x10; // 0x10 -> 16bits
		stheader.pan = instr._pan &0xFF;
		stheader.relnote = instr.waveTune + 30;

		Write(&stheader,sizeof(stheader));
	}
	
	void XMSongExport::SaveSampleData(Song& song,const int instrIdx)
	{
		// pack sample data
		short* samples = song._pInstrument[instrIdx]->waveDataL;
		int length = song._pInstrument[instrIdx]->waveLength;
		short prev=0;
		for(int j=0;j<length;j++)
		{
			short delta =  samples[j] - prev;
			Write(&delta,sizeof(short));
			prev = samples[j];
		} 
	}

	
	void XMSongExport::SetEnvelopes(Song& song, int instrIdx, XMSAMPLEHEADER & sampleHeader)
	{
/*
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
*/
	}	



}
}

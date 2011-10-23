#include <psycle/host/detail/project.private.hpp>
#include "XMSongExport.hpp"
#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/internalkeys.hpp>
#include <sstream>
#include <iomanip>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

using namespace universalis::stdlib;

void XMSongExport::exportsong(Song& song) {
	lines_per_beat_ = ComputeLinesPerBeat(song);
	writeSongHeader(song);
	SavePatterns(song);
	SaveInstruments(song);
}

void XMSongExport::writeSongHeader(Song &song) {
	//We find the last index of machine, to use as first index of instruments
	lastMachine = 63;
	while (lastMachine >= 0 && song.machine(lastMachine) == 0) --lastMachine;
	++lastMachine;

	for (int i=0; i<lastMachine; ++i) {
		if (song.machine(i) != 0 && 
			song.machine(i)->getMachineKey() == InternalKeys::sampler ) {
				isSampler[i] = 1;
		}
		else {
			isSampler[i] = 0;
		}
	}

	WriteArray(XM_HEADER, 17);//ID text

	std::ostringstream buf;
	buf << "PE:" << std::setw(20) <<  std::left << song.name().substr(0, 17);		
	WriteArray(buf.str().c_str(), 20);//Module name
	uint8_t temp = 0x1A;
	Write(temp);							
	WriteArray("FastTracker v2.00   ", 20);//Tracker name
	uint16_t temp2 = 0x0104;
	Write(temp2);//Version number

	std::memset(&m_Header, 0, sizeof m_Header);
	m_Header.size = sizeof(m_Header);
	psycle::core::SequenceLine & line = **(song.sequence().begin() + 1);
	int playLength = line.size();
	m_Header.norder = playLength;
	m_Header.restartpos = 0;
	m_Header.channels = std::min(static_cast<int>(song.tracks()), 32);
	int highest = 0;
	for(Sequence::patterns_type::iterator pite = song.sequence().patterns_begin(); pite != song.sequence().patterns_end(); ++pite) {
		Pattern & pat(**pite);
		highest = std::max(highest, pat.id());
	}
	m_Header.patterns = highest+1;
	m_Header.instruments = std::min(128, lastMachine + song.GetHighestInstrumentIndex() + 1);
	m_Header.flags = 0x0001; // linear frequency.
	m_Header.speed = 24 / lines_per_beat_;
	m_Header.tempo = song.BeatsPerMin();

	//Pattern order table
	SequenceLine::iterator it = line.begin();
	for (int i=0; it != line.end(); ++it, ++i) {
		m_Header.order[i] = (*it).second->pattern().id();
	}
	WriteHeader(m_Header);
}

void XMSongExport::SavePatterns(Song & song) {
	// todo sort after id
	Sequence::patterns_type::iterator pite = song.sequence().patterns_begin();
	for ( ; pite != song.sequence().patterns_end(); ++pite) {
		if ( (*pite) != &song.sequence().master_pattern() ) {
			SavePattern(song, *pite);
		}
	}		
}

// Load instruments
void XMSongExport::SaveInstruments(Song& song) {
	for (int i = 0; i < lastMachine ; i++ ) {
		if ( song.machine(i) != 0 ) {
			SaveEmptyInstrument(song.machine(i)->GetEditName());
		} else {
			SaveEmptyInstrument("");
		}
	}
	int remaining = m_Header.instruments - lastMachine;
	for (int i = 0 ; i < remaining; i++ ){
		SaveInstrument(song,i);
	}
}

void XMSongExport::SavePattern(Song& song, psycle::core::Pattern* pattern) {
	XMPATTERNHEADER ptHeader;
	memset(&ptHeader,0,sizeof(ptHeader));
	ptHeader.size = sizeof(ptHeader);
	//ptHeader.packingtype = 0; implicit from memset.
	ptHeader.rows = std::min(256,static_cast<int>(pattern->beats()*lines_per_beat_));
	//ptHeader.packedsize = 0; implicit from memset.

	WriteHeader(ptHeader);
	std::size_t currentpos = GetPos();	
	// check every pattern for validity
	if (pattern->size() != 0) {	
		int len = (ptHeader.rows * song.tracks());
		std::vector<PatternEvent> events(len);

		// Build pattern matrix
		psycle::core::Pattern::iterator it = pattern->begin();
		for ( ; it != pattern->end(); ++it) {
			PatternEvent& ev = it->second;
			int j = static_cast<int>(it->first * lines_per_beat_);
			int i = ev.track();
			int pos = (i%m_Header.channels) + j*m_Header.channels;
			events[pos] = ev;
		}

		// write the matrix to the file
		for (int j = 0; j < ptHeader.rows && j < 256; ++j) {
			for (int i = 0; i < m_Header.channels; ++i) {
				int pos = (i%m_Header.channels) + j*m_Header.channels;
				PatternEvent& ev = events[pos];
				unsigned char note;
				if (ev.note() <= notecommands::b9) {
					if (ev.note() >= 12 && ev.note() < 108 ) {
						if (ev.machine() < MAX_MACHINES && song.machine(ev.machine()) != 0 
							&& isSampler[ev.machine()] != 0)
						{ // The sampler machine uses C-4 as middle C.
							note = ev.note() +1;
						} else {
							note = ev.note() - 11;
						}
					} else {
						note = 0x00;
					}
				}
				else if (ev.note() == notecommands::release) {
					note = 0x61;
				} else {
					note = 0x00;
				}
				
				unsigned char instr=0;
				
				//Very simple method for now:
				if (ev.machine() < MAX_MACHINES) {
					if ( song.machine(ev.machine()) != 0 && isSampler[ev.machine()] != 0)
					{
						if (ev.instrument() != 0xFF) instr = lastMachine +  ev.instrument() +1;
					}
					else instr = ev.machine() + 1;
				}

				unsigned char vol=0;
				unsigned char type=0;
				unsigned char param=0;


			//Putting just a few commands for now.
		
				bool foundEffect = true;
				int singleEffectCharacter = (ev.command() & 0xF0);					

				if (singleEffectCharacter == 0xE0) { //slide up
					int slideAmount = (ev.command() & 0x0F);
					type = XMCMD::PORTAUP;
					param = ev.parameter();
				}
				else if (singleEffectCharacter == 0xD0) { //slide down
					int slideAmount = (ev.command() & 0x0F);
					type = XMCMD::PORTADOWN;
					param = ev.parameter();
				}
				else {
					switch(ev.command()) {
						case 0xC3:
							type = XMCMD::PORTA2NOTE;
							param = ev.parameter();
							break;
						case 0x0C:
							vol = 0x10 + (ev.parameter()/4);
							break;
						case PatternCmd::SET_TEMPO:
							if (ev.parameter() > 32) {
								type = XMCMD::SETSPEED;
								param = ev.parameter();
							}
							break;
						case PatternCmd::EXTENDED:
							switch(ev.parameter()&0xF0) {
							case PatternCmd::extended::SET_LINESPERBEAT0:
							case PatternCmd::extended::SET_LINESPERBEAT1:
								type = XMCMD::SETSPEED;
								param = ev.parameter();
								break;
							case PatternCmd::extended::PATTERN_LOOP:
								type = XMCMD::EXTENDED;
								param = XMCMD_E::E_PATTERN_LOOP + (ev.parameter() & 0x0F);
								break;
							case PatternCmd::extended::PATTERN_DELAY:
								type = XMCMD::EXTENDED;
								param = XMCMD_E::E_PATTERN_DELAY + (ev.parameter() & 0x0F);
								break;
							default:
								break;
							}
							break;							
						case PatternCmd::BREAK_TO_LINE:
							type = XMCMD::PATTERN_BREAK;
							param = (ev.parameter()/10)<<4 + (ev.parameter()%10);
							break;
						case PatternCmd::SET_VOLUME:
							type = XMCMD::SET_GLOBAL_VOLUME;
							param = ev.parameter()/2;
							break;
						default:
							foundEffect = false;
							break;
					}
				}

				if ((foundEffect == false) & (ev.command() > 0)) {
					type = XMCMD::ARPEGGIO;
					param = ev.command();
				}
				
				bool bWriteNote = (note != 0);
				bool bWriteInstr = (instr != 0);
				bool bWriteVol = (vol != 0);
				bool bWriteType = (type != 0);
				bool bWriteParam  = (param !=0);
				char is_compressed = 0x80 + bWriteNote + (bWriteInstr << 1) + (bWriteVol << 2)
								+ (bWriteType << 3) + ( bWriteParam << 4);

				if (is_compressed !=  0x9F ) Write(is_compressed); // 0x9F means to write everything.
				if (bWriteNote) Write(note);
				if (bWriteInstr) Write(instr);
				if (bWriteVol) Write(vol);
				if (bWriteType) Write(type);
				if (bWriteParam) Write(param);
			}
		}
		ptHeader.packedsize = static_cast<uint16_t>((GetPos() - currentpos) & 0xFFFF);
		Seek(currentpos-sizeof(ptHeader));
		WriteHeader(ptHeader);
		Skip(ptHeader.packedsize);
	} else {
		WriteHeader(ptHeader);
	}

}

// return address of next pattern, 0 for invalid
#if 0
//Old method, to check if we've forgot anything
void XMSongExport::SavePattern(Song & song, const int patIdx) {
		XMPATTERNHEADER ptHeader;
		memset(&ptHeader,0,sizeof(ptHeader));
		ptHeader.size = sizeof(ptHeader);
		//ptHeader.packingtype = 0; implicit from memset.
		ptHeader.rows = std::min(256,song.patternLines[patIdx]);
		//ptHeader.packedsize = 0; implicit from memset.

		Write(&ptHeader,sizeof(ptHeader));
		std::size_t currentpos = GetPos();

		// check every pattern for validity
		if (song.IsPatternUsed(patIdx)) {
			for (int j = 0; j < ptHeader.rows && j < 256; j++) {
				for (int i = 0; i < song.tracks(); i++) {
					
					PatternEvent* pData = (PatternEvent*) song._ptrackline(patIdx,i,j);
					
					
					unsigned char note;
					if (pData->note() <= notecommands::b9) {
						if (pData->note() >= 12 && pData->note() < 108 ) {
							if (pData->machine() < MAX_MACHINES && song.machine(pData->machine()) != 0 
								&& isSampler[pData->machine()] != 0)
							{ // The sampler machine uses C-4 as middle C.
								note = pData->note() +1;
							} else {
								note = pData->note() - 11;
							}
						} else {
							note = 0x00;
						}
					}
					else if (pData->note() == notecommands::release) {
						note = 0x61;
					} else {
						note = 0x00;
					}
					
					unsigned char instr=0;
					
					//Very simple method for now:
					if (pData->machine() < MAX_MACHINES) {
						if ( song.machine(pData->machine()) != 0 && isSampler[pData->machine()] != 0)
						{
							if (pData->instrument() != 0xFF) instr = lastMachine +  pData->instrument() +1;
						}
						else instr = pData->machine() + 1;
					}

					unsigned char vol=0;
					unsigned char type=0;
					unsigned char param=0;


				//Putting just a few commands for now.
			
					bool foundEffect = true;
					int singleEffectCharacter = (pData->command() & 0xF0);					

					if (singleEffectCharacter == 0xE0) { //slide up
						int slideAmount = (pData->command() & 0x0F);
						type = XMCMD::PORTAUP;
						param = pData->parameter();
					}
					else if (singleEffectCharacter == 0xD0) { //slide down
						int slideAmount = (pData->command() & 0x0F);
						type = XMCMD::PORTADOWN;
						param = pData->parameter();
					}
					else {
						switch(pData->command()) {
							case 0xC3:
								type = XMCMD::PORTA2NOTE;
								param = pData->parameter();
								break;
							case 0x0C:
								vol = 0x10 + (pData->parameter()/4);
								break;
							case PatternCmd::SET_TEMPO:
								if (pData->parameter() > 32) {
									type = XMCMD::SETSPEED;
									param = pData->parameter();
								}
								break;
							case PatternCmd::EXTENDED:
								switch(pData->parameter()&0xF0) {
								case PatternCmd::SET_LINESPERBEAT0:
								case PatternCmd::SET_LINESPERBEAT1:
									type = XMCMD::SETSPEED;
									param = pData->parameter();
									break;
								case PatternCmd::PATTERN_LOOP:
									type = XMCMD::EXTENDED;
									param = XMCMD_E::E_PATTERN_LOOP + (pData->parameter() & 0x0F);
									break;
								case PatternCmd::PATTERN_DELAY:
									type = XMCMD::EXTENDED;
									param = XMCMD_E::E_PATTERN_DELAY + (pData->parameter() & 0x0F);
									break;
								default:
									break;
								}
								break;							
							case PatternCmd::BREAK_TO_LINE:
								type = XMCMD::PATTERN_BREAK;
								param = (pData->parameter()/10)<<4 + (pData->parameter()%10);
								break;
							case PatternCmd::SET_VOLUME:
								type = XMCMD::SET_GLOBAL_VOLUME;
								param = pData->parameter()/2;
								break;
							default:
								foundEffect = false;
								break;
						}
					}

					if ((foundEffect == false) & (pData->command() > 0)) {
						type = XMCMD::ARPEGGIO;
						param = pData->command();
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
			ptHeader.packedsize = static_cast<uint16_t>((GetPos() - currentpos) & 0xFFFF);
			Seek(currentpos-sizeof(ptHeader));
			Write(&ptHeader,sizeof(ptHeader));
			Skip(ptHeader.packedsize);
		} else {
			Write(&ptHeader,sizeof(ptHeader));
		}
}
#endif

int XMSongExport::ComputeLinesPerBeat(Song& song) {
	psycle::core::Sequence& seq = song.sequence();
	psycle::core::Sequence::patterns_type::iterator it = seq.patterns_begin();
	double min = 1.0;
	for ( ; it != seq.patterns_end(); ++it) {
		psycle::core::Pattern & pattern = **it;
		psycle::core::Pattern::iterator pat_it = pattern.begin();
		double old_pos = 0;
		for ( ; pat_it != pattern.end(); ++pat_it ) {
			double pos = pat_it->first;
			double delta = pos - old_pos;
			if ( delta != 0 && delta < min)
				min = delta;
		}
	}
	return static_cast<int>(1 / min);
}

void XMSongExport::SaveEmptyInstrument(std::string const & name) {
	XMINSTRUMENTHEADER insHeader;
	memset(&insHeader,0,sizeof insHeader);
	//insHeader.type = 0; Implicit by memset
	insHeader.size = sizeof insHeader;
	strncpy(insHeader.name, name.c_str(), 21);
	//insHeader.samples = 0; Implicit by memset
	WriteHeader(insHeader);
}


void XMSongExport::SaveInstrument(Song& song, int instIdx) {
	XMINSTRUMENTHEADER insHeader;
	memset(&insHeader,0,sizeof(insHeader));
	strncpy(insHeader.name,song._pInstrument[instIdx]->_sName,21);
	//insHeader.type = 0; Implicit by memset

	//If no samples for this instrument, write it and exit.
	if (song._pInstrument[instIdx]->Empty()) {
		insHeader.size = sizeof(insHeader);
		//insHeader.samples = 0; Implicit by memset
		WriteHeader(insHeader);
		return;
	}
	
	insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
	// sampler just has one sample per instrument.
	insHeader.samples = 1;
	WriteHeader(insHeader);

	XMSAMPLEHEADER _samph;
	std::memset(&_samph, 0, sizeof(_samph));
	//For now, everything zeroed. Later on we can convert the ADSR curves to envelopes.
	//SetEnvelopes(instIdx,_samph);
	_samph.volfade=0x400;
	int filepos = GetPos();
	_samph.shsize = sizeof(XMSAMPLESTRUCT);
	WriteHeader(_samph);

	SaveSampleHeader(song, instIdx);
	SaveSampleData(song, instIdx);
}

void XMSongExport::SaveSampleHeader(Song& song, int instIdx) {
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
	stheader.finetune = ((instr.waveFinetune/2)-28) & 0xFF ;
	stheader.type = instr._loop?1:0 + 0x10; // 0x10 -> 16bits
	stheader.pan = instr._pan &0xFF;
	stheader.relnote = instr.waveTune + 29;

	WriteHeader(stheader);
}

void XMSongExport::SaveSampleData(Song& song,const int instrIdx) {
	// pack sample data
	short* samples = song._pInstrument[instrIdx]->waveDataL;
	int length = song._pInstrument[instrIdx]->waveLength;
	short prev=0;
	for(int j=0;j<length;j++)
	{
		short delta =  samples[j] - prev;
		Write(delta);
		prev = samples[j];
	} 
}


void XMSongExport::SetEnvelopes(Song& song, int instrIdx, XMSAMPLEHEADER & sampleHeader)
{
	#if 0
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
		} else {
			// We can't ignore the loop because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			// IT also sets the Sustain points to the end of the envelope, but i can't see a reason for this to be needed.
			//inst.AmpEnvelope()->SustainBegin(inst.AmpEnvelope()->NumOfPoints()-1);
			//inst.AmpEnvelope()->SustainEnd(inst.AmpEnvelope()->NumOfPoints()-1);
		}

		
		if(sampleHeader.vtype & 4){
			if(sampleHeader.vloops < sampleHeader.vloope){
				inst.AmpEnvelope()->LoopStart(sampleHeader.vloops);
				inst.AmpEnvelope()->LoopEnd(sampleHeader.vloope);
			}
			// if loopstart >= loopend, Fasttracker ignores the loop!.
			// We can't ignore them because IT Envelopes do a fadeout when the envelope end is reached and FT does not.
			else {
				//inst.AmpEnvelope()->LoopStart(XMInstrument::Envelope::INVALID);
				//inst.AmpEnvelope()->LoopEnd(XMInstrument::Envelope::INVALID);
				inst.AmpEnvelope()->LoopStart(inst.AmpEnvelope()->NumOfPoints()-1);
				inst.AmpEnvelope()->LoopEnd(inst.AmpEnvelope()->NumOfPoints()-1);
			}
		} else {
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
	#endif
}	

///\todo: Actually, these methods should always write in Little endian mode.
bool XMSongExport::WriteHeader(XMFILEHEADER header) {
	Write(header.size);
	Write(header.norder);
	Write(header.restartpos);
	Write(header.channels);
	Write(header.patterns);
	Write(header.instruments);
	Write(header.flags);
	Write(header.speed);
	Write(header.tempo);
	return WriteArray(header.order,sizeof(header.order));
}

bool XMSongExport::WriteHeader(XMPATTERNHEADER header) {
	Write(header.size);
	Write(header.packingtype);
	Write(header.rows);
	return Write(header.packedsize);
}

bool XMSongExport::WriteHeader(XMINSTRUMENTHEADER header) {
	Write(header.size);
	WriteArray(header.name, sizeof(header.name));
	Write(header.type);
	return Write(header.samples);
}

bool XMSongExport::WriteHeader(XMSAMPLEHEADER header) {
	Write(header.shsize);
	WriteArray(header.snum, sizeof(header.snum));
	WriteArray(header.venv, sizeof(header.venv));
	WriteArray(header.penv, sizeof(header.penv));
	Write(header.vnum);
	Write(header.pnum);
	Write(header.vsustain);
	Write(header.vloops);
	Write(header.vloope);
	Write(header.psustain);
	Write(header.ploops);
	Write(header.ploope);
	Write(header.vtype);
	Write(header.ptype);
	Write(header.vibtype);
	Write(header.vibsweep);
	Write(header.vibdepth);
	Write(header.vibrate);
	Write(header.volfade);
	return WriteArray(header.reserved, sizeof(header.reserved));
}

bool XMSongExport::WriteHeader(XMSAMPLESTRUCT header) {
	Write(header.samplen);
	Write(header.loopstart);
	Write(header.looplen);
	Write(header.vol);
	Write(header.finetune);
	Write(header.type);
	Write(header.pan);
	Write(header.relnote);
	Write(header.res);
	return WriteArray(header.name, sizeof(header.name));
}

}}

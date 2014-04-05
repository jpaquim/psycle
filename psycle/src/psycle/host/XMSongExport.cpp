/** @file 
 *  @brief implementation file
 *  $Date: 2008-02-12 00:44:11 +0100 (mar, 12 feb 2008) $
 *  $Revision: 6303 $
 */

#include <psycle/host/detail/project.private.hpp>
#include "XMSongExport.hpp"
#include "Song.hpp"
#include "Sampler.hpp"

namespace psycle{
namespace host{


	XMSongExport::XMSongExport()
	{
	}

	XMSongExport::~XMSongExport()
	{

	}

	void XMSongExport::exportsong(const Song& song)
	{

		writeSongHeader(song);

		SavePatterns(song);
		SaveInstruments(song);

	}

	void XMSongExport::writeSongHeader(const Song &song)
	{
		macInstruments=0;
		bool hasSampler=false;
		for (int i=0; i<MAX_BUSES; i++) {
			if (song._pMachine[i] != NULL) {
				isSampler[i] = hasSampler = (song._pMachine[i]->_type == MACH_SAMPLER);
				isSampulse[i] = (song._pMachine[i]->_type == MACH_XMSAMPLER);
				if ( ! (isSampler[i] || isSampulse[i])) { macInstruments++; }
			}
			else {
				isSampler[i] = false;
				isSampulse[i] = false;
			}
		}
		//If the xminstrument 0 is not used, do not increase the instrument number (the loader does this so exporting a loaded song adds a new empty slot)
		correctionIndex = (song.xminstruments.IsEnabled(0))?1:0;
		xmInstruments = song.GetHighestXMInstrumentIndex()+1;
		if (xmInstruments > 1 && correctionIndex==0) xmInstruments--;
		//If there is a sampler machine, we have to take the samples into account.
		int samInstruments = (hasSampler) ? song.GetHighestInstrumentIndex()+1: 0;
		
		Write(XM_HEADER, 17);//ID text
		std::string name = "PE:" + song.name.substr(0,17);
		Write(name.c_str(), 20);//Module name
		uint16_t temp = 0x1A;
		Write(&temp, 1);
		Write("FastTracker v2.00   ", 20);//Tracker name
		temp = 0x0104;
		Write(&temp, 2);//Version number

		memset(&m_Header,0,sizeof(m_Header));
		m_Header.size = sizeof(m_Header);
		m_Header.norder = song.playLength;
		m_Header.restartpos = 0;
		m_Header.channels = std::min(song.SONGTRACKS,32);
		m_Header.patterns = song.GetHighestPatternIndexInSequence()+1;
		m_Header.instruments = std::min(128,macInstruments + xmInstruments + samInstruments);
		m_Header.flags = 0x0001; //Linear frequency.
		m_Header.speed = floor(24.f/song.LinesPerBeat()) + song.ExtraTicksPerLine();
		m_Header.tempo =  song.BeatsPerMin();

		//Pattern order table
		for (int i = 0; i < song.playLength; i++) {
			m_Header.order[i] =  song.playOrder[i];
		}
		Write(&m_Header,sizeof(m_Header));
	}

	void XMSongExport::SavePatterns(const Song & song)
	{
		for (int i = 0; i < m_Header.patterns ; i++)
		{
			SaveSinglePattern(song,i);
		}
	}

	// Load instruments
	void XMSongExport::SaveInstruments(const Song& song)
	{
		for (int i=0; i<MAX_BUSES; i++) {
			if (song._pMachine[i] != NULL && song._pMachine[i]->_type != MACH_SAMPLER
					&& song._pMachine[i]->_type != MACH_XMSAMPLER) {
				SaveEmptyInstrument(song._pMachine[i]->_editName);
			}
		}
		for (int j=0, i=(correctionIndex==0)?1:0 ; j < xmInstruments; j++, i++) {
			if (song.xminstruments.IsEnabled(i)) {
				SaveSampulseInstrument(song,i);
			}
			else {
				SaveEmptyInstrument(song.xminstruments.Exists(i) ? song.xminstruments[i].Name() : "");
			}
		}
		int remaining = m_Header.instruments - macInstruments - xmInstruments;
		for (int i = 0 ; i < remaining; i++ ){
			if (song.samples.IsEnabled(i)) {
				SaveSamplerInstrument(song,i);
			}
			else {
				SaveEmptyInstrument(song.samples.Exists(i) ? song.samples[i].WaveName() : "");
			}
		}
	}


	// return address of next pattern, 0 for invalid
	void XMSongExport::SaveSinglePattern(const Song & song, const int patIdx)
	{
		XMPATTERNHEADER ptHeader;
		memset(&ptHeader,0,sizeof(ptHeader));
		ptHeader.size = sizeof(ptHeader);
		//ptHeader.packingtype = 0; implicit from memset.
		ptHeader.rows = std::min(256,song.patternLines[patIdx]);
		//ptHeader.packedsize = 0; implicit from memset.

		Write(&ptHeader,sizeof(ptHeader));
		std::size_t currentpos = GetPos();

		int maxtracks = std::min(song.SONGTRACKS,32);
		// check every pattern for validity
		if (song.IsPatternUsed(patIdx))
		{
			for (int j = 0; j < ptHeader.rows; j++) {
				for (int i = 0; i < maxtracks; i++) {
					
					const PatternEntry* pData = reinterpret_cast<const PatternEntry*>(song._ptrackline(patIdx,i,j));
					
					if (pData->_note == notecommands::tweak || pData->_note == notecommands::tweakslide || 
						(pData->_mach >= MAX_BUSES && pData->_mach < MAX_MACHINES)) {
						continue;
					}

					
					unsigned char note;
					if (pData->_note >= 12 && pData->_note < 108 && pData->_mach < MAX_MACHINES) {
						if ( song._pMachine[pData->_mach] != NULL && isSampler[pData->_mach]
							&& ((Sampler*)song._pMachine[pData->_mach])->isDefaultC4() == false )
						{
							note = pData->_note +1;
						} else {
							note = pData->_note - 11;
						}
					}
					else if (pData->_note == notecommands::release) {
						note = 0x61;
					} else {
						note = 0x00;
					}
					
					unsigned char instr=0;
					
					if (pData->_mach < MAX_MACHINES && song._pMachine[pData->_mach] != NULL) {
						if (isSampler[pData->_mach] && pData->_inst != 0xFF) instr = macInstruments + xmInstruments + pData->_inst +1;
						else if (isSampulse[pData->_mach] && pData->_inst != 0xFF) instr = macInstruments + pData->_inst +correctionIndex;
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
								param = pData->_parameter/2;
								break;
							default:
								foundEffect = false;
								break;
						}
					}

					if ((foundEffect == false) && (pData->_cmd > 0)) {
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

					if (compressed != 0x9F) Write(&compressed,1); // 0x9F means to write everything.
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
		}
		else {
			Write(&ptHeader,sizeof(ptHeader));
		}
	}
	void XMSongExport::SaveEmptyInstrument(const std::string& name)
	{
		XMINSTRUMENTHEADER insHeader;
		memset(&insHeader,0,sizeof(insHeader));
		//insHeader.type = 0; Implicit by memset
		insHeader.size = sizeof(insHeader);
		strncpy(insHeader.name,name.c_str(),21);
		//insHeader.samples = 0; Implicit by memset
		Write(&insHeader,sizeof(insHeader));
	}
	void XMSongExport::SaveSampulseInstrument(const Song& song, int instIdx)
	{
		const XMInstrument &inst = song.xminstruments[instIdx];
		XMINSTRUMENTHEADER insHeader;
		memset(&insHeader,0,sizeof(insHeader));
		strncpy(insHeader.name,inst.Name().c_str(),21);
		//insHeader.type = 0; Implicit by memset
		XMSAMPLEHEADER samphead;
		std::memset(&samphead, 0, sizeof(samphead));

		std::set<int> sampidxs;
		for ( int i=0; i < XMInstrument::NOTE_MAP_SIZE; i++) {
			const XMInstrument::NotePair &pair = inst.NoteToSample(i);
			if (pair.second != 255 ) {
				sampidxs.insert(pair.second);
				samphead.snum[i] = pair.second;
			}
		}

		//If no samples for this instrument, write it and exit.
		if (sampidxs.size() == 0) {
			insHeader.size = sizeof(insHeader);
			//insHeader.samples = 0; Implicit by memset
			Write(&insHeader,sizeof(insHeader));
		}
		else {
			insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
			insHeader.samples = sampidxs.size();
			Write(&insHeader,sizeof(insHeader));

			SetSampulseEnvelopes(song, instIdx,samphead);
			samphead.volfade = static_cast<uint16_t>(floor(inst.VolumeFadeSpeed()*32768.0f));
			samphead.shsize = sizeof(XMSAMPLESTRUCT);
			Write(&samphead,sizeof(samphead));

			for (std::set<int>::iterator ite = sampidxs.begin(); ite != sampidxs.end(); ++ite) {
				SaveSampleHeader(song, *ite);
				SaveSampleData(song, *ite);
			}
		}
	}



	void XMSongExport::SaveSamplerInstrument(const Song& song, int instIdx)
	{
		XMINSTRUMENTHEADER insHeader;
		memset(&insHeader,0,sizeof(insHeader));
		strncpy(insHeader.name,song.samples[instIdx].WaveName().c_str(),21);
		//insHeader.type = 0; Implicit by memset

		//If it has samples, add the whole header.
		if (song.samples.IsEnabled(instIdx)) {
			insHeader.size = sizeof(insHeader) + sizeof(XMSAMPLEHEADER);
			insHeader.samples = 1;
			Write(&insHeader,sizeof(insHeader));

			XMSAMPLEHEADER _samph;
			std::memset(&_samph, 0, sizeof(_samph));
			SetSamplerEnvelopes(song,instIdx,_samph);
			_samph.volfade=0x400;
			_samph.shsize = sizeof(XMSAMPLESTRUCT);
			Write(&_samph,sizeof(_samph));

			SaveSampleHeader(song, instIdx);
			SaveSampleData(song, instIdx);
		}
		else {
			insHeader.size = sizeof(insHeader);
			//insHeader.samples = 0; Implicit by memset
			Write(&insHeader,sizeof(insHeader));
		}
	}

	void XMSongExport::SaveSampleHeader(const Song& song, int instIdx)
	{
		const XMInstrument::WaveData<>& wave = song.samples[instIdx];

		XMSAMPLESTRUCT stheader;
		memset(&stheader,0,sizeof(stheader));
		// stheader.name Implicitely set at zero by memset
		// stheader.res Implicitely set at zero by memset

		int tune = wave.WaveTune();
		int finetune = static_cast<int>((float)wave.WaveFineTune()*1.28);
		if (wave.WaveSampleRate() != 8363) {
			//correct the tuning
			double newtune = log10(double(wave.WaveSampleRate())/8363.0)/log10(2.0);
			double floortune = floor(newtune*12.0);
			tune += static_cast<int>(floortune);
			finetune += static_cast<int>(floor(((newtune*12.0)-floortune)*128.0));
			if (finetune > 127) { tune--; finetune -=127; }
		}

		//All samples are 16bits in Psycle.
		stheader.samplen = wave.WaveLength() *2;
		stheader.loopstart = wave.WaveLoopStart() * 2;
		stheader.looplen = (wave.WaveLoopEnd() - wave.WaveLoopStart()) * 2;
		stheader.vol = std::min(64,wave.WaveVolume()*64);
		stheader.relnote = tune;
		stheader.finetune = finetune;

		uint8_t type = 0;
		if (wave.WaveLoopType()==XMInstrument::WaveData<>::LoopType::NORMAL) type = 1;
		else if (wave.WaveLoopType()==XMInstrument::WaveData<>::LoopType::BIDI) type = 2;
		type += 0x10; // 0x10 -> 16bits
		stheader.type = type;
		stheader.pan = int(wave.PanFactor()*256)&0xFF;

		Write(&stheader,sizeof(stheader));
	}
	
	void XMSongExport::SaveSampleData(const Song& song,const int instrIdx)
	{
		const XMInstrument::WaveData<>& wave = song.samples[instrIdx];
		// pack sample data
		short* samples = wave.pWaveDataL();
		int length = wave.WaveLength();
		short prev=0;
		for(int j=0;j<length;j++)
		{
			short delta =  samples[j] - prev;
			//This is expected to be in little endian.
			Write(&delta,sizeof(short));
			prev = samples[j];
		} 
	}

	
	void XMSongExport::SetSampulseEnvelopes(const Song& song, int instrIdx, XMSAMPLEHEADER & sampleHeader)
	{
		sampleHeader.vtype = 0;
		const XMInstrument &inst = song.xminstruments[instrIdx];

		if ( inst.AmpEnvelope().IsEnabled() ) {
			sampleHeader.vtype = 1;
			const XMInstrument::Envelope & env = inst.AmpEnvelope();

			 // Max number of envelope points in Fasttracker format is 12.
			sampleHeader.vnum = std::min(12u, env.NumOfPoints());
			float convert = 1.f;
			if (env.Mode() == XMInstrument::Envelope::Mode::MILIS) {
				convert = (24.f*static_cast<float>(song.BeatsPerMin()))/60000.f;
			}
			// Format of FastTracker points is :
			// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
			// Value : 0..64. , divide by 64 to use it as a multiplier.
			int idx=0;
			for ( unsigned int i; i < env.NumOfPoints() && i < 24;i++) {
				sampleHeader.venv[idx]=static_cast<uint16_t>(env.GetTime(i)*convert); idx++;
				sampleHeader.venv[idx]=static_cast<uint16_t>(env.GetValue(i)*64.f); idx++;
			}

			if (env.SustainBegin() != XMInstrument::Envelope::INVALID) {
				sampleHeader.vtype &= 2;
				sampleHeader.vsustain = static_cast<uint16_t>(env.SustainBegin());
			}
			if (env.LoopStart() != XMInstrument::Envelope::INVALID) {
				sampleHeader.vtype &= 4;
				sampleHeader.vloops = static_cast<uint16_t>(env.LoopStart());
				sampleHeader.vloope = static_cast<uint16_t>(env.LoopEnd());
			}
		}
		if ( inst.PanEnvelope().IsEnabled() ) {
			sampleHeader.ptype = 1;
			const XMInstrument::Envelope & env = inst.PanEnvelope();

			 // Max number of envelope points in Fasttracker format is 12.
			sampleHeader.pnum = std::min(12u, env.NumOfPoints());
			float convert = 1.f;
			if (env.Mode() == XMInstrument::Envelope::Mode::MILIS) {
				convert = (24.f*static_cast<float>(song.BeatsPerMin()))/60000.f;
			}
			// Format of FastTracker points is :
			// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
			// Value : 0..64. , divide by 64 to use it as a multiplier.
			int idx=0;
			for ( unsigned int i; i < env.NumOfPoints() && i < 24;i++) {
				sampleHeader.penv[idx]=static_cast<uint16_t>(env.GetTime(i)*convert); idx++;
				sampleHeader.penv[idx]=static_cast<uint16_t>(env.GetValue(i)*64.f); idx++;
			}

			if (env.SustainBegin() != XMInstrument::Envelope::INVALID) {
				sampleHeader.ptype &= 2;
				sampleHeader.psustain = static_cast<uint16_t>(env.SustainBegin());
			}
			if (env.LoopStart() != XMInstrument::Envelope::INVALID) {
				sampleHeader.ptype &= 4;
				sampleHeader.ploops = static_cast<uint16_t>(env.LoopStart());
				sampleHeader.ploope = static_cast<uint16_t>(env.LoopEnd());
			}
		}
	}	
	void XMSongExport::SetSamplerEnvelopes(const Song& song, int instrIdx, XMSAMPLEHEADER & sampleHeader)
	{
		sampleHeader.vtype = 0;
		Instrument *inst = song._pInstrument[instrIdx];

		if (inst->ENV_AT != 1 || inst->ENV_DT != 1 || inst->ENV_SL != 100 || inst->ENV_RT != 16) {
			sampleHeader.vtype = 3;
			sampleHeader.vsustain = 1;

			sampleHeader.vnum = 4;
			float convert = 60.f/(44100*24.f*static_cast<float>(song.BeatsPerMin()));
			// Format of FastTracker points is :
			// Point : frame number. ( 1 frame= line*(24/TPB), samplepos= frame*(samplesperrow*TPB/24))
			// Value : 0..64. , divide by 64 to use it as a multiplier.
			int idx=0;
			sampleHeader.venv[idx]=0; idx++;
			sampleHeader.venv[idx]=0; idx++;
			sampleHeader.venv[idx]=static_cast<uint16_t>(inst->ENV_AT*convert); idx++;
			sampleHeader.venv[idx]=64; idx++;
			sampleHeader.venv[idx]=static_cast<uint16_t>((inst->ENV_AT+inst->ENV_DT)*convert); idx++;
			sampleHeader.venv[idx]=static_cast<uint16_t>(inst->ENV_SL*0.64f); idx++;
			sampleHeader.venv[idx]=static_cast<uint16_t>((inst->ENV_AT+inst->ENV_DT+inst->ENV_RT)*convert); idx++;
			sampleHeader.venv[idx]=0; idx++;
		}
	}


}
}

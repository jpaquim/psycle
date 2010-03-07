#include "XMSongLoader.hpp"

#include "ProgressDialog.hpp"

#include <psycle/core/song.h>
#include <psycle/core/machine.h>
#include <psycle/core/machinefactory.h>
#include <psycle/core/xminstrument.h>
#include <psycle/core/xmsampler.h>
#include <psycle/core/player.h>

#include <algorithm>
#include <cstring>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {
	
	int16_t const MODSongLoader::BIGMODPERIODTABLE[37 * 8] = { // (12note * 3oct + 1note) * 8fine
		907,900,894,887,881,875,868,862,856,850,844,838,832,826,820,814,
		808,802,796,791,785,779,774,768,762,757,752,746,741,736,730,725,
		720,715,709,704,699,694,689,684,678,675,670,665,660,655,651,646,
		640,636,632,628,623,619,614,610,604,601,597,592,588,584,580,575,
		570,567,563,559,555,551,547,543,538,535,532,528,524,520,516,513,
		508,505,502,498,494,491,487,484,480,477,474,470,467,463,460,457,
		453,450,447,444,441,437,434,431,428,425,422,419,416,413,410,407,
		404,401,398,395,392,390,387,384,381,379,376,373,370,368,365,363,
		360,357,355,352,350,347,345,342,339,337,335,332,330,328,325,323,
		320,318,316,314,312,309,307,305,302,300,298,296,294,292,290,288,
		285,284,282,280,278,276,274,272,269,268,266,264,262,260,258,256,
		254,253,251,249,247,245,244,242,240,239,237,235,233,232,230,228,
		226,225,224,222,220,219,217,216,214,212,211,209,208,206,205,203,
		202,200,199,198,196,195,193,192,190,189,188,187,185,184,183,181,
		180,179,177,176,175,174,172,171,170,169,167,166,165,164,163,161,
		160,159,158,157,156,155,154,152,151,150,149,148,147,146,145,144,
		143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
		127,126,125,125,123,123,122,121,120,119,118,118,117,116,115,114,
		113,113,112,111,110,109,109,108
	};

	XMSongLoader::XMSongLoader() {
		for(int i = 0; i < 32; ++i) {
			highOffset[i] = 0;
			memPortaUp[i] = 0;
			memPortaDown[i] = 0;
			memPortaNote[i] = 0;
			memPortaPos[i] = 0;
		}
		for(int i = 0; i < 256; ++i) {
			smpLen[i] = 0;
			smpFlags[i] = 0;
		}
	}

	void XMSongLoader::LoadInstrumentFromFile(XMSampler & sampler, int idx) {
		XMINSTRUMENTFILEHEADER fileheader;

		ReadHeader(fileheader);
		if ( !strcmp(fileheader.name,"Extended Instrument")) return;
		m_pSong->rInstrument(idx).Init();
		fileheader.name[22] = 0;
		m_pSong->rInstrument(idx).Name(fileheader.name);

		XMSAMPLEFILEHEADER _insheader;
		ReadHeader(_insheader);
		int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
			XMInstrument::WaveData::WaveForms::SQUARE,
			XMInstrument::WaveData::WaveForms::SAWDOWN,
			XMInstrument::WaveData::WaveForms::SAWUP
		};
		
		XMSAMPLEHEADER _insheaderb;
		memcpy(&_insheader,&_insheaderb+4,sizeof(XMSAMPLEFILEHEADER)-2);
		SetEnvelopes(m_pSong->rInstrument(idx),_insheaderb);

		std::uint32_t iSampleCount(0);
		for (std::uint32_t i=0; i<96; i++)
		{
			if (_insheader.snum[i] > iSampleCount) iSampleCount = _insheader.snum[i];
		}
		iSampleCount++;
		if (iSampleCount > 32) iSampleCount = 32;


		unsigned char *sRemap = new unsigned char[iSampleCount];
		unsigned int i;
		int curSample(0);
		// read instrument data	
		for(i=0;i<iSampleCount;i++)
		{
			while (m_pSong->SampleData(curSample).WaveLength() > 0 && curSample < MAX_INSTRUMENTS-1) curSample++;
			LoadSampleHeader(sampler,GetPos(),idx,curSample);
			// Only get REAL samples.
			if ( smpLen[curSample] > 0 && curSample < MAX_INSTRUMENTS-2 ) {	sRemap[i]=curSample; }
			else { sRemap[i]=MAX_INSTRUMENTS-1; }
		}

		// load individual samples
		for(i=0;i<iSampleCount;i++)
		{
			if ( sRemap[i] < MAX_INSTRUMENTS-1)
			{
				m_pSong->rInstrument(idx).IsEnabled(true);
				LoadSampleData(sampler,GetPos(),idx,sRemap[i]);

				//\todo : Improve autovibrato. (correct depth? fix for sweep?)
				XMInstrument::WaveData& _wave = m_pSong->SampleData(sRemap[i]);
				_wave.VibratoAttack(_insheader.vibsweep!=0?255/_insheader.vibsweep:255);
				_wave.VibratoDepth(_insheader.vibdepth<<1);
				_wave.VibratoSpeed(_insheader.vibrate);
				_wave.VibratoType(exchwave[_insheader.vibtype&3]);
			}
		}

		XMInstrument::NotePair npair;
		if ( _insheader.snum[0] < iSampleCount) npair.second=sRemap[_insheader.snum[0]];
		else npair.second=0;
		for(int i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
			npair.first=i;
			if (i< 12){
				//npair.second=_samph.snum[0]; implicit.
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			} else if(i < 108){
				if ( _insheader.snum[i] < iSampleCount) npair.second=sRemap[_insheader.snum[i-12]];
				else npair.second=curSample-1;
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			} else {
				//npair.second=_samph.snum[95]; implicit.
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			}
		}
		delete[] sRemap;
	}

	void XMSongLoader::Load(Song& song, bool fullopen) {
		// check validity
		if(!IsValid()){
			return;
		}
		m_pSong = &song;
		song.SetReady(false);
		// song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse",0);
		m_pSampler = (XMSampler*) MachineFactory::getInstance().CreateMachine(InternalKeys::sampulse);
		song.AddMachine(m_pSampler);
		song.InsertConnection(*m_pSampler,*song.machine(MASTER_INDEX),0,0,0.35f);
		song.seqBus=m_pSampler->id();
		// get song name
		char * pSongName = AllocReadStr(20,17);
		if(pSongName==NULL)
			return;
		song.setName(pSongName);
		song.setAuthor("");
		std::string imported = "Imported from FastTracker Module: ";
		imported.append(file_name());
		song.setComment(imported);
		delete [] pSongName; pSongName = 0;

		size_t iInstrStart = LoadPatterns(song);
		LoadInstruments(*m_pSampler,iInstrStart);
		song.SetReady(true);
	}

	bool XMSongLoader::IsValid() {			
		bool bIsValid = false;
		
		// get header
		char * pID = AllocReadStr(17,0);

		// tracker name	
		char * pTrackerName = AllocReadStr(20,38);
		
		// get tracker version
		char * pTrackerVer = AllocReadStr(2,58);	

		// process
		TRACE(_T("Header: %s\n"),pID);
		TRACE(_T("Tracker: %s\n"),pTrackerName);
		TRACE(_T("Fileformat Version %i.%i\n"),int(pTrackerVer[1]),int(pTrackerVer[0]));

		// check header
		bIsValid = (!stricmp(pID,XM_HEADER));


		// cleanup
		delete[] pID;
		delete[] pTrackerName;
		delete[] pTrackerVer;

		return bIsValid;
	}

	size_t XMSongLoader::LoadPatterns(Song & song) {
		// get data
		Seek(60);
		ReadHeader(m_Header);
		m_pSampler->IsAmigaSlides((m_Header.flags & 0x01)?false:true);
		m_pSampler->XMSampler::PanningMode(XMSampler::PanningMode::TwoWay);
		//using std::max;
		song.setTracks(std::max(m_Header.channels, std::uint16_t(4)));
		m_iInstrCnt = m_Header.instruments;
		song.BeatsPerMin(m_Header.tempo);
		song.LinesPerBeat(m_pSampler->Speed2LPB(m_Header.speed));
		double pos = 0;
		song.sequence().removeAll();
		// here we add in one single Line the patterns
		SequenceLine & line = song.sequence().createNewLine();

		// get pattern data
		size_t nextPatStart = m_Header.size + 60;
		for(int j = 0;j < m_Header.patterns && nextPatStart > 0;j++){
			nextPatStart = LoadPattern(song,nextPatStart,j,m_Header.channels);
		}
		for (int i = 0; i != m_Header.norder; ++i) {
			Pattern* pat = song.sequence().FindPattern(m_Header.order[i]);
			assert(pat);
			line.createEntry(*pat, pos);
			pos += pat->beats();
		}	
		return nextPatStart;
	}

	// Load instruments
	bool XMSongLoader::LoadInstruments(XMSampler & sampler, size_t iInstrStart) {	
		int currentSample=0;
		for(int i = 1;i <= m_iInstrCnt;i++){
			iInstrStart = LoadInstrument(sampler,iInstrStart,i,currentSample);
			TRACE2("%d %s\n",i,m_pSong->rInstrument(i).Name().c_str());
		}
		return true;
	}

	char * XMSongLoader::AllocReadStr(const int32_t size, size_t start) {
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
		if(ReadArray(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}

	// return address of next pattern, 0 for invalid
	size_t XMSongLoader::LoadPattern(Song & song, size_t start, int patIdx, int iTracks) {
		Seek(start);
		int iHeaderLen = ReadInt4();
		Skip(1); //char iPackingType = ReadInt1();
		short iNumRows = ReadInt2();
		short iPackedSize = ReadInt2();

		Pattern & pat = *new Pattern();
		pat.setName("unnamed");
		pat.setID(patIdx);
		song.sequence().Add(pat);

		PatternEvent e;

		if(iPackedSize == 0)
		{
			//Pattern is emtpy.
		}	
		else
		{
			// get next values
			for(int row = 0;row < iNumRows;row++)
			{
				for(int col=0;col<iTracks;col++)
				{	
					// reset
					unsigned char note=notecommands::empty;
					unsigned char instr=255;
					unsigned char vol=0;
					unsigned char type=0;
					unsigned char param=0;

					// read note
					note = ReadInt1();

					// is compression bit set?
					if(note & 0x80)
					{
						unsigned char bReadNote = note&0x01;
						unsigned char bReadInstr = note&0x02;
						unsigned char bReadVol = note&0x04;
						unsigned char bReadType = note&0x08;
						unsigned char bReadParam  = note&0x10;

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
					e.setInstrument(instr);	
					e.setMachine( 0);
					e.setVolume(255);

					// volume/command
					if(vol >= 0x10 && vol <= 0x50)
					{
						e.setVolume((vol == 0x50)?0x3F:(vol-0x10));

					} else if(vol >= 0x60){						
						switch(vol&0xF0)
						{
						case XMVOL_CMD::XMV_VOLUMESLIDEDOWN:
							e.setVolume(XMSampler::CMD_VOL::VOL_VOLSLIDEDOWN|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_VOLUMESLIDEUP:
							e.setVolume(XMSampler::CMD_VOL::VOL_VOLSLIDEUP|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_FINEVOLUMESLIDEDOWN:
							e.setVolume(XMSampler::CMD_VOL::VOL_FINEVOLSLIDEDOWN|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_FINEVOLUMESLIDEUP:
							e.setVolume(XMSampler::CMD_VOL::VOL_FINEVOLSLIDEUP|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_PANNING:
							e.setVolume(XMSampler::CMD_VOL::VOL_PANNING|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_PANNINGSLIDELEFT:
							// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
							e.setVolume(XMSampler::CMD_VOL::VOL_PANSLIDELEFT|((vol&0x0F)>>2));
							break;
						case XMVOL_CMD::XMV_PANNINGSLIDERIGHT:
							// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
							e.setVolume(XMSampler::CMD_VOL::VOL_PANSLIDERIGHT|((vol&0x0F)>>2));
							break;
// Ignoring this command for now.
//						case XMVOL_CMD::XMV_VIBRATOSPEED:
//							e.setVolume(XMSampler::CMD_VOL::VOL_VIBRATO_SPEED|(vol&0x0F));
//							break;
						case XMVOL_CMD::XMV_VIBRATO:
							e.setVolume(XMSampler::CMD_VOL::VOL_VIBRATO|(vol&0x0F));
							break;
						case XMVOL_CMD::XMV_PORTA2NOTE:
							e.setVolume(XMSampler::CMD_VOL::VOL_TONEPORTAMENTO|(vol&0x0F));
							break;
						default:
							break;
						}
					}
					e.setParameter(param);
					int exchwave[3]={XMInstrument::WaveData::WaveForms::SINUS,
						XMInstrument::WaveData::WaveForms::SAWDOWN,
						XMInstrument::WaveData::WaveForms::SQUARE
					};
					switch(type){
						case XMCMD::ARPEGGIO:
							if(param != 0){
								e.setCommand(XMSampler::CMD::ARPEGGIO);
							} else {
								e.setCommand(XMSampler::CMD::NONE);
							}
							break;
						case XMCMD::PORTAUP:
							e.setCommand(XMSampler::CMD::PORTAMENTO_UP);
							if ( e.parameter() == 0) {
								if ( memPortaPos[col] != 1 )
								{
									e.setParameter(memPortaUp[col]);
								}
							}
							else {
								if ( e.parameter() > 0xDF ) { e.setParameter(0xDF); }
								memPortaUp[col] = e.parameter();
							}
							memPortaPos[col] = 1;
							break;
						case XMCMD::PORTADOWN:
							e.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
							if ( e.parameter() == 0) {
								if ( memPortaPos[col] != 2 )
								{
									e.setParameter(memPortaDown[col]);
								}
							}
							else {
								if ( e.parameter() > 0xDF ) { e.setParameter(0xDF); }
								memPortaDown[col] = e.parameter();
							}
							memPortaPos[col] = 2;
							break;
						case XMCMD::PORTA2NOTE:
							e.setCommand(XMSampler::CMD::PORTA2NOTE);
							if ( e.parameter() == 0) {
								if ( memPortaPos[col] != 3 )
								{
									e.setParameter(memPortaNote[col]);
								}
							}
							else {
								memPortaNote[col] = e.parameter();
							}
							memPortaPos[col] = 3;
							break;
						case XMCMD::VIBRATO:
							e.setCommand(XMSampler::CMD::VIBRATO);
							break;
						case XMCMD::TONEPORTAVOL:
							e.setCommand(XMSampler::CMD::TONEPORTAVOL);
							break;
						case XMCMD::VIBRATOVOL:
							e.setCommand(XMSampler::CMD::VIBRATOVOL);
							break;
						case XMCMD::TREMOLO:
							e.setCommand(XMSampler::CMD::TREMOLO);
							break;
						case XMCMD::PANNING:
							e.setCommand(XMSampler::CMD::PANNING);
							break;
						case XMCMD::OFFSET:
							e.setCommand(XMSampler::CMD::OFFSET | highOffset[col]); 
							break;
						case XMCMD::VOLUMESLIDE:
							e.setCommand(XMSampler::CMD::VOLUMESLIDE);
							break;
						case XMCMD::POSITION_JUMP:
							e.setCommand(PatternCmd::JUMP_TO_ORDER);
							break;
						case XMCMD::VOLUME:
							e.setCommand(XMSampler::CMD::VOLUME);
							e.setParameter(param * 2);
							break;
						case XMCMD::PATTERN_BREAK:
							e.setCommand(PatternCmd::BREAK_TO_LINE);
							e.setParameter(((param&0xF0)>>4)*10 + (param&0x0F));
							break;
						case XMCMD::EXTENDED:
							switch(param & 0xf0){
							case XMCMD_E::E_FINE_PORTA_UP:
								e.setCommand(XMSampler::CMD::PORTAMENTO_UP);
								e.setParameter(0xF0+(param&0x0F));
								break;
							case XMCMD_E::E_FINE_PORTA_DOWN:
								e.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
								e.setParameter(0xF0+(param&0x0F));
								break;
							case XMCMD_E::E_GLISSANDO_STATUS:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMSampler::CMD_E::E_GLISSANDO_TYPE | ((param==0)?0:1));
								break;
							case XMCMD_E::E_VIBRATO_WAVE:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[param & 0x3]);
								break;
							case XMCMD_E::E_FINETUNE:
								e.setCommand(XMSampler::CMD::NONE);
								e.setParameter( 0);
								break;
							case XMCMD_E::E_PATTERN_LOOP:
								e.setCommand(PatternCmd::EXTENDED);
								e.setParameter(PatternCmd::PATTERN_LOOP + (param & 0xf));
								break;
							case XMCMD_E::E_TREMOLO_WAVE:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3]);
								break;
							case XMCMD_E::E_MOD_RETRIG:
								e.setCommand(XMSampler::CMD::RETRIG);
								e.setParameter(param & 0xf);
								break;
							case XMCMD_E::E_FINE_VOLUME_UP:
								e.setCommand(XMSampler::CMD::VOLUMESLIDE);
								e.setParameter(0xf0 + (param & 0xf));
								break;
							case XMCMD_E::E_FINE_VOLUME_DOWN:
								e.setCommand(XMSampler::CMD::VOLUMESLIDE);
								e.setParameter(0x0f + ((param & 0xf)<<4));
								break;
							case XMCMD_E::E_DELAYED_NOTECUT:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMSampler::CMD_E::E_DELAYED_NOTECUT | (param & 0xf));
								break;
							case XMCMD_E::E_NOTE_DELAY:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf));
								break;
							case XMCMD_E::E_PATTERN_DELAY:
								e.setCommand(PatternCmd::EXTENDED);
								e.setParameter(PatternCmd::PATTERN_DELAY | (param & 0xf));
								break;
							case XMCMD_E::E_SET_MIDI_MACRO:
								e.setCommand(XMSampler::CMD::EXTENDED);
								e.setParameter(XMCMD::MIDI_MACRO | (param & 0x0f));
								break;
							default:
								e.setCommand(XMSampler::CMD::NONE);
								break;
							}
							break;
						case XMCMD::SETSPEED:
							if ( param < 32)
							{
								e.setCommand(PatternCmd::EXTENDED);
								e.setParameter(24 / ((param == 0)?6:param));
							}
							else
							{
								e.setCommand(PatternCmd::SET_TEMPO);
							}
							break;
						case XMCMD::SET_GLOBAL_VOLUME:
							e.setCommand(XMSampler::CMD::SET_GLOBAL_VOLUME);
							if (param >= 0x80) e.setParameter(0xFF);
							else e.setParameter(param*2);
							break;
						case XMCMD::GLOBAL_VOLUME_SLIDE:
							e.setCommand(XMSampler::CMD::GLOBAL_VOLUME_SLIDE);
							//Double the parameter, since FT2's range is 0-0x40.
							if ( (param & 0x0F) == 0 || (param & 0x0F) == 0xF){ // Slide up
								e.setParameter((param & 0xF0)>>4);
								e.setParameter(e.parameter()>7?15:e.parameter()*2);
								e.setParameter(e.parameter()<<4);
								e.setParameter(e.parameter() | (param & 0x0F));
							}
							else if ( (param & 0xF0) == 0 || (param & 0xF0) == 0xF0)  { // Slide down
								e.setParameter((param & 0x0F));
								e.setParameter(e.parameter()>7?15:e.parameter()*2);
								e.setParameter(e.parameter() | (param & 0xF0));
							}
							break;
						case XMCMD::NOTE_OFF:
							e.setCommand(XMSampler::CMD::VOLUME);
							e.setParameter( 0);
							break;
						case XMCMD::SET_ENV_POSITION:
							e.setCommand(XMSampler::CMD::SET_ENV_POSITION);
							break;
						case XMCMD::PANNINGSLIDE:
							e.setCommand(XMSampler::CMD::PANNINGSLIDE);
							break;
						case XMCMD::RETRIG:
							e.setCommand(XMSampler::CMD::RETRIG);
							break;
						case XMCMD::TREMOR:
							e.setCommand(XMSampler::CMD::TREMOR);
							break;
						case XMCMD::EXTEND_XM_EFFECTS:
							switch(param & 0xf0){
							case XMCMD_X::X_EXTRA_FINE_PORTA_DOWN:
								e.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
								e.setParameter(0xE0 | (param & +0xf));
								break;
							case XMCMD_X::X_EXTRA_FINE_PORTA_UP:
								e.setCommand(XMSampler::CMD::PORTAMENTO_UP);
								e.setParameter(0xE0 | (param & +0xf));
								break;
							case XMCMD_X::X9:
								switch ( param & 0xf){
								case XMCMD_X9::X9_SURROUND_OFF:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_SURROUND_OFF);
									break;
								case XMCMD_X9::X9_SURROUND_ON:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_SURROUND_ON);
									break;
								case XMCMD_X9::X9_REVERB_OFF:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_REVERB_OFF);
									break;
								case XMCMD_X9::X9_REVERB_FORCE:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_REVERB_FORCE);
									break;
								case XMCMD_X9::X9_STANDARD_SURROUND:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_STANDARD_SURROUND);
									break;
								case XMCMD_X9::X9_QUAD_SURROUND:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_QUAD_SURROUND);
									break;
								case XMCMD_X9::X9_GLOBAL_FILTER:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_GLOBAL_FILTER);
									break;
								case XMCMD_X9::X9_LOCAL_FILTER:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_LOCAL_FILTER);
									break;
								case XMCMD_X9::X9_PLAY_FORWARD:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_PLAY_FORWARD);
									break;
								case XMCMD_X9::X9_PLAY_BACKWARD:
									e.setCommand(XMSampler::CMD::EXTENDED);
									e.setParameter(XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_PLAY_BACKWARD);
									break;
								default:
									e.setCommand(XMSampler::CMD::NONE);
									break;
								}
								break;
							case XMCMD_X::X_HIGH_OFFSET:
								highOffset[col] = param & 0x0F;
								break;
							default:
								e.setCommand(XMSampler::CMD::NONE);
								break;
							}
							break;
						case XMCMD::PANBRELLO:
							e.setCommand(XMSampler::CMD::PANBRELLO);
							break;
						case XMCMD::MIDI_MACRO:
							e.setCommand(XMSampler::CMD::MIDI_MACRO);
							break;
						default:
							e.setCommand(XMSampler::CMD::NONE);
							break;
					}
					// instrument/note
					note = note & 0x7f;
					switch(note)
					{
						case 0x00: 
							e.setNote(notecommands::empty);
							break;// no note

						case 0x61:
							e.setNote(notecommands::release);
							e.setInstrument(255);
							e.setMachine( 0);
							break;// noteoff		
						
						default: 
							if(note >= 96 || note < 0)
								TRACE(_T("invalid note\n"));
							e.setNote(note+11); // +11 -> +12 ( FT2 C-0 is Psycle's C-1) -1 ( Ft2 C-0 is value 1)

							break;	// transpose
					}


					if ((e.note() == notecommands::empty) && (e.command() == 00) && (e.parameter() == 00) && (e.instrument() == 255) && (e.volume() == 255))
					{
						e.setMachine(255);
					}
					e.set_track(col);
					double beat = row / static_cast<float>(XMSampler::Speed2LPB(m_Header.speed));
					if (!e.empty())
						pat.insert(beat, e);
				}
			}
		}

		//int z = ftell(_file);
		return start + iPackedSize + iHeaderLen;

	}

	size_t XMSongLoader::LoadInstrument(XMSampler & sampler, size_t iStart, int idx,int &curSample) {
		Seek(iStart);

		// read header
		int iInstrSize = ReadInt4();
		//assert(iInstrSize==0x107||iInstrSize==0x21); // Skale Tracker (or MadTracker or who knows which more) don't have the "reserved[20]" parameter in the XMSAMPLEHEADER
		char sInstrName[23] = {0}; ///\todo it's probably useless to zero-initialise the array content
		ReadArray(sInstrName,22);
		sInstrName[22]= 0;

		Skip(1); //int iInstrType = ReadInt1();
		int iSampleCount = ReadInt2();

		if(iSampleCount>1)
 			TRACE(_T("ssmple count = %d\n"),iSampleCount);

		m_pSong->rInstrument(idx).Name(sInstrName);
		iStart += iInstrSize;

		if(iSampleCount==0)
			return iStart;

        
		XMSAMPLEHEADER _samph;
		std::memset(&_samph, 0, sizeof _samph);
		ReadHeader(_samph);
		
		int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
			XMInstrument::WaveData::WaveForms::SQUARE,
			XMInstrument::WaveData::WaveForms::SAWDOWN,
			XMInstrument::WaveData::WaveForms::SAWUP
		};		

		SetEnvelopes(m_pSong->rInstrument(idx),_samph);

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
				m_pSong->rInstrument(idx).IsEnabled(true);
				iStart = LoadSampleData(sampler,iStart,idx,sRemap[i]);

				//\todo : Improve autovibrato. (correct depth? fix for sweep?)
				XMInstrument::WaveData& _wave = m_pSong->SampleData(sRemap[i]);
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
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			} else if(i < 108){
				if ( _samph.snum[i] < iSampleCount) npair.second=sRemap[_samph.snum[i-12]];
				else npair.second=curSample-1;
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			} else {
				//npair.second=_samph.snum[95]; implicit.
				m_pSong->rInstrument(idx).NoteToSample(i,npair);
			}
		}
		delete[] sRemap;
		return iStart;
	}

	size_t XMSongLoader::LoadSampleHeader(XMSampler & sampler, size_t iStart, int iInstrIdx, int iSampleIdx) {
		// get sample header
		XMSAMPLESTRUCT header;
		std::memset(&header, 0, sizeof header);
		Seek(iStart);
		ReadHeader(header);
		header.name[21]='\0';

		uint32_t iLen = header.samplen;
		// parse
		BOOL bLoop = (header.type & 0x01 || header.type & 0x02) && (header.looplen>0);
		BOOL bPingPong = header.type & 0x02;
		BOOL b16Bit = header.type & 0x10;
	
		// alloc wave memory

		assert(iLen < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.

		XMInstrument::WaveData& _wave = m_pSong->SampleData(iSampleIdx);
		
		_wave.Init();
		if(iLen > 0) { // Sounds Stupid, but it isn't. Some modules save sample header when there is no sample.
			_wave.AllocWaveData(b16Bit?iLen / 2:iLen,false);
			_wave.WaveLength(b16Bit?iLen / 2:iLen);
		}
		else _wave.WaveLength(0);
		_wave.PanEnabled(true);
		_wave.PanFactor(header.pan/255.0f);
		//XMInstrument::WaveData& _data = sampler.Instrument(iInstrIdx).rWaveData(0).
		//sampler.Instrument(iInstrIdx).rWaveData()..Name() = sName;
		
		if(bLoop) {
			if(bPingPong){
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::BIDI);
			} else {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
			}
			if(b16Bit) {
				_wave.WaveLoopStart(header.loopstart / 2);
				_wave.WaveLoopEnd((header.looplen  + header.loopstart )/ 2);
			} else {
				_wave.WaveLoopStart(header.loopstart);
				_wave.WaveLoopEnd(header.looplen + header.loopstart);
			}
			//TRACE2("l:%x s:%x e:%x \n",_wave.WaveLength(),_wave.WaveLoopStart(),_wave.WaveLoopEnd()); 
		} else {
			_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
		}


		_wave.WaveVolume(header.vol * 2);
		_wave.WaveTune(header.relnote);
		_wave.WaveFineTune(header.finetune*2); // WaveFineTune has double range.
		std::string sName = header.name;
		_wave.WaveName(sName);

		smpLen[iSampleIdx] = iLen;
		smpFlags[iSampleIdx] = header.type;

		return iStart + 40;
	}

	size_t XMSongLoader::LoadSampleData(XMSampler & sampler, size_t iStart, int iInstrIdx, int iSampleIdx) {
		// parse
		
		BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
		XMInstrument::WaveData& _wave =  m_pSong->SampleData(iSampleIdx);
		short wNew=0;

		// cache sample data
		Seek(iStart);
		char * smpbuf = new char[smpLen[iSampleIdx]];
		memset(smpbuf,0,smpLen[iSampleIdx]);
		ReadArray(smpbuf,smpLen[iSampleIdx]);

		uint32_t sampleCnt = smpLen[iSampleIdx];

		// unpack sample data
		if(b16Bit) {				
			// 16 bit mono sample, delta
			int out=0;
			for(uint32_t j = 0; j < sampleCnt; j += 2) {
				wNew += smpbuf[j] | (smpbuf[j+1]<<8);
				*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
				out++;
			}   
		} else {
			// 8 bit mono sample
			for(uint32_t j = 0; j < sampleCnt; ++j) {			
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
	
	void XMSongLoader::SetEnvelopes(XMInstrument & inst, const XMSAMPLEHEADER & sampleHeader) {
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

	}	

	MODSongLoader::MODSongLoader(void) {
		for(int i = 0; i < 32; ++i) smpLen[i] = 0;
	}

	void MODSongLoader::Load(Song& song,const bool fullopen) {
		// check validity
		if(!IsValid()){
			return;
		}
		m_pSong = &song;
		song.SetReady(false);
		m_pSampler = (XMSampler*) MachineFactory::getInstance().CreateMachine(InternalKeys::sampulse);
		song.AddMachine(m_pSampler);
		//song.InsertConnection(*m_pSampler,*s->machine(MASTER_INDEX),0,0,0.75f); // This is done later, when determining the number of channels.
		song.seqBus=m_pSampler->id();

		// get song name
		char * pSongName = AllocReadStr(20,0);
		if(pSongName==NULL)
			return;

		song.setName(pSongName);
		song.setAuthor("");
		std::string imported = "Imported from MOD Module: ";
		imported.append(file_name());
		song.setComment(imported);
		delete [] pSongName; pSongName = 0;

		// get data
		Seek(20);
		for (int i=0;i<31;i++) LoadSampleHeader(*m_pSampler,i);
		Seek(950);
		ReadHeader(m_Header);
		
		char pID[5];
		pID[0]=m_Header.pID[0];pID[1]=m_Header.pID[1];pID[2]=m_Header.pID[2];pID[3]=m_Header.pID[3];pID[4]=0;
		
		
		m_pSampler->IsAmigaSlides(true);

		float volume = 0.7f;
		if ( !stricmp(pID,"M.K.")) { song.setTracks(4); }
		else if ( !stricmp(pID,"M!K!")) { song.setTracks(4); }
		else if ( !stricmp(pID+1,"CHN")) { char tmp[2]; tmp[0] = pID[0]; tmp[1]=0; song.setTracks(atoi(tmp)); volume= 0.5f; }
		else if ( !stricmp(pID+2,"CH")) { char tmp[3]; tmp[0] = pID[0]; tmp[1]=pID[1]; tmp[2]=0; song.setTracks(atoi(tmp)); volume = 0.35f; }
		song.InsertConnection(*m_pSampler,*song.machine(MASTER_INDEX),0,0,volume);
		song.BeatsPerMin(125);
		song.LinesPerBeat(4);

		for (int i = 0; i< song.tracks() ; i++ )
		{
			if (i%4 == 0 || i%4 == 3) m_pSampler->rChannel(i).DefaultPanFactorFloat(0.25f,true);
			else m_pSampler->rChannel(i).DefaultPanFactorFloat(0.75,true);
		}

		LoadPatterns(song);
		for(int i = 0;i < 31;i++){
			LoadInstrument(*m_pSampler,i);
		}
		song.SetReady(true);
	}

	bool MODSongLoader::IsValid() {
		bool bIsValid = false;
		char *pID = AllocReadStr(4,1080);

		bIsValid = !stricmp(pID,"M.K.");
		if ( !bIsValid ) bIsValid = !stricmp(pID,"M!K!");
		if ( !bIsValid ) bIsValid = !stricmp(pID+1,"CHN");
		if ( !bIsValid ) bIsValid = !stricmp(pID+2,"CH");

		delete[] pID;
		return bIsValid;
	}

	void MODSongLoader::LoadPatterns(Song & song) {

		double pos = 0;
		song.sequence().removeAll();
		// here we add in one single Line the patterns
		SequenceLine & line = song.sequence().createNewLine();

		int npatterns = 0;
		for(int i = 0; i < m_Header.songlength; ++i) {
			if ( m_Header.order[i] > npatterns) npatterns=m_Header.order[i];
		}
		++npatterns;

		// get pattern data
		Seek(1084);
		for(int j = 0; j < npatterns; ++j){
			LoadPattern(song,j,song.tracks() );
		}
		for(int i = 0; i < m_Header.songlength; ++i) {
			Pattern * pat = song.sequence().FindPattern(m_Header.order[i]);
			assert(pat);
			line.createEntry(*pat, pos);
			pos += pat->beats();
		}	
	}

	char * MODSongLoader::AllocReadStr(int32_t size, size_t start) {
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
		if(ReadArray(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}

	// return address of next pattern, 0 for invalid
	void MODSongLoader::LoadPattern(Song & song, int patIdx, int iTracks) {
		short iNumRows = 64;

		Pattern & pat = *new Pattern();
		pat.setName("unnamed");
		pat.setID(patIdx);
		song.sequence().Add(pat);

		PatternEvent e;
		unsigned char mentry[4];

			// get next values
			for(int row = 0;row < iNumRows;row++)
			{
				for(int col=0;col<iTracks;col++)
				{	
					// reset
					unsigned char note=notecommands::empty;
					unsigned char instr=255;
					unsigned char type=0;
					unsigned char param=0;
					unsigned short period=428;

					// read note
					mentry[0] = ReadUInt1(); mentry[1] = ReadUInt1(); mentry[2] = ReadUInt1(); mentry[3] = ReadUInt1();
					instr = ((mentry[0] & 0xF0) + (mentry[2] >> 4));
					period = ((mentry[0]& 0x0F) << 8) + mentry[1];
					type = (mentry[2] & 0x0F);
					param = mentry[3];
					note = ConvertPeriodtoNote(period);

					// translate
					if ( instr != 0 ) e.setInstrument(instr-1);
					else e.setInstrument(255);
					e.setMachine( 0);
					e.setParameter(param);

					int exchwave[3]={XMInstrument::WaveData::WaveForms::SINUS,
						XMInstrument::WaveData::WaveForms::SAWDOWN,
						XMInstrument::WaveData::WaveForms::SQUARE
					};

					switch(type){
						case XMCMD::ARPEGGIO:
							if(param != 0){
								e.setCommand(XMSampler::CMD::ARPEGGIO);
							} else {
								e.setCommand(XMSampler::CMD::NONE);
							}
							break;
						case XMCMD::PORTAUP:
							e.setCommand(XMSampler::CMD::PORTAMENTO_UP);
							break;
						case XMCMD::PORTADOWN:
							e.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
							break;
						case XMCMD::PORTA2NOTE:
							e.setCommand(XMSampler::CMD::PORTA2NOTE);
							break;
						case XMCMD::VIBRATO:
							e.setCommand(XMSampler::CMD::VIBRATO);
							break;
						case XMCMD::TONEPORTAVOL:
							e.setCommand(XMSampler::CMD::TONEPORTAVOL);
							break;
						case XMCMD::VIBRATOVOL:
							e.setCommand(XMSampler::CMD::VIBRATOVOL);
							break;
						case XMCMD::TREMOLO:
							e.setCommand(XMSampler::CMD::TREMOLO);
							break;
						case XMCMD::PANNING:
							e.setCommand(XMSampler::CMD::PANNING);
							break;
						case XMCMD::OFFSET:
							e.setCommand(XMSampler::CMD::OFFSET);
							break;
						case XMCMD::VOLUMESLIDE:
							e.setCommand(XMSampler::CMD::VOLUMESLIDE);
							break;
						case XMCMD::POSITION_JUMP:
							e.setCommand(PatternCmd::JUMP_TO_ORDER);
							break;
						case XMCMD::VOLUME:
							e.setCommand(XMSampler::CMD::VOLUME);
							e.setParameter(param<=0x40?param*2:0x80);
							break;
						case XMCMD::PATTERN_BREAK:
							e.setCommand(PatternCmd::BREAK_TO_LINE);
							e.setParameter(((param&0xF0)>>4)*10 + (param&0x0F));
							break;
						case XMCMD::EXTENDED:
							switch(param & 0xf0){
						case XMCMD_E::E_FINE_PORTA_UP:
							e.setCommand(XMSampler::CMD::PORTAMENTO_UP);
							e.setParameter(0xF0+(param&0x0F));
							break;
						case XMCMD_E::E_FINE_PORTA_DOWN:
							e.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
							e.setParameter(0xF0+(param&0x0F));
							break;
						case XMCMD_E::E_GLISSANDO_STATUS:
							e.setCommand(XMSampler::CMD::EXTENDED);
							e.setParameter(XMSampler::CMD_E::E_GLISSANDO_TYPE | ((param==0)?0:1));
							break;
						case XMCMD_E::E_VIBRATO_WAVE:
							e.setCommand(XMSampler::CMD::EXTENDED);
							e.setParameter(XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[param & 0x3]);
							break;
						case XMCMD_E::E_FINETUNE:
							e.setCommand(XMSampler::CMD::NONE);
							e.setParameter( 0);
							break;
						case XMCMD_E::E_PATTERN_LOOP:
							e.setCommand(PatternCmd::EXTENDED);
							e.setParameter(PatternCmd::PATTERN_LOOP | (param & 0xf));
							break;
						case XMCMD_E::E_TREMOLO_WAVE:
							e.setCommand(XMSampler::CMD::EXTENDED);
							e.setParameter(XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3]);
							break;
						case XMCMD_E::E_MOD_RETRIG:
							e.setCommand(XMSampler::CMD::RETRIG);
							e.setParameter(param & 0xf);
							break;
						case XMCMD_E::E_FINE_VOLUME_UP:
							e.setCommand(XMSampler::CMD::VOLUMESLIDE);
							e.setParameter(0xf0 + (param & 0xf));
							break;
						case XMCMD_E::E_FINE_VOLUME_DOWN:
							e.setCommand(XMSampler::CMD::VOLUMESLIDE);
							e.setParameter(0x0f + ((param & 0xf)<<4));
							break;
						case XMCMD_E::E_DELAYED_NOTECUT:
							e.setCommand(XMSampler::CMD::EXTENDED);
							e.setParameter(XMSampler::CMD_E::E_DELAYED_NOTECUT | (param & 0xf));
							break;
						case XMCMD_E::E_NOTE_DELAY:
							e.setCommand(XMSampler::CMD::EXTENDED);
							e.setParameter(XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf));
							break;
						case XMCMD_E::E_PATTERN_DELAY:
							e.setCommand(PatternCmd::EXTENDED);
							e.setParameter(PatternCmd::PATTERN_DELAY | (param & 0xf));
							break;
						default:
							e.setCommand(XMSampler::CMD::NONE);
							e.setParameter( 0);
							break;
							}
							break;
						case XMCMD::SETSPEED:
							if ( param < 32)
							{
								e.setCommand(PatternCmd::EXTENDED);
								e.setParameter(24 / ((param == 0)?6:param));
							}
							else
							{
								e.setCommand(PatternCmd::SET_TEMPO);
							}
							break;
						default:
							e.setCommand(XMSampler::CMD::NONE);
							break;
					}
					// instrument/note
					if ( note != 255 ) e.setNote(note+12);
					else e.setNote(note);

					if ((e.note() == notecommands::empty) && (e.command() == 00) && (e.parameter() == 00) && (e.instrument() == 255))
					{
						e.setMachine(255);
					}
					e.set_track(col);
					double beat = row *0.25;
					if(!e.empty()) 
						pat.insert(beat, e);
				}
			}
	}

	unsigned char MODSongLoader::ConvertPeriodtoNote(unsigned short period) {
		for(int count2 = 1; count2 < 37; ++count2) {
			if (period > BIGMODPERIODTABLE[count2*8]-2 && period < BIGMODPERIODTABLE[count2*8]+2 )
				return count2-1+36; // three octaves above.
		}
		return 255;
	}

	void MODSongLoader::LoadInstrument(XMSampler & sampler, int idx) {
		m_pSong->rInstrument(idx).Init();
		m_pSong->rInstrument(idx).Name(m_Samples[idx].sampleName);

		if (m_Samples[idx].sampleLength > 0 ) 
		{
			m_pSong->rInstrument(idx).IsEnabled(true);
			LoadSampleData(sampler,idx);
		}

		int i;
		XMInstrument::NotePair npair;
		npair.second=idx;
		for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
			npair.first=i;
			m_pSong->rInstrument(idx).NoteToSample(i,npair);
		}
	}

	void MODSongLoader::LoadSampleHeader(XMSampler & sampler, int iInstrIdx) {
		ReadArray(m_Samples[iInstrIdx].sampleName,22);	m_Samples[iInstrIdx].sampleName[21]='\0';

		smpLen[iInstrIdx] = (ReadUInt1()*0x100+ReadUInt1())*2; 
		m_Samples[iInstrIdx].sampleLength = smpLen[iInstrIdx];
		m_Samples[iInstrIdx].finetune = ReadUInt1();
		m_Samples[iInstrIdx].volume = ReadUInt1();
		m_Samples[iInstrIdx].loopStart =(ReadUInt1()*256+ReadUInt1())*2; 
		m_Samples[iInstrIdx].loopLength = (ReadUInt1()*256+ReadUInt1())*2; 

		// parse
		BOOL bLoop = (m_Samples[iInstrIdx].loopLength > 3);

		XMInstrument::WaveData& _wave = m_pSong->SampleData(iInstrIdx);

		_wave.Init();
		if ( smpLen[iInstrIdx] > 0 )
		{
			_wave.AllocWaveData(smpLen[iInstrIdx],false);
			_wave.WaveLength(smpLen[iInstrIdx]);
		}
		else _wave.WaveLength(0);

		if(bLoop)
		{
			_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
			_wave.WaveLoopStart(m_Samples[iInstrIdx].loopStart);
			if ( m_Samples[iInstrIdx].loopStart+m_Samples[iInstrIdx].loopLength > smpLen[iInstrIdx]) 
			{
					_wave.WaveLoopEnd(smpLen[iInstrIdx]);
			} else 	_wave.WaveLoopEnd(m_Samples[iInstrIdx].loopStart+m_Samples[iInstrIdx].loopLength);
		} else {
			_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
		}

		_wave.WaveVolume(m_Samples[iInstrIdx].volume * 2);
		char tmpfine = (char)m_Samples[iInstrIdx].finetune;
		if (tmpfine > 7 ) tmpfine -= 16;
		_wave.WaveFineTune(tmpfine*32);
		std::string sName = m_Samples[iInstrIdx].sampleName;
		_wave.WaveName(sName);

	}

	void MODSongLoader::LoadSampleData(XMSampler & sampler, int iInstrIdx) {
		// parse

		XMInstrument::WaveData& _wave =  m_pSong->SampleData(iInstrIdx);
		short wNew=0;

		// cache sample data
		unsigned char * smpbuf = new unsigned char[smpLen[iInstrIdx]];
		ReadArray(smpbuf,smpLen[iInstrIdx]);

		int sampleCnt = smpLen[iInstrIdx];

		// 8 bit mono sample
		for(int j = 0; j < sampleCnt; ++j) {
			//if ( smpbuf[j] < 128 ) wNew = (smpbuf[j]<<8);
			//else wNew = ((256-smpbuf[j])<<8);
			wNew = (smpbuf[j]<<8);
			*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew;
		}

		// cleanup
		delete[] smpbuf;

	}

	///\todo: Actually, these methods should always write in Little endian mode.
	bool XMSongLoader::ReadHeader(XMFILEHEADER& header) {
		Read(header.size);
		Read(header.norder);
		Read(header.restartpos);
		Read(header.channels);
		Read(header.patterns);
		Read(header.instruments);
		Read(header.flags);
		Read(header.speed);
		Read(header.tempo);
		return ReadArray(header.order,sizeof(header.order));
	}

	bool XMSongLoader::ReadHeader(XMPATTERNHEADER& header) {
		Read(header.size);
		Read(header.packingtype);
		Read(header.rows);
		return Read(header.packedsize);
	}

	bool XMSongLoader::ReadHeader(XMINSTRUMENTHEADER& header) {
		Read(header.size);
		ReadArray(header.name, sizeof(header.name));
		Read(header.type);
		return Read(header.samples);
	}

	bool XMSongLoader::ReadHeader(XMSAMPLEHEADER& header) {
		Read(header.shsize);
		ReadArray(header.snum, sizeof(header.snum));
		ReadArray(header.venv, 24);
		ReadArray(header.penv, 24);
		Read(header.vnum);
		Read(header.pnum);
		Read(header.vsustain);
		Read(header.vloops);
		Read(header.vloope);
		Read(header.psustain);
		Read(header.ploops);
		Read(header.ploope);
		Read(header.vtype);
		Read(header.ptype);
		Read(header.vibtype);
		Read(header.vibsweep);
		Read(header.vibdepth);
		Read(header.vibrate);
		Read(header.volfade);
		return ReadArray(header.reserved, 11);
	}

	bool XMSongLoader::ReadHeader(XMSAMPLESTRUCT& header) {
		Read(header.samplen);
		Read(header.loopstart);
		Read(header.looplen);
		Read(header.vol);
		Read(header.finetune);
		Read(header.type);
		Read(header.pan);
		Read(header.relnote);
		Read(header.res);
		return ReadArray(header.name, sizeof(header.name));
	}

	bool XMSongLoader::ReadHeader(XMSAMPLEFILEHEADER& header) {
		ReadArray(header.snum, sizeof(header.snum));
		ReadArray(header.venv, 24);
		ReadArray(header.penv, 24);
		Read(header.vnum);
		Read(header.pnum);
		Read(header.vsustain);
		Read(header.vloops);
		Read(header.vloope);
		Read(header.psustain);
		Read(header.ploops);
		Read(header.ploope);
		Read(header.vtype);
		Read(header.ptype);
		Read(header.vibtype);
		Read(header.vibsweep);
		Read(header.vibdepth);
		Read(header.vibrate);
		Read(header.volfade);
		ReadArray(header.reserved, 11);
		return Read(header.reserved2);

	}

	bool XMSongLoader::ReadHeader(XMINSTRUMENTFILEHEADER& header) {
		ReadArray(header.extxi, sizeof(header.extxi));
		ReadArray(header.name, sizeof(header.name));
		ReadArray(header.trkname, sizeof(header.trkname));
		return Read(header.shsize);
	}

	bool MODSongLoader::ReadHeader(MODHEADER& header) {
		Read(header.songlength);
		Read(header.unused);
		ReadArray(header.order, sizeof(header.order));
		return ReadArray(header.pID, sizeof(header.pID));
	}

	bool MODSongLoader::ReadHeader(MODSAMPLEHEADER& header) {
		ReadArray(header.sampleName, sizeof(header.sampleName));
		Read(header.sampleLength);
		Read(header.finetune);
		Read(header.volume);
		Read(header.loopStart);
		return Read(header.loopLength);
	}
}}

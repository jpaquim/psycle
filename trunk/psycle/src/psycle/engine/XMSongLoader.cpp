/** @file 
 *  @brief implementation file
 *  $Date$
 *  $Revision$
 */
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/resources/resources.hpp> // Is this needed?
#include <psycle/host/ProgressDialog.hpp> // Is this needed?
#include <psycle/engine/song.hpp>
#include <psycle/engine/machine.hpp> // It wouldn't be needed, since it is already included in "song.h"
#include <psycle/engine/XMInstrument.hpp>
#include <psycle/engine/XMSampler.hpp>
#include <psycle/engine/XMSongLoader.hpp>
#include <psycle/engine/player.hpp>



#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	///\todo this is disabled because of the annoying warning flood, but should prolly be fixed
	#pragma warning(disable:4482) // nonstandard extension used: enum 'xxxxx::yyyyy' used in qualified name
#endif



namespace psycle
{
	namespace host
	{

	const short MODSongLoader::BIGMODPERIODTABLE[37*8] = //((12note*3oct)+1note)*8fine
	{
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


	XMSongLoader::XMSongLoader(void)
	{
		for (int i=0; i<32; i++)
		{
			highOffset[i]=0;
			memPortaUp[i]=0;
			memPortaDown[i]=0;
			memPortaNote[i]=0;
			memPortaPos[i]=0;
		}
		for (int i=0; i<256; i++)
		{
			smpLen[i]=0;
			smpFlags[i]=0;
		}
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
		song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse");
		song.InsertConnection(Machine::id_type(0),MASTER_INDEX,0.35f);
		song.seqBus=0;
		// build sampler
		m_pSampler = (XMSampler *)(song._pMachine[0]);
		// get song name

		char * pSongName = AllocReadStr(20,17);

		if(pSongName==NULL)
			return;

		strcpy(song.Name,pSongName);
		strcpy(song.Author,"");
		strcpy(song.Comment,"Imported from FastTracker II Module: ");
		strcat(song.Comment,file_name().c_str());
		zapArray(pSongName);

		LONG iInstrStart = LoadPatterns(song);
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

	const long XMSongLoader::LoadPatterns(Song & song)
	{
		// get data
		Seek(60);
		Read(m_Header);

		m_pSampler->IsAmigaSlides((m_Header.flags & 0x01)?false:true);
		m_pSampler->XMSampler::PanningMode(XMSampler::PanningMode::TwoWay);
		//using std::max;
		#define max UNIVERSALIS__STANDARD_LIBRARY__LOOSE_MAX
		song.tracks(max(m_Header.channels,4));
		m_iInstrCnt = m_Header.instruments;
		song.BeatsPerMin(m_Header.tempo);
		song.LinesPerBeat(m_pSampler->Speed2LPB(m_Header.speed));

		for(int i = 0;i < MAX_SONG_POSITIONS && i < m_Header.norder;i++)
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
		int currentSample=0;
		for(int i = 1;i <= m_iInstrCnt;i++){
			iInstrStart = LoadInstrument(sampler,iInstrStart,i,currentSample);
			TRACE2("%d %s\n",i,sampler.rInstrument(i).Name().c_str());
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
		if(ReadChunk(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}





	// return address of next pattern, 0 for invalid
	const LONG XMSongLoader::LoadSinglePattern(Song & song, const LONG start,const int patIdx,const int iTracks)
	{

		int iHeaderLen = ReadInt4(start);
		Skip(1); //char iPackingType = ReadInt1();
		short iNumRows = ReadInt2();
		short iPackedSize = ReadInt2();

		song.AllocNewPattern(patIdx,"unnamed",iNumRows,false);

		PatternEntry e;

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
					e._inst = instr;	
					e._mach = 0;
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
					e._volume = 255;

					// volume/command
					if(vol >= 0x10 && vol <= 0x50)
					{
						e._volume=(vol == 0x50)?0x3F:(vol-0x10);

					} else if(vol >= 0x60){						
						switch(vol&0xF0)
						{
						case XMVOL_CMD::XMV_VOLUMESLIDEDOWN:
							e._volume = XMSampler::CMD_VOL::VOL_VOLSLIDEDOWN|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_VOLUMESLIDEUP:
							e._volume = XMSampler::CMD_VOL::VOL_VOLSLIDEUP|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_FINEVOLUMESLIDEDOWN:
							e._volume = XMSampler::CMD_VOL::VOL_FINEVOLSLIDEDOWN|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_FINEVOLUMESLIDEUP:
							e._volume = XMSampler::CMD_VOL::VOL_FINEVOLSLIDEUP|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_PANNING:
							e._volume = XMSampler::CMD_VOL::VOL_PANNING|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_PANNINGSLIDELEFT:
							// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
							e._volume = XMSampler::CMD_VOL::VOL_PANSLIDELEFT|((vol&0x0F)>>2);
							break;
						case XMVOL_CMD::XMV_PANNINGSLIDERIGHT:
							// Panning in FT2 has 256 values, so we convert to the 64values used in Sampulse.
							e._volume = XMSampler::CMD_VOL::VOL_PANSLIDERIGHT|((vol&0x0F)>>2);
							break;
// Ignoring this command for now.
//						case XMVOL_CMD::XMV_VIBRATOSPEED:
//							e._volume = XMSampler::CMD_VOL::VOL_VIBRATO_SPEED|(vol&0x0F);
//							break;
						case XMVOL_CMD::XMV_VIBRATO:
							e._volume = XMSampler::CMD_VOL::VOL_VIBRATO|(vol&0x0F);
							break;
						case XMVOL_CMD::XMV_PORTA2NOTE:
							e._volume = XMSampler::CMD_VOL::VOL_TONEPORTAMENTO|(vol&0x0F);
							break;
						default:
							break;
						}
					}
					e._parameter = param;
					int exchwave[3]={XMInstrument::WaveData::WaveForms::SINUS,
						XMInstrument::WaveData::WaveForms::SAWDOWN,
						XMInstrument::WaveData::WaveForms::SQUARE
					};
					switch(type){
	#else
					e._parameter = param;
					int exchwave[3]={XMInstrument::WaveData::WaveForms::SINUS,
						XMInstrument::WaveData::WaveForms::SAWDOWN,
						XMInstrument::WaveData::WaveForms::SQUARE
					};
					// volume/command
					if(vol >= 0x10 && vol <= 0x50)
					{
						e._cmd = 0x0C;
						e._parameter = (vol-0x10)*2;
					}
					else switch(type){
	#endif
#endif
						case XMCMD::ARPEGGIO:
							if(param != 0){
								e._cmd = XMSampler::CMD::ARPEGGIO;
							} else {
								e._cmd = XMSampler::CMD::NONE;
							}
							break;
						case XMCMD::PORTAUP:
							e._cmd = XMSampler::CMD::PORTAMENTO_UP;
							if ( e._parameter == 0) {
								if ( memPortaPos[col] != 1 )
								{
									e._parameter = memPortaUp[col];
								}
							}
							else {
								if ( e._parameter > 0xDF ) { e._parameter = 0xDF; }
								memPortaUp[col] = e._parameter;
							}
							memPortaPos[col] = 1;
							break;
						case XMCMD::PORTADOWN:
							e._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
							if ( e._parameter == 0) {
								if ( memPortaPos[col] != 2 )
								{
									e._parameter = memPortaDown[col];
								}
							}
							else {
								if ( e._parameter > 0xDF ) { e._parameter = 0xDF; }
								memPortaDown[col] = e._parameter;
							}
							memPortaPos[col] = 2;
							break;
						case XMCMD::PORTA2NOTE:
							e._cmd = XMSampler::CMD::PORTA2NOTE;
							if ( e._parameter == 0) {
								if ( memPortaPos[col] != 3 )
								{
									e._parameter = memPortaNote[col];
								}
							}
							else {
								memPortaNote[col] = e._parameter;
							}
							memPortaPos[col] = 3;
							break;
						case XMCMD::VIBRATO:
							e._cmd = XMSampler::CMD::VIBRATO;
							break;
						case XMCMD::TONEPORTAVOL:
							e._cmd = XMSampler::CMD::TONEPORTAVOL;
							break;
						case XMCMD::VIBRATOVOL:
							e._cmd = XMSampler::CMD::VIBRATOVOL;
							break;
						case XMCMD::TREMOLO:
							e._cmd = XMSampler::CMD::TREMOLO;
							break;
						case XMCMD::PANNING:
							e._cmd = XMSampler::CMD::PANNING;
							break;
						case XMCMD::OFFSET:
							e._cmd = XMSampler::CMD::OFFSET | highOffset[col]; 
							break;
						case XMCMD::VOLUMESLIDE:
							e._cmd = XMSampler::CMD::VOLUMESLIDE;
							break;
						case XMCMD::POSITION_JUMP:
							e._cmd = PatternCmd::JUMP_TO_ORDER;
							break;
						case XMCMD::VOLUME:
							e._cmd = XMSampler::CMD::VOLUME;
							e._parameter = param * 2;
							break;
						case XMCMD::PATTERN_BREAK:
							e._cmd = PatternCmd::BREAK_TO_LINE;
							e._parameter = ((param&0xF0)>>4)*10 + (param&0x0F);
							break;
						case XMCMD::EXTENDED:
							switch(param & 0xf0){
							case XMCMD_E::E_FINE_PORTA_UP:
								e._cmd = XMSampler::CMD::PORTAMENTO_UP;
								e._parameter= 0xF0+(param&0x0F);
								break;
							case XMCMD_E::E_FINE_PORTA_DOWN:
								e._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
								e._parameter= 0xF0+(param&0x0F);
								break;
							case XMCMD_E::E_GLISSANDO_STATUS:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter = XMSampler::CMD_E::E_GLISSANDO_TYPE | ((param==0)?0:1);
								break;
							case XMCMD_E::E_VIBRATO_WAVE:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter =XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[param & 0x3];
								break;
							case XMCMD_E::E_FINETUNE:
								e._cmd = XMSampler::CMD::NONE;
								e._parameter = 0;
								break;
							case XMCMD_E::E_PATTERN_LOOP:
								e._cmd = PatternCmd::PATTERN_LOOP;
								e._parameter = param & 0xf;
								break;
							case XMCMD_E::E_TREMOLO_WAVE:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter = XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3];
								break;
							case XMCMD_E::E_MOD_RETRIG:
								e._cmd = XMSampler::CMD::RETRIG;
								e._parameter = param & 0xf;
								break;
							case XMCMD_E::E_FINE_VOLUME_UP:
								e._cmd = XMSampler::CMD::VOLUMESLIDE;
								e._parameter = 0xf0 + (param & 0xf);
								break;
							case XMCMD_E::E_FINE_VOLUME_DOWN:
								e._cmd = XMSampler::CMD::VOLUMESLIDE;
								e._parameter = 0x0f + ((param & 0xf)<<4);
								break;
							case XMCMD_E::E_DELAYED_NOTECUT:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter = XMSampler::CMD_E::E_DELAYED_NOTECUT | (param & 0xf);
								break;
							case XMCMD_E::E_NOTE_DELAY:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter = XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf);
								break;
							case XMCMD_E::E_PATTERN_DELAY:
								e._cmd = PatternCmd::EXTENDED;
								e._parameter =  PatternCmd::PATTERN_DELAY | (param & 0xf);
								break;
							case XMCMD_E::E_SET_MIDI_MACRO:
								e._cmd = XMSampler::CMD::EXTENDED;
								e._parameter = XMCMD::MIDI_MACRO | (param & 0x0f);
								break;
							default:
								e._cmd = XMSampler::CMD::NONE;
								break;
							}
							break;
						case XMCMD::SETSPEED:
							if ( param < 32)
							{
								e._cmd=PatternCmd::EXTENDED;
								e._parameter = 24 / ((param == 0)?6:param);
							}
							else
							{
								e._cmd = PatternCmd::SET_TEMPO;
							}
							break;
						case XMCMD::SET_GLOBAL_VOLUME:
							e._cmd = XMSampler::CMD::SET_GLOBAL_VOLUME;
							e._parameter = param*2;
							break;
						case XMCMD::GLOBAL_VOLUME_SLIDE:
							e._cmd = XMSampler::CMD::GLOBAL_VOLUME_SLIDE;
							//Double the parameter, since FT2's range is 0-0x40.
							if ( (param & 0x0F) == 0 || (param & 0x0F) == 0xF){ // Slide up
								e._parameter = (param & 0xF0)>>4;
								e._parameter = e._parameter>7?15:e._parameter*2;
								e._parameter<<=4;
								e._parameter |= (param & 0x0F);
							}
							else if ( (param & 0xF0) == 0 || (param & 0xF0) == 0xF0)  { // Slide down
								e._parameter = (param & 0x0F);
								e._parameter = e._parameter>7?15:e._parameter*2;
								e._parameter |= (param & 0xF0);
							}
							break;
						case XMCMD::NOTE_OFF:
							e._cmd = XMSampler::CMD::VOLUME;
							e._parameter = 0;
							break;
						case XMCMD::SET_ENV_POSITION:
							e._cmd = XMSampler::CMD::SET_ENV_POSITION;
							break;
						case XMCMD::PANNINGSLIDE:
							e._cmd = XMSampler::CMD::PANNINGSLIDE;
							break;
						case XMCMD::RETRIG:
							e._cmd = XMSampler::CMD::RETRIG;
							break;
						case XMCMD::TREMOR:
							e._cmd =  XMSampler::CMD::TREMOR;
							break;
						case XMCMD::EXTEND_XM_EFFECTS:
							switch(param & 0xf0){
							case XMCMD_X::X_EXTRA_FINE_PORTA_DOWN:
								e._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
								e._parameter = 0xE0 | (param & +0xf);
								break;
							case XMCMD_X::X_EXTRA_FINE_PORTA_UP:
								e._cmd = XMSampler::CMD::PORTAMENTO_UP;
								e._parameter = 0xE0 | (param & +0xf);
								break;
							case XMCMD_X::X9:
								switch ( param & 0xf){
								case XMCMD_X9::X9_SURROUND_OFF:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_SURROUND_OFF;
									break;
								case XMCMD_X9::X9_SURROUND_ON:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_SURROUND_ON;
									break;
								case XMCMD_X9::X9_REVERB_OFF:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_REVERB_OFF;
									break;
								case XMCMD_X9::X9_REVERB_FORCE:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_REVERB_FORCE;
									break;
								case XMCMD_X9::X9_STANDARD_SURROUND:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_STANDARD_SURROUND;
									break;
								case XMCMD_X9::X9_QUAD_SURROUND:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_QUAD_SURROUND;
									break;
								case XMCMD_X9::X9_GLOBAL_FILTER:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_GLOBAL_FILTER;
									break;
								case XMCMD_X9::X9_LOCAL_FILTER:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_LOCAL_FILTER;
									break;
								case XMCMD_X9::X9_PLAY_FORWARD:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_PLAY_FORWARD;
									break;
								case XMCMD_X9::X9_PLAY_BACKWARD:
									e._cmd = XMSampler::CMD::EXTENDED;
									e._parameter = XMSampler::CMD_E::E9 | XMSampler::CMD_E9::E9_PLAY_BACKWARD;
									break;
								default:
									e._cmd = XMSampler::CMD::NONE;
									break;
								}
								break;
							case XMCMD_X::X_HIGH_OFFSET:
								highOffset[col] = param & 0x0F;
								break;
							default:
								e._cmd = XMSampler::CMD::NONE;
								break;
							}
							break;
						case XMCMD::PANBRELLO:
							e._cmd = XMSampler::CMD::PANBRELLO;
							break;
						case XMCMD::MIDI_MACRO:
							e._cmd = XMSampler::CMD::MIDI_MACRO;
							break;
						default:
							e._cmd = XMSampler::CMD::NONE;
							break;
					}
					// instrument/note
					note = note & 0x7f;
					switch(note)
					{
						case 0x00: 
							e._note = notecommands::empty;
							break;// no note

						case 0x61:
							e._note = notecommands::release;
							e._inst = 255;
							e._mach = 0;
							break;// noteoff		
						
						default: 
							if(note >= 96 || note < 0)
								TRACE(_T("invalid note\n"));
							e._note  = note+11; // +11 -> +12 ( FT2 C-0 is Psycle's C-1) -1 ( Ft2 C-0 is value 1)

							break;	// transpose
					}

#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
					if ((e._note == notecommands::empty) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255) && (e._volume == 255))
	#else
					if ((e._note == notecommands::empty) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255))
	#endif
#endif
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

	const LONG XMSongLoader::LoadInstrument(XMSampler & sampler, LONG iStart, const int idx,int &curSample)
	{
		// read header
		Seek(iStart);

		int iInstrSize = ReadInt4();
//		assert(iInstrSize==0x107||iInstrSize==0x21); // Skale Tracker (or MadTracker or who knows which more) don't have the "reserved[20]" parameter in the XMSAMPLEHEADER
		TCHAR sInstrName[23];
		ZeroMemory(sInstrName,sizeof(sInstrName) * sizeof(TCHAR));
		ReadChunk(sInstrName,22); sInstrName[22]= 0;

		Skip(1); //int iInstrType = ReadInt1();
		int iSampleCount = ReadInt2();

		if(iSampleCount>1)
 			TRACE(_T("ssmple count = %d\n"),iSampleCount);

		sampler.rInstrument(idx).Name(sInstrName);
		iStart += iInstrSize;

		if(iSampleCount==0)
			return iStart;

        
		XMSAMPLEHEADER _samph;
		ZeroMemory(&_samph,sizeof(XMSAMPLEHEADER));
		Read(_samph);

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
		Skip(1); //char iReserved = ReadInt1();	

		// sample name
		char * cName = AllocReadStr(22);

		// parse
		BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
		BOOL bPingPong = iFlags & 0x02;
		BOOL b16Bit = iFlags & 0x10;
	
		// alloc wave memory

		assert(iLen < (1 << 30)); // Since in some places, signed values are used, we cannot use the whole range.

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

	const LONG XMSongLoader::LoadSampleData(XMSampler & sampler, LONG iStart,const int iInstrIdx,const int iSampleIdx)
	{
		// parse
		
		BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
		XMInstrument::WaveData& _wave =  sampler.SampleData(iSampleIdx);
		short wNew=0;

		// cache sample data
		Seek(iStart);
		char * smpbuf = new char[smpLen[iSampleIdx]];
		memset(smpbuf,0,smpLen[iSampleIdx]);
		ReadChunk(smpbuf,smpLen[iSampleIdx]);

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

	
	void XMSongLoader::SetEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader)
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

	};		








	
	MODSongLoader::MODSongLoader(void)
	{
		for (int i=0; i<32; i++)
		{
			smpLen[i]=0;
		}
	}

	MODSongLoader::~MODSongLoader(void)
	{

	}

	void MODSongLoader::Load(Song& song,const bool fullopen)
	{

		// check validity
		if(!IsValid()){
			return;
		}
		song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse");
//		song.InsertConnection(0,MASTER_INDEX,0.75f); // This is done later, when determining the number of channels.
		song.seqBus=0;
		// build sampler
		m_pSampler = (XMSampler *)(song._pMachine[0]);
		// get song name

		char * pSongName = AllocReadStr(20,0);
		if(pSongName==NULL)
			return;

		strcpy(song.Name,pSongName);
		strcpy(song.Author,"");
		strcpy(song.Comment,"Imported from MOD Module: ");
		strcat(song.Comment,file_name().c_str());
		zapArray(pSongName);

		// get data
		Seek(20);
		for (int i=0;i<31;i++) LoadSampleHeader(*m_pSampler,i);
		Seek(950);
		Read(m_Header);
		
		char pID[5];
		pID[0]=m_Header.pID[0];pID[1]=m_Header.pID[1];pID[2]=m_Header.pID[2];pID[3]=m_Header.pID[3];pID[4]=0;
		
		
		m_pSampler->IsAmigaSlides(true);
		if ( !stricmp(pID,"M.K.")) { song.tracks(4); song.InsertConnection(Machine::id_type(0),MASTER_INDEX,0.75f); }
		else if ( !stricmp(pID,"M!K!")) { song.tracks(4); song.InsertConnection(Machine::id_type(0),MASTER_INDEX,0.75f); }
		else if ( !stricmp(pID+1,"CHN")) { char tmp[2]; tmp[0] = pID[0]; tmp[1]=0; song.tracks(atoi(tmp));  song.InsertConnection(Machine::id_type(0),MASTER_INDEX,0.5f); }
		else if ( !stricmp(pID+2,"CH")) { char tmp[3]; tmp[0] = pID[0]; tmp[1]=pID[1]; tmp[2]=0; song.tracks(atoi(tmp)); song.InsertConnection(Machine::id_type(0),MASTER_INDEX,0.35f);}
		song.BeatsPerMin(125);
		song.LinesPerBeat(4);

		for (int i = 0; i< song.tracks(); i++ )
		{
			if (i%4 == 0 || i%4 == 3) m_pSampler->rChannel(i).DefaultPanFactor(12);
			else m_pSampler->rChannel(i).DefaultPanFactor(48);
		}


		LoadPatterns(song);
		for(int i = 0;i < 31;i++){
			LoadInstrument(*m_pSampler,i);
		}
	}

	const bool MODSongLoader::IsValid()
	{
		bool bIsValid = false;
		char *pID = AllocReadStr(4,1080);

		bIsValid = !stricmp(pID,"M.K.");
		if ( !bIsValid ) bIsValid = !stricmp(pID,"M!K!");
		if ( !bIsValid ) bIsValid = !stricmp(pID+1,"CHN");
		if ( !bIsValid ) bIsValid = !stricmp(pID+2,"CH");

		delete[] pID;
		return bIsValid;
	}

	const void MODSongLoader::LoadPatterns(Song & song)
	{
		int npatterns=0;
		for(int i = 0;i < MAX_SONG_POSITIONS && i < m_Header.songlength;i++)
		{
			if ( m_Header.order[i] < MAX_PATTERNS ){
				song.playOrder[i]=m_Header.order[i];
				if ( m_Header.order[i] > npatterns) npatterns=m_Header.order[i];
			} else { 
				song.playOrder[i]=0;
				if ( m_Header.order[i] > npatterns) npatterns=m_Header.order[i];
			}
		}
		npatterns++;
		if ( m_Header.songlength > MAX_SONG_POSITIONS ){
			song.playLength=MAX_SONG_POSITIONS;
		} else {
			song.playLength=m_Header.songlength;
		}

		// get pattern data
		Seek(1084);
		for(int j = 0;j < npatterns ;j++){
			LoadSinglePattern(song,j,song.tracks());
		}
	}

	char * MODSongLoader::AllocReadStr(const LONG size, const LONG start)
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
		if(ReadChunk(pData,size))
			return pData;

		delete[] pData;
		return NULL;
	}





	// return address of next pattern, 0 for invalid
	const void MODSongLoader::LoadSinglePattern(Song & song,const int patIdx,const int iTracks)
	{

		short iNumRows = 64;

		song.AllocNewPattern(patIdx,"unnamed",iNumRows,false);

		PatternEntry e;
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
					if ( instr != 0 ) e._inst = instr-1;
					else e._inst = 255;
					e._mach = 0;
					e._parameter = param;

					int exchwave[3]={XMInstrument::WaveData::WaveForms::SINUS,
						XMInstrument::WaveData::WaveForms::SAWDOWN,
						XMInstrument::WaveData::WaveForms::SQUARE
					};

					switch(type){
						case XMCMD::ARPEGGIO:
							if(param != 0){
								e._cmd = XMSampler::CMD::ARPEGGIO;
							} else {
								e._cmd = XMSampler::CMD::NONE;
							}
							break;
						case XMCMD::PORTAUP:
							e._cmd = XMSampler::CMD::PORTAMENTO_UP;
							break;
						case XMCMD::PORTADOWN:
							e._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
							break;
						case XMCMD::PORTA2NOTE:
							e._cmd = XMSampler::CMD::PORTA2NOTE;
							break;
						case XMCMD::VIBRATO:
							e._cmd = XMSampler::CMD::VIBRATO;
							break;
						case XMCMD::TONEPORTAVOL:
							e._cmd = XMSampler::CMD::TONEPORTAVOL;
							break;
						case XMCMD::VIBRATOVOL:
							e._cmd = XMSampler::CMD::VIBRATOVOL;
							break;
						case XMCMD::TREMOLO:
							e._cmd = XMSampler::CMD::TREMOLO;
							break;
						case XMCMD::PANNING:
							e._cmd = XMSampler::CMD::PANNING;
							break;
						case XMCMD::OFFSET:
							e._cmd = XMSampler::CMD::OFFSET;
							break;
						case XMCMD::VOLUMESLIDE:
							e._cmd = XMSampler::CMD::VOLUMESLIDE;
							break;
						case XMCMD::POSITION_JUMP:
							e._cmd = PatternCmd::JUMP_TO_ORDER;
							break;
						case XMCMD::VOLUME:
							e._cmd = XMSampler::CMD::VOLUME;
							e._parameter = param<=0x40?param*2:0x80;
							break;
						case XMCMD::PATTERN_BREAK:
							e._cmd = PatternCmd::BREAK_TO_LINE;
							e._parameter = ((param&0xF0)>>4)*10 + (param&0x0F);
							break;
						case XMCMD::EXTENDED:
							switch(param & 0xf0){
						case XMCMD_E::E_FINE_PORTA_UP:
							e._cmd = XMSampler::CMD::PORTAMENTO_UP;
							e._parameter= 0xF0+(param&0x0F);
							break;
						case XMCMD_E::E_FINE_PORTA_DOWN:
							e._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
							e._parameter= 0xF0+(param&0x0F);
							break;
						case XMCMD_E::E_GLISSANDO_STATUS:
							e._cmd = XMSampler::CMD::EXTENDED;
							e._parameter = XMSampler::CMD_E::E_GLISSANDO_TYPE | ((param==0)?0:1);
							break;
						case XMCMD_E::E_VIBRATO_WAVE:
							e._cmd = XMSampler::CMD::EXTENDED;
							e._parameter =XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[param & 0x3];
							break;
						case XMCMD_E::E_FINETUNE:
							e._cmd = XMSampler::CMD::NONE;
							e._parameter = 0;
							break;
						case XMCMD_E::E_PATTERN_LOOP:
							e._cmd = PatternCmd::EXTENDED;
							e._parameter = PatternCmd::PATTERN_LOOP | (param & 0xf);
							break;
						case XMCMD_E::E_TREMOLO_WAVE:
							e._cmd = XMSampler::CMD::EXTENDED;
							e._parameter = XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[param & 0x3];
							break;
						case XMCMD_E::E_MOD_RETRIG:
							e._cmd = XMSampler::CMD::RETRIG;
							e._parameter = param & 0xf;
							break;
						case XMCMD_E::E_FINE_VOLUME_UP:
							e._cmd = XMSampler::CMD::VOLUMESLIDE;
							e._parameter = 0xf0 + (param & 0xf);
							break;
						case XMCMD_E::E_FINE_VOLUME_DOWN:
							e._cmd = XMSampler::CMD::VOLUMESLIDE;
							e._parameter = 0x0f + ((param & 0xf)<<4);
							break;
						case XMCMD_E::E_DELAYED_NOTECUT:
							e._cmd = XMSampler::CMD::EXTENDED;
							e._parameter = XMSampler::CMD_E::E_DELAYED_NOTECUT | (param & 0xf);
							break;
						case XMCMD_E::E_NOTE_DELAY:
							e._cmd = XMSampler::CMD::EXTENDED;
							e._parameter = XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf);
							break;
						case XMCMD_E::E_PATTERN_DELAY:
							e._cmd = PatternCmd::EXTENDED;
							e._parameter =  PatternCmd::PATTERN_DELAY | (param & 0xf);
							break;
						default:
							e._cmd = XMSampler::CMD::NONE;
							e._parameter = 0;
							break;
							}
							break;
						case XMCMD::SETSPEED:
							if ( param < 32)
							{
								e._cmd=PatternCmd::EXTENDED;
								e._parameter = 24 / ((param == 0)?6:param);
							}
							else
							{
								e._cmd = PatternCmd::SET_TEMPO;
							}
							break;
						default:
							e._cmd = XMSampler::CMD::NONE;
							break;
					}
					// instrument/note
					if ( note != 255 ) e._note  = note+12;
					else e._note  = note;

					if ((e._note == notecommands::empty) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255))
					{
						e._mach = 255;
					}
					WritePatternEntry(song,patIdx,row,col,e);	
				}
			}
	}
	const unsigned char MODSongLoader::ConvertPeriodtoNote(const unsigned short period)
	{
		for (int count2=1;count2<37; count2++)
		{
			if (period > BIGMODPERIODTABLE[count2*8]-2 && period < BIGMODPERIODTABLE[count2*8]+2 )
				return count2-1+36; // three octaves above.
		}
		return 255;
	}

	const BOOL MODSongLoader::WritePatternEntry(Song & song,
		const int patIdx, const int row, const int col,PatternEntry &e)
	{
		// don't overflow song buffer 
		if(patIdx>=MAX_PATTERNS) return false;

		PatternEntry* pData = (PatternEntry*) song._ptrackline(patIdx,col,row);

		*pData = e;

		return true;
	}	

	const void MODSongLoader::LoadInstrument(XMSampler & sampler, const int idx)
	{
		sampler.rInstrument(idx).Init();
		sampler.rInstrument(idx).Name(m_Samples[idx].sampleName);

		if (m_Samples[idx].sampleLength > 0 ) 
		{
			sampler.rInstrument(idx).IsEnabled(true);
			LoadSampleData(sampler,idx);
		}

		int i;
		XMInstrument::NotePair npair;
		npair.second=idx;
		for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
			npair.first=i;
			sampler.rInstrument(idx).NoteToSample(i,npair);
		}
	}

	const void MODSongLoader::LoadSampleHeader(XMSampler & sampler, const int iInstrIdx)
	{
		ReadChunk(m_Samples[iInstrIdx].sampleName,22);	m_Samples[iInstrIdx].sampleName[21]='\0';

		smpLen[iInstrIdx] = (ReadUInt1()*0x100+ReadUInt1())*2; 
		m_Samples[iInstrIdx].sampleLength = smpLen[iInstrIdx];
		m_Samples[iInstrIdx].finetune = ReadUInt1();
		m_Samples[iInstrIdx].volume = ReadUInt1();
		m_Samples[iInstrIdx].loopStart =(ReadUInt1()*256+ReadUInt1())*2; 
		m_Samples[iInstrIdx].loopLength = (ReadUInt1()*256+ReadUInt1())*2; 

		// parse
		BOOL bLoop = (m_Samples[iInstrIdx].loopLength > 3);

		XMInstrument::WaveData& _wave = sampler.SampleData(iInstrIdx);

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

	const void MODSongLoader::LoadSampleData(XMSampler & sampler, const int iInstrIdx)
	{
		// parse

		XMInstrument::WaveData& _wave =  sampler.SampleData(iInstrIdx);
		short wNew=0;

		// cache sample data
		unsigned char * smpbuf = new unsigned char[smpLen[iInstrIdx]];
		ReadChunk(smpbuf,smpLen[iInstrIdx]);

		int sampleCnt = smpLen[iInstrIdx];

		// 8 bit mono sample
		for(int j=0;j<sampleCnt;j++)
		{
//			if ( smpbuf[j] < 128 ) wNew = (smpbuf[j]<<8);
//			else wNew = ((256-smpbuf[j])<<8);
			wNew = (smpbuf[j]<<8);
			*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew;
		}

		// cleanup
		delete[] smpbuf;

	}


}
}



#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

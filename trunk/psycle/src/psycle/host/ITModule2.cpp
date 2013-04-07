#include <psycle/host/detail/project.private.hpp>
#include "ITModule2.h"
#include "Configuration.hpp"

#include "Player.hpp"
#include "Song.hpp"
#include "XMSampler.hpp"

#include <algorithm>
namespace psycle
{
	namespace host
	{
		ITModule2::ITModule2(void)
		{
			embeddedData = NULL;
			for (int i=0;i<64;i++) highOffset[i]=0;
		}

		ITModule2::~ITModule2(void)
		{
			delete embeddedData;
		}
		bool ITModule2::BitsBlock::ReadBlock(OldPsyFile *pFile)
		{
			// block layout : uint16 size, <size> bytes data
			std::uint16_t size;
			pFile->Read(&size,2);
			pdata = new unsigned char[size];
			if (!pdata) return false;
			if (!pFile->Read(pdata,size))
			{
				delete[] pdata;
				return false;
			}
			rpos=pdata;
			rend=pdata+size;
			rembits=8;
			return true;
		}

		std::uint32_t ITModule2::BitsBlock::ReadBits(unsigned char bitwidth)
		{
			std::uint32_t val = 0;
			int b = 0;

			// If reached the end of the buffer, exit.
			if (rpos >= rend) return val;

			// while we have more bits to read than the remaining bits in this byte
			while (bitwidth > rembits) {
				// add to val, the data-bits from rpos, shifting the necessary number of bits.
				val |= *rpos++ << b;
				//if reached the end, exit.
				if (rpos >= rend) return val;
				// increment the shift
				b += rembits;
				// decrease the remaining bits to read
				bitwidth -= rembits;
				// set back the number of bits.
				rembits = 8;
			}
			// Filter the bottom-most bitwidth bytes from rpos, and shift them by b to add to the final value.
			val |= (*rpos & ((1 << bitwidth) - 1)) << b;
			// shift down the remaining bits so that they are read the next time.
			*rpos >>= bitwidth;
			// reduce the remaining bits.
			rembits -= bitwidth;

			return val;
		}
		void ITModule2::LoadInstrumentFromFile(Song& song, const int idx)
		{
			itFileH.flags=0;
			itInsHeader1x inshead;
			Read(&inshead,sizeof(itInsHeader1x));
			XMInstrument instr;
			instr.Init();
			if (inshead.trackerV < 0x200 ) {
				LoadOldITInst(inshead,instr,idx);
			}
			else {
				LoadITInst(*reinterpret_cast<itInsHeader2x*>(&inshead),instr,idx);
				//From ITTECH: Total length of an instrument is 547 bytes, but 554 bytes are
				//written, just to simplify the loading of the old format.
			}
			if (inshead.noS < 1 && !Eof()) {
				inshead.noS = 0;
				//The header sample count might be wrong. Let's recalculate it.
				std::size_t curpos = GetPos();
				while(Expect(&IMPS_ID,sizeof(IMPS_ID))) {
					inshead.noS++;
					Skip(sizeof(itSampleHeader)-sizeof(IMPS_ID));
				}
				Seek(curpos);
			}
			int curSample(0);
			std::vector<unsigned char> sRemap;
			for (unsigned int i(0); i<inshead.noS; i++)
			{
				while (song.samples.IsEnabled(curSample) && curSample < MAX_INSTRUMENTS-1) curSample++;
				XMInstrument::WaveData wave;
				itSampleHeader curH;
				Read(&curH,sizeof(curH));
				std::size_t curpos = GetPos();
				LoadITSample(curH,wave,curSample);
				Seek(curpos);
				// Only get REAL samples.
				if ( wave.WaveLength() > 0 && curSample < MAX_INSTRUMENTS-2 ) {
					song.samples.SetSample(wave, curSample);
					sRemap.push_back(curSample);
				}
				else { sRemap.push_back(MAX_INSTRUMENTS-1); }
			}

			for(int i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++)
			{
				XMInstrument::NotePair npair = instr.NoteToSample(i);
				npair.second=sRemap[(npair.second<inshead.noS)?npair.second:0];
				instr.NoteToSample(i,npair);
			}
			song.xminstruments.SetInst(instr,idx);
		}
		bool ITModule2::LoadITModule(Song& song)
		{
			CExclusiveLock lock(&song.semaphore, 2, true);
			_pSong=&song;
			if (Read(&itFileH,sizeof(itFileH))==0 ) return false;
			if (itFileH.tag != IMPM_ID ) return false;

			song.name = itFileH.songName;
			song.author = "";
			song.comments = "Imported from Impulse Tracker Module: ";
			song.comments.append(szName);

			song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse",0);
			song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,(itFileH.mVol>128?128:itFileH.mVol)/128.0f);
			song.seqBus=0;
			XMSampler* sampler = ((XMSampler*)song._pMachine[0]);

			song.BeatsPerMin(itFileH.iTempo);
			int extraticks=0;
			song.LinesPerBeat(XMSampler::CalcLPBFromSpeed(itFileH.iSpeed,extraticks));
			if (extraticks != 0) {
				//\todo: setup something...
			}

			sampler->IsAmigaSlides(itFileH.flags&Flags::LINEARSLIDES?false:true);
			sampler->GlobalVolume(itFileH.gVol);
/*
Flags:    Bit 0: On = Stereo, Off = Mono
			Bit 1: Vol0MixOptimizations - If on, no mixing occurs if
			the volume at mixing time is 0 (redundant v1.04+)
			Bit 2: On = Use instruments, Off = Use samples.
			Bit 3: On = Linear slides, Off = Amiga slides.
			Bit 4: On = Old Effects, Off = IT Effects
Differences:
		- Vibrato is updated EVERY frame in IT mode, whereas
			it is updated every non-row frame in other formats.
			Also, it is two times deeper with Old Effects ON
			- Command Oxx will set the sample offset to the END
			of a sample instead of ignoring the command under
			old effects mode.
			- (More to come, probably)
			Bit 5: On = Link Effect G's memory with Effect E/F. Also
			Gxx with an instrument present will cause the
			envelopes to be retriggered. If you change a
			sample on a row with Gxx, it'll adjust the
			frequency of the current note according to:

		  NewFrequency = OldFrequency * NewC5 / OldC5;
		  Bit 6: Use MIDI pitch controller, Pitch depth given by PWD
			  Bit 7: Request embedded MIDI configuration
			  (Coded this way to permit cross-version saving)

Special:  Bit 0: On = song message attached.
		  Song message:
		  Stored at offset given by "Message Offset" field.
			  Length = MsgLgth.
			  NewLine = 0Dh (13 dec)
			  EndOfMsg = 0

		Note: v1.04+ of IT may have song messages of up to
			8000 bytes included.
			Bit 1: Reserved
			Bit 2: Reserved
			Bit 3: MIDI configuration embedded
			Bit 4-15: Reserved

*/
			bool stereo=itFileH.flags&Flags::STEREO;
			int i,j;
			for (i=0;i<64;i++)
			{
				if (stereo && !(itFileH.chanPan[i]&ChanFlags::IS_DISABLED))
				{
					if (itFileH.chanPan[i]==ChanFlags::IS_SURROUND )
					{
						sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
						sampler->rChannel(i).DefaultIsSurround(true);
					}
					else {
						sampler->rChannel(i).DefaultPanFactorFloat((itFileH.chanPan[i]&0x7F)/64.0f,true);
					}
				}
				else{
					sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
				}
				sampler->rChannel(i).DefaultVolumeFloat(itFileH.chanVol[i]/64.0f,true);
				if ( (itFileH.chanPan[i]&ChanFlags::IS_DISABLED) ) 
				{
					sampler->rChannel(i).DefaultIsMute(true);
				}
				else {
					m_maxextracolumn=i;
				}
				sampler->rChannel(i).DefaultFilterType(dsp::F_ITLOWPASS);
			}
			if(m_maxextracolumn==63) { m_maxextracolumn=15; }

			i=0;
			for (j=0;j<itFileH.ordNum && i<MAX_SONG_POSITIONS;j++)
			{
				song.playOrder[i]=ReadUInt8(); // 254 = ++ (skip), 255 = --- (end of tune).
				if (song.playOrder[i]!= 254 &&song.playOrder[i] != 255 ) i++;
			}
			Skip(itFileH.ordNum-j);
/*
			while (j<itFileH.ordNum)
			{
				char tmp=ReadInt(1);
				j++;
			}
*/
			song.playLength=i;
			if ( song.playLength == 0) // Add at least one pattern to the sequence.
			{
				song.playLength = 1;
				song.playOrder[0]=0;
			}

			std::uint32_t *pointersi = new std::uint32_t[itFileH.insNum];
			Read(pointersi,itFileH.insNum*sizeof(std::uint32_t));
			std::uint32_t * pointerss = new std::uint32_t[itFileH.sampNum];
			Read(pointerss,itFileH.sampNum*sizeof(std::uint32_t));
			std::uint32_t * pointersp = new std::uint32_t[itFileH.patNum];
			Read(pointersp,itFileH.patNum*sizeof(std::uint32_t));

			if ( itFileH.special&SpecialFlags::MIDIEMBEDDED)
			{
				embeddedData = new EmbeddedMIDIData;
				short skipnum;
				Read(&skipnum,sizeof(short));
				Skip(skipnum*8); // This is some strange data. It is not documented.

				Read(embeddedData,sizeof(EmbeddedMIDIData));

				for ( int i=0; i<128; i++ )
				{
					std::string zxx = embeddedData->Zxx[i];
					std::string zxx2 = zxx.substr(0,5);
					if ( zxx2 == "F0F00")
					{
						int mode=0;
						if (embeddedData->Zxx[i][5] >= '0' && embeddedData->Zxx[i][5] <= '9')
							mode = embeddedData->Zxx[i][5] - '0';
						else if (embeddedData->Zxx[i][5] >= 'A' && embeddedData->Zxx[i][5] <= 'F')
							mode = embeddedData->Zxx[i][5] - 'A' + 0xA;

						int tmp=0;
						if (embeddedData->Zxx[i][6] >= '0' && embeddedData->Zxx[i][6] <= '9')
							tmp = (embeddedData->Zxx[i][6] - '0') * 0x10;
						else if (embeddedData->Zxx[i][6] >= 'A' && embeddedData->Zxx[i][6] <= 'F')
							tmp = (embeddedData->Zxx[i][6] - 'A' + 0xA) * 0x10;
						if (embeddedData->Zxx[i][7] >= '0' && embeddedData->Zxx[i][7] <= '9')
							tmp += (embeddedData->Zxx[i][7] - '0');
						else if (embeddedData->Zxx[i][7] >= 'A' && embeddedData->Zxx[i][7] <= 'F')
							tmp += (embeddedData->Zxx[i][7] - 'A' + 0xA);

						sampler->SetZxxMacro(i,mode,tmp);
					}
				}
			}
			else // initializing with the default midi.cfg values.
			{
				for(i = 0; i < 16;i++)
				{
					sampler->SetZxxMacro(i,1,i*8);
				}

				for(i = 16; i < 128;i++)
				{
					sampler->SetZxxMacro(i,-1,0);
				}
			}

			if ( itFileH.special&SpecialFlags::HASMESSAGE)
			{
				Seek(itFileH.msgOffset);
				// read itFileH.msgLen
				// NewLine = 0Dh (13 dec)
				// EndOfMsg = 0
			}


			for (i=0;i<itFileH.insNum;i++)
			{
				Seek(pointersi[i]);
				XMInstrument instr;
				if (itFileH.ffv < 0x200 ) {
					itInsHeader1x curH;
					Read(&curH,sizeof(curH));
					LoadOldITInst(curH,instr,i);
				}
				else {
					itInsHeader2x curH;
					Read(&curH,sizeof(curH));
					LoadITInst(curH, instr,i);
				}
				song.xminstruments.SetInst(instr,i);
			}
			for (i=0;i<itFileH.sampNum;i++)
			{
				Seek(pointerss[i]);
				XMInstrument::WaveData wave;
				itSampleHeader curH;
				Read(&curH,sizeof(curH));
				bool created = LoadITSample(curH,wave,i);
				// If this MOD doesn't use Instruments, we need to map the notes manually.
				if (created && !(itFileH.flags & Flags::USEINSTR)) 
				{
					if (song.xminstruments.IsEnabled(i) ==false) {
						XMInstrument instr;
						instr.Init();
						song.xminstruments.SetInst(instr,i);
					}
					XMInstrument& instrument = song.xminstruments.get(i);
					XMInstrument::NotePair npair;
					npair.second=i;
					for(int j = 0;j < XMInstrument::NOTE_MAP_SIZE;j++){
						npair.first=j;
						instrument.NoteToSample(j,npair);
					}
					instrument.IsEnabled(true);
				}
				song.samples.SetSample(wave,i);
			}
			int numchans=m_maxextracolumn;
			m_maxextracolumn=0;
			for (i=0;i<itFileH.patNum;i++)
			{
				if (pointersp[i]==0)
				{
					song.AllocNewPattern(i,"unnamed",64,false);
				} else {
					Seek(pointersp[i]);
					LoadITPattern(i,numchans);
				}
			}
			song.SONGTRACKS = std::max(numchans+1,(int)m_maxextracolumn);

			delete[] pointersi;
			delete[] pointerss;
			delete[] pointersp;
			
			return true;
		}

		bool ITModule2::LoadOldITInst(const itInsHeader1x& curH,XMInstrument &xins,int iInstIdx)
		{
			std::string itname(curH.sName);
			xins.Name(itname);

			xins.VolumeFadeSpeed(curH.fadeout/512.0f);

			xins.NNA((XMInstrument::NewNoteAction::Type)curH.NNA);
			if ( curH.DNC )
			{	
				xins.DCT(XMInstrument::DupeCheck::NOTE);
				xins.DCA(XMInstrument::NewNoteAction::STOP);
			}
			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=short(curH.notes[i].first);
				npair.second=short(curH.notes[i].second)-1;
				xins.NoteToSample(i,npair);
			}
			xins.AmpEnvelope().Init();
			if(curH.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.AmpEnvelope().IsEnabled(true);

				if(curH.flg& EnvFlags::USE_SUSTAIN){
					xins.AmpEnvelope().SustainBegin(curH.sustainS);
					xins.AmpEnvelope().SustainEnd(curH.sustainE);
				}

				if(curH.flg & EnvFlags::USE_LOOP){
					xins.AmpEnvelope().LoopStart(curH.loopS);
					xins.AmpEnvelope().LoopEnd(curH.loopE);
				}
				for(int i = 0; i < 25;i++){
					std::uint8_t tick = curH.nodepair[i].first;
					std::uint8_t value = curH.nodepair[i].second;
					if (value == 0xFF || tick == 0xFF) break;

					xins.AmpEnvelope().Append(tick,(float)value/ 64.0f);
				}
			}
			xins.PanEnvelope().Init();
			xins.PitchEnvelope().Init();
			xins.FilterEnvelope().Init();
			xins.IsEnabled(true);
			return true;
		}
		bool ITModule2::LoadITInst(const itInsHeader2x& curH, XMInstrument &xins,int iInstIdx)
		{
			std::string itname(curH.sName);
			xins.Name(itname);

			xins.NNA((XMInstrument::NewNoteAction::Type)curH.NNA);
			xins.DCT((XMInstrument::DupeCheck::Type)curH.DCT);
			switch (curH.DCA)
			{
			case DCAction::NOTEOFF:xins.DCA(XMInstrument::NewNoteAction::NOTEOFF);break;
			case DCAction::FADEOUT:xins.DCA(XMInstrument::NewNoteAction::FADEOUT);break;
			case DCAction::STOP://fallthrough
			default:xins.DCA(XMInstrument::NewNoteAction::NOTEOFF);break;
			}

			xins.Pan((curH.defPan & 0x7F)/64.0f);
			xins.PanEnabled((curH.defPan & 0x80)?false:true);
			xins.NoteModPanCenter(curH.pPanCenter);
			xins.NoteModPanSep(curH.pPanSep);
			xins.GlobVol(curH.gVol/127.0f);
			xins.VolumeFadeSpeed(curH.fadeout/1024.0f);
			xins.RandomVolume(curH.randVol);
			xins.RandomPanning(curH.randPan);
			if ( (curH.inFC&0x80) != 0)
			{
				xins.FilterType(dsp::F_ITLOWPASS);
				int fc = curH.inFC&0x7F;
				xins.FilterCutoff(fc);
			}
			if ((curH.inFR&0x80) != 0)
			{
				xins.FilterType(dsp::F_ITLOWPASS);
				xins.FilterResonance(curH.inFR&0x7F);
			}


			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=curH.notes[i].first;
				npair.second=curH.notes[i].second-1;
				xins.NoteToSample(i,npair);
			}

			// volume envelope
			xins.AmpEnvelope().Init();
			
			if(curH.volEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.AmpEnvelope().IsEnabled(true);
				if(curH.volEnv.flg& EnvFlags::ENABLE_CARRY) xins.AmpEnvelope().IsCarry(true);
				if(curH.volEnv.flg& EnvFlags::USE_SUSTAIN){
					xins.AmpEnvelope().SustainBegin(curH.volEnv.sustainS);
					xins.AmpEnvelope().SustainEnd(curH.volEnv.sustainE);
				}

				if(curH.volEnv.flg & EnvFlags::USE_LOOP){
					xins.AmpEnvelope().LoopStart(curH.volEnv.loopS);
					xins.AmpEnvelope().LoopEnd(curH.volEnv.loopE);
				}

				int envelope_point_num = curH.volEnv.numP;
				if(envelope_point_num > 25){
					envelope_point_num = 25;
				}

				for(int i = 0; i < envelope_point_num;i++){
					short envtmp = curH.volEnv.nodes[i].secondlo | (curH.volEnv.nodes[i].secondhi <<8);
					xins.AmpEnvelope().Append(envtmp ,(float)curH.volEnv.nodes[i].first/ 64.0f);
				}

			} else {
				xins.AmpEnvelope().IsEnabled(false);
			}

			// Pan envelope
			xins.PanEnvelope().Init();

			if(curH.panEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.PanEnvelope().IsEnabled(true);
				if(curH.panEnv.flg& EnvFlags::ENABLE_CARRY) xins.PanEnvelope().IsCarry(true);
				if(curH.panEnv.flg& EnvFlags::USE_SUSTAIN){
					xins.PanEnvelope().SustainBegin(curH.panEnv.sustainS);
					xins.PanEnvelope().SustainEnd(curH.panEnv.sustainE);
				}

				if(curH.panEnv.flg & EnvFlags::USE_LOOP){
					xins.PanEnvelope().LoopStart(curH.panEnv.loopS);
					xins.PanEnvelope().LoopEnd(curH.panEnv.loopE);
				}

				int envelope_point_num = curH.panEnv.numP;
				if(envelope_point_num > 25){ // Max number of envelope points in Impulse format is 25.
					envelope_point_num = 25;
				}

				for(int i = 0; i < envelope_point_num;i++){
					short pantmp = curH.panEnv.nodes[i].secondlo | (curH.panEnv.nodes[i].secondhi <<8);
					xins.PanEnvelope().Append(pantmp,(float)(curH.panEnv.nodes[i].first)/ 32.0f);
				}

			} else {
				xins.PanEnvelope().IsEnabled(false);
			}
			// Pitch/Filter envelope
			xins.PitchEnvelope().Init();
			xins.FilterEnvelope().Init();

			if(curH.pitchEnv.flg & EnvFlags::USE_ENVELOPE){// enable pitch/filter envelope
				int envelope_point_num = curH.pitchEnv.numP;
				if(envelope_point_num > 25){ // Max number of envelope points in Impulse format is 25.
					envelope_point_num = 25;
				}

				if (curH.pitchEnv.flg & EnvFlags::ISFILTER)
				{
					xins.FilterType(dsp::F_ITLOWPASS);
					xins.FilterEnvelope().IsEnabled(true);
					xins.PitchEnvelope().IsEnabled(false);
					if(curH.pitchEnv.flg& EnvFlags::ENABLE_CARRY) xins.FilterEnvelope().IsCarry(true);
					if(curH.pitchEnv.flg& EnvFlags::USE_SUSTAIN){
						xins.FilterEnvelope().SustainBegin(curH.pitchEnv.sustainS);
						xins.FilterEnvelope().SustainEnd(curH.pitchEnv.sustainE);
					}

					if(curH.pitchEnv.flg & EnvFlags::USE_LOOP){
						xins.FilterEnvelope().LoopStart(curH.pitchEnv.loopS);
						xins.FilterEnvelope().LoopEnd(curH.pitchEnv.loopE);
					}

					for(int i = 0; i < envelope_point_num;i++){
						short pitchtmp = curH.pitchEnv.nodes[i].secondlo | (curH.pitchEnv.nodes[i].secondhi <<8);
						xins.FilterEnvelope().Append(pitchtmp,(float)(curH.pitchEnv.nodes[i].first+32)/ 64.0f);
					}
					if ( xins.FilterCutoff() < 127 )
					{
						xins.FilterEnvAmount((-1)*xins.FilterCutoff());
					} else { xins.FilterEnvAmount(-128); }
				} else {
					xins.PitchEnvelope().IsEnabled(true);
					xins.FilterEnvelope().IsEnabled(false);
					if(curH.pitchEnv.flg& EnvFlags::ENABLE_CARRY) xins.PitchEnvelope().IsCarry(true);
					if(curH.pitchEnv.flg& EnvFlags::USE_SUSTAIN){
						xins.PitchEnvelope().SustainBegin(curH.pitchEnv.sustainS);
						xins.PitchEnvelope().SustainEnd(curH.pitchEnv.sustainE);
					}

					if(curH.pitchEnv.flg & EnvFlags::USE_LOOP){
						xins.PitchEnvelope().LoopStart(curH.pitchEnv.loopS);
						xins.PitchEnvelope().LoopEnd(curH.pitchEnv.loopE);
					}

					for(int i = 0; i < envelope_point_num;i++){
						short pitchtmp = curH.pitchEnv.nodes[i].secondlo | (curH.pitchEnv.nodes[i].secondhi <<8);
						xins.PitchEnvelope().Append(pitchtmp,(float)(curH.pitchEnv.nodes[i].first)/ 32.0f);
					}
				}
			} else {
				xins.PitchEnvelope().IsEnabled(false);
				xins.FilterEnvelope().IsEnabled(false);
			}

			xins.IsEnabled(true);
			return true;
		}

		bool ITModule2::LoadITSample(const itSampleHeader& curH, XMInstrument::WaveData& _wave,int iSampleIdx)
		{
			char renamed[26];
			for(int i=0;i<25;i++){
				if(curH.sName[i]=='\0') renamed[i]=' ';
				else renamed[i] = curH.sName[i];
			}
			renamed[25]='\0';
			std::string sName = renamed;

/*		      Flg:      Bit 0. On = sample associated with header.
			Bit 1. On = 16 bit, Off = 8 bit.
			Bit 2. On = stereo, Off = mono. Stereo samples not supported yet
			Bit 3. On = compressed samples.
			Bit 4. On = Use loop
			Bit 5. On = Use sustain loop
			Bit 6. On = Ping Pong loop, Off = Forwards loop
			Bit 7. On = Ping Pong Sustain loop, Off = Forwards Sustain loop
*/
			bool bstereo=curH.flg&SampleFlags::ISSTEREO;
			bool b16Bit=curH.flg&SampleFlags::IS16BIT;
			bool bcompressed=curH.flg&SampleFlags::ISCOMPRESSED;
			bool bLoop=curH.flg&SampleFlags::USELOOP;
			bool bsustainloop=curH.flg&SampleFlags::USESUSTAIN;

			if (curH.flg&SampleFlags::HAS_SAMPLE)
			{
				_wave.Init();
				_wave.AllocWaveData(curH.length,bstereo);

				_wave.WaveLoopStart(curH.loopB);
				_wave.WaveLoopEnd(curH.loopE);
				if(bLoop) {
					if (curH.flg&SampleFlags::ISLOOPPINPONG)
					{
						_wave.WaveLoopType(XMInstrument::WaveData::LoopType::BIDI);
					}
					else _wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
				} else {
					_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
				}
				_wave.WaveSusLoopStart(curH.sustainB);
				_wave.WaveSusLoopEnd(curH.sustainE);
				if(bsustainloop)
				{
					if (curH.flg&SampleFlags::ISSUSTAINPINPONG)
					{
						_wave.WaveSusLoopType(XMInstrument::WaveData::LoopType::BIDI);
					}
					else _wave.WaveSusLoopType(XMInstrument::WaveData::LoopType::NORMAL);
				} else {
					_wave.WaveSusLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
				}

				_wave.WaveVolume(curH.vol *2);
				_wave.WaveGlobVolume(curH.gVol /64.0f);


//				Older method. conversion from speed to tune. Replaced by using the samplerate directly
//				double tune = log(double(curH.c5Speed)/8363.0f)/log(double(2));
//				double maintune = floor(tune*12);
//				double finetune = floor(((tune*12)-maintune)*100);

				int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
					XMInstrument::WaveData::WaveForms::SAWDOWN,
					XMInstrument::WaveData::WaveForms::SQUARE,
					XMInstrument::WaveData::WaveForms::RANDOM
				};
//				_wave.WaveTune(maintune);
//				_wave.WaveFineTune(finetune);
				_wave.WaveSampleRate(curH.c5Speed);
				_wave.WaveName(sName);
				_wave.PanEnabled(curH.dfp&0x80);
				_wave.PanFactor((curH.dfp&0x7F)/64.0f);
				_wave.VibratoAttack(curH.vibR);
				_wave.VibratoSpeed(curH.vibS);
				_wave.VibratoDepth(curH.vibD);
				_wave.VibratoType(exchwave[curH.vibT&3]);

				if (curH.length > 0) {
					Seek(curH.smpData);
					if (bcompressed) LoadITCompressedData(_wave,iSampleIdx,curH.length,b16Bit,curH.cvt);
					else LoadITSampleData(_wave,iSampleIdx,curH.length,bstereo,b16Bit,curH.cvt);
				}
				return true;
			}

			return false;
		}

		bool ITModule2::LoadITSampleData(XMInstrument::WaveData& _wave,int iSampleIdx,std::uint32_t iLen,bool bstereo,bool b16Bit, unsigned char convert)
		{
			signed short wNew,wTmp;
			int offset=(convert & SampleConvert::IS_SIGNED)?0:-32768;
			int lobit=(convert & SampleConvert::IS_MOTOROLA)?8:0;
			int hibit=8-lobit;
			std::uint32_t j,out;

			if (b16Bit) iLen*=2;
			unsigned char * smpbuf = new unsigned char[iLen];
			Read(smpbuf,iLen);

			out=0;wNew=0;
			if (b16Bit) {
				for (j = 0; j < iLen; j+=2) {
					wTmp= ((smpbuf[j]<<lobit) | (smpbuf[j+1]<<hibit))+offset;
					wNew=(convert& SampleConvert::IS_DELTA)?wNew+wTmp:wTmp;
					*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew ^65535;
					out++;
				}
				if (bstereo) {
					Read(smpbuf,iLen);
					out=0;
					for (j = 0; j < iLen; j+=2) {
						wTmp= ((smpbuf[j]<<lobit) | (smpbuf[j+1]<<hibit))+offset;
						wNew=(convert& SampleConvert::IS_DELTA)?wNew+wTmp:wTmp;
						*(const_cast<signed short*>(_wave.pWaveDataR()) + out) = wNew ^65535;
						out++;
					}
				}
			} else {
				for (j = 0; j < iLen; j++) {
					wNew=(convert& SampleConvert::IS_DELTA)?wNew+smpbuf[j]:smpbuf[j];
					*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = ((wNew<<8)+offset)  ^65535;
				}
				if (bstereo) {
					Read(smpbuf,iLen);
					for (j = 0; j < iLen; j++) {
						wNew=(convert& SampleConvert::IS_DELTA)?wNew+smpbuf[j]:smpbuf[j];
						*(const_cast<signed short*>(_wave.pWaveDataR()) + j) = ((wNew<<8)+offset) ^65535;
					}
				}
			}
			delete [] smpbuf; smpbuf = 0;
			return true;
		}

		bool ITModule2::LoadITCompressedData(XMInstrument::WaveData& _wave,int iSampleIdx,std::uint32_t iLen,bool b16Bit,unsigned char convert)
		{
			unsigned char bitwidth,packsize,maxbitsize;
			std::uint32_t topsize, val,j;
			short d1, d2,wNew;
			char d18,d28;

			bool deltapack=(itFileH.ffv>=0x215 && convert&SampleConvert::IS_DELTA); // Impulse Tracker 2.15 added a delta packed compressed sample format
			
			if (b16Bit)	{
				topsize=0x4000;		packsize=4;	maxbitsize=16;
			} else {
				topsize=0x8000;		packsize=3;	maxbitsize=8;
			}
			
			j=0;
			while(j<iLen) // While we haven't decompressed the whole sample
			{	
				BitsBlock block;
				if (!block.ReadBlock(this)) return false;

				// Size of the block of data to process, in blocks of max size=0x8000bytes ( 0x4000 samples if 16bits)
				std::uint32_t blocksize=(iLen-j<topsize)?iLen-j:topsize;
				std::uint32_t blockpos=0;

				bitwidth = maxbitsize+1;
				d1 = d2 = 0;
				d18 = d28 = 0;

				//Start the decompression:
				while (blockpos < blocksize) {
					val = block.ReadBits(bitwidth);

					//Check if value contains a bitwidth change. If it does, change and proceed with next value.
					if (bitwidth < 7) { //Method 1:
						if (val == unsigned(1 << (bitwidth - 1))) { // if the value == topmost bit set.
							val = block.ReadBits(packsize) + 1;
							bitwidth = (val < bitwidth) ? val : val + 1;
							continue;
						}
					} else if (bitwidth < maxbitsize+1) { //Method 2
						unsigned short border = (((1<<maxbitsize)-1) >> (maxbitsize+1 - bitwidth)) - (maxbitsize/2);

						if ((val > border) && (val <= unsigned(border + maxbitsize))) {
							val -= border;
							bitwidth = val < bitwidth ? val : val + 1;
							continue;
						}
					} else if (bitwidth == maxbitsize+1) { //Method 3
						if (val & (1<<maxbitsize)) {
							bitwidth = (val + 1) & 0xFF;
							continue;
						}
					} else { //Illegal width, abort ?
						return false;
					}

					//If we reach here, val contains a value to decompress, so do it.
					{
						if (b16Bit) 
						{	
							short v; //The sample value:
							if (bitwidth < maxbitsize) {
								unsigned char shift = maxbitsize - bitwidth;
								v = (short)(val << shift);
								v >>= shift;
							}
							else
								v = (short)val;

							//And integrate the sample value
							d1 += v;
							d2 += d1;
							wNew = deltapack?d2:d1;
						}
						else
						{	
							char v; //The sample value:
							if (bitwidth < maxbitsize) {
								unsigned char shift = maxbitsize - bitwidth;
								v = (val << shift);
								v >>= shift;
							}
							else
								v = (char)val;

								d18 +=v;
								d28 +=d18;
								wNew = deltapack?d28:d18;
								wNew <<=8;
						}
					}

					//Store the decompressed value to Wave pointer.
					*(const_cast<signed short*>(_wave.pWaveDataL()+j+blockpos)) = wNew;
					
					blockpos++;
				}
				j+=blocksize;
			}
			return false;
		}

		bool ITModule2::LoadITPattern(int patIdx, int &numchans)
		{
			unsigned char newEntry;
			unsigned char lastnote[64];
			unsigned char lastinst[64];
			unsigned char lastvol[64];
			unsigned char lastcom[64];
			unsigned char lasteff[64];
			unsigned char mask[64];
			std::memset(lastnote,255,sizeof(char)*64);
			std::memset(lastinst,255,sizeof(char)*64);
			std::memset(lastvol,255,sizeof(char)*64);
			std::memset(lastcom,255,sizeof(char)*64);
			std::memset(lasteff,255,sizeof(char)*64);
			std::memset(mask,255,sizeof(char)*64);

			PatternEntry pempty;
			pempty._note=notecommands::empty; pempty._mach=255;pempty._inst=255;pempty._cmd=0;pempty._parameter=0;
			PatternEntry pent=pempty;

			Skip(2); // packedSize
			std::int16_t rowCount=ReadInt16();
			Skip(4); // unused
			if (rowCount > MAX_LINES ) rowCount=MAX_LINES;
			_pSong->AllocNewPattern(patIdx,"unnamed",rowCount,false);
			//char* packedpattern = new char[packedSize];
			//Read(packedpattern, packedSize);
			for (int row=0;row<rowCount;row++)
			{
				m_extracolumn = numchans+1;
				Read(&newEntry,1);
				while ( newEntry )
				{
					unsigned char channel=(newEntry-1)&0x3F;
					if (channel >= m_extracolumn) { m_extracolumn = channel+1;}
					if (newEntry&0x80) mask[channel]=ReadUInt8();
					unsigned char volume=255;
					if(mask[channel]&1)
					{
						unsigned char note=ReadUInt8();
						if (note==255) pent._note = notecommands::release;
						else if (note==254) pent._note=notecommands::release; //\todo: Attention ! Psycle doesn't have a note-cut note.
						else pent._note = note;
						pent._mach=0;
						lastnote[channel]=pent._note;
					}
					if (mask[channel]&2)
					{
						pent._inst=ReadUInt8()-1;
						pent._mach=0;
						lastinst[channel]=pent._inst;
					}
					if (mask[channel]&4 || mask[channel]&0x40)
					{
						unsigned char tmp;
						pent._mach=0;
						if (mask[channel]&0x40 ) tmp=lastvol[channel];
						else tmp=ReadUInt8();
						lastvol[channel]=tmp;
						// Volume ranges from 0->64
						// Panning ranges from 0->64, mapped onto 128->192
						// Prepare for the following also:
						//  65->74 = Fine volume up
						//  75->84 = Fine volume down
						//  85->94 = Volume slide up
						//  95->104 = Volume slide down
						//  105->114 = Pitch Slide down
						//  115->124 = Pitch Slide up
						//  193->202 = Portamento to
						//  203->212 = Vibrato
						if ( tmp<=64)
						{
							volume=tmp<64?tmp:63;
						}
						else if (tmp<75)
						{
							volume=XMSampler::CMD_VOL::VOL_FINEVOLSLIDEUP | (tmp-65);
						}
						else if (tmp<85)
						{
							volume=XMSampler::CMD_VOL::VOL_FINEVOLSLIDEDOWN | (tmp-75);
						}
						else if (tmp<95)
						{
							volume=XMSampler::CMD_VOL::VOL_VOLSLIDEUP | (tmp-85);
						}
						else if (tmp<105)
						{
							volume=XMSampler::CMD_VOL::VOL_VOLSLIDEDOWN | (tmp-95);
						}
						else if (tmp<115)
						{
							volume=XMSampler::CMD_VOL::VOL_PITCH_SLIDE_DOWN | (tmp-105);
						}
						else if (tmp<125)
						{
							volume=XMSampler::CMD_VOL::VOL_PITCH_SLIDE_UP | (tmp-115);
						}
						else if (tmp<193)
						{
							tmp= (tmp==192)?15:(tmp-128)/4;
							volume=XMSampler::CMD_VOL::VOL_PANNING | tmp;
						}
						else if (tmp<203)
						{
							volume=XMSampler::CMD_VOL::VOL_TONEPORTAMENTO | (tmp-193);
						}
						else if (tmp<213)
						{
							volume=XMSampler::CMD_VOL::VOL_VIBRATO | ( tmp-203 );
						}
					}
					std::uint8_t param=6;
					if(mask[channel]&8)
					{
						pent._mach=0;
						std::uint8_t command=ReadUInt8();
						param=ReadUInt8();
						if ( command != 0 ) pent._parameter = param;
						ParseEffect(pent,patIdx,row,command,param,channel);
						lastcom[channel]=pent._cmd;
						lasteff[channel]=pent._parameter;

					}
					if (mask[channel]&0x10) { pent._note=lastnote[channel]; pent._mach=0; }
					if (mask[channel]&0x20) { pent._inst=lastinst[channel]; pent._mach=0; }
					if ( mask[channel]&0x80 )
					{
						pent._cmd = lastcom[channel];
						pent._parameter = lasteff[channel];
					}

#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
					pent._volume = volume;
#else
					if(pent._cmd != 0 || pent._parameter != 0) {
						if(volume!=255 && m_extracolumn < MAX_TRACKS) {
							PatternEntry* pData = reinterpret_cast<PatternEntry*>(_pSong->_ptrackline(patIdx,m_extracolumn,row));
							pData->_note = notecommands::midicc;
							pData->_inst = channel;
							pData->_mach = pent._mach;
							pData->_cmd = XMSampler::CMD::SENDTOVOLUME;
							pData->_parameter = volume;
							m_extracolumn++;
						}
					}
					else if(volume < 0x40) {
						pent._cmd = XMSampler::CMD::VOLUME;
						pent._parameter = volume*2;
					}
					else if(volume!=255) {
						pent._cmd = XMSampler::CMD::SENDTOVOLUME;
						pent._parameter = volume;
					}
	#endif
#endif
					PatternEntry* pData = reinterpret_cast<PatternEntry*>(_pSong->_ptrackline(patIdx,channel,row));
					*pData = pent;
					pent=pempty;

					numchans = std::max((int)channel,numchans);

					Read(&newEntry,1);
				}
				m_maxextracolumn = std::max(m_maxextracolumn,m_extracolumn);
			}
			return true;
		}

		void ITModule2::ParseEffect(PatternEntry&pent, int patIdx, int row, int command,int param,int channel)
		{
			int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
				XMInstrument::WaveData::WaveForms::SAWDOWN,
				XMInstrument::WaveData::WaveForms::SQUARE,
				XMInstrument::WaveData::WaveForms::RANDOM
			};
			switch(command){
				case ITModule2::CMD::SET_SPEED:
					{
						pent._cmd=PatternCmd::EXTENDED;
						int extraticks=0;
						pent._parameter = XMSampler::CalcLPBFromSpeed(param,extraticks);
						if (extraticks != 0 && m_extracolumn < MAX_TRACKS) {
							PatternEntry* pData = reinterpret_cast<PatternEntry*>(_pSong->_ptrackline(patIdx,m_extracolumn,row));
							pData->_note = notecommands::empty;
							pData->_inst = 255;
							pData->_mach = pent._mach;
							pData->_cmd = PatternCmd::EXTENDED;
							pData->_parameter = PatternCmd::MEMORY_PAT_DELAY | extraticks;
							m_extracolumn++;
						}
					}
					break;
				case ITModule2::CMD::JUMP_TO_ORDER:
					pent._cmd = PatternCmd::JUMP_TO_ORDER;
					break;
				case ITModule2::CMD::BREAK_TO_ROW:
					pent._cmd = PatternCmd::BREAK_TO_LINE;
					break;
				case ITModule2::CMD::VOLUME_SLIDE:
					pent._cmd = XMSampler::CMD::VOLUMESLIDE;
					break;
				case ITModule2::CMD::PORTAMENTO_DOWN:
					pent._cmd = XMSampler::CMD::PORTAMENTO_DOWN;
					break;
				case ITModule2::CMD::PORTAMENTO_UP:
					pent._cmd = XMSampler::CMD::PORTAMENTO_UP;
					break;
				case ITModule2::CMD::TONE_PORTAMENTO:
					pent._cmd = XMSampler::CMD::PORTA2NOTE;
					break;
				case ITModule2::CMD::VIBRATO:
					pent._cmd = XMSampler::CMD::VIBRATO;
					break;
				case ITModule2::CMD::TREMOR:
					pent._cmd = XMSampler::CMD::TREMOR;
					break;
				case ITModule2::CMD::ARPEGGIO:
					pent._cmd = XMSampler::CMD::ARPEGGIO;
					break;
				case ITModule2::CMD::VOLSLIDE_VIBRATO:
					pent._cmd = XMSampler::CMD::VIBRATOVOL;
					break;
				case ITModule2::CMD::VOLSLIDE_TONEPORTA:
					pent._cmd = XMSampler::CMD::TONEPORTAVOL;
					break;
				case CMD::SET_CHANNEL_VOLUME: // IT
					pent._cmd = XMSampler::CMD::SET_CHANNEL_VOLUME;
					break;
				case CMD::CHANNEL_VOLUME_SLIDE: // IT
					pent._cmd = XMSampler::CMD::CHANNEL_VOLUME_SLIDE;
					break;
				case CMD::SET_SAMPLE_OFFSET:
					pent._cmd = XMSampler::CMD::OFFSET | highOffset[channel];
					break;
				case ITModule2::CMD::PANNING_SLIDE: // IT
					pent._cmd = XMSampler::CMD::PANNINGSLIDE;
					break;
				case ITModule2::CMD::RETRIGGER_NOTE:
					pent._cmd = XMSampler::CMD::RETRIG;
					break;
				case ITModule2::CMD::TREMOLO:
					pent._cmd = XMSampler::CMD::TREMOLO;
					break;
				case ITModule2::CMD::S:
					switch(param & 0xf0){
						case CMD_S::S_SET_FILTER:
							pent._cmd = XMSampler::CMD::NONE;
							break;
						case CMD_S::S_SET_GLISSANDO_CONTROL:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_GLISSANDO_TYPE | (param & 0xf);
							break;
						case CMD_S::S_FINETUNE:
							pent._cmd = XMSampler::CMD::NONE;
							break;
						case CMD_S::S_SET_VIBRATO_WAVEFORM:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[(param & 0x3)];
							break;
						case CMD_S::S_SET_TREMOLO_WAVEFORM:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[(param & 0x3)];
							break;
						case CMD_S::S_SET_PANBRELLO_WAVEFORM: // IT
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_PANBRELLO_WAVE | exchwave[(param & 0x3)];
							break;
						case CMD_S::S_FINE_PATTERN_DELAY: // IT
							break;
						case CMD_S::S7: // IT
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::EE | (param&0x0F);
							break;
						case CMD_S::S_SET_PAN:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_SET_PAN | (param & 0xf);
							break;
						case CMD_S::S9: // IT
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E9 | (param&0x0F);
							break;
						case CMD_S::S_SET_HIGH_OFFSET: // IT
							highOffset[channel] = param &0x0F;
							pent._cmd = XMSampler::CMD::NONE;
							pent._parameter = 0;
							break;
						case CMD_S::S_PATTERN_LOOP:
							pent._cmd = PatternCmd::EXTENDED;
							pent._parameter = PatternCmd::PATTERN_LOOP | (param & 0xf);
							break;
						case CMD_S::S_DELAYED_NOTE_CUT:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_DELAYED_NOTECUT  | (param & 0xf);
							break;
						case CMD_S::S_NOTE_DELAY:
							pent._cmd = XMSampler::CMD::EXTENDED;
							pent._parameter = XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf);
							break;
						case CMD_S::S_PATTERN_DELAY:
							pent._cmd = PatternCmd::EXTENDED;
							pent._parameter = PatternCmd::PATTERN_DELAY | (param & 0xf);
							break;
						case CMD_S::S_SET_MIDI_MACRO:
							pent._cmd = XMSampler::CMD::EXTENDED;
							if ( embeddedData)
							{
								std::string zxx = embeddedData->SFx[(param & 0xF)];
								std::string zxx2 = zxx.substr(0,5);
								if ( zxx2 == "F0F00")
								{
									pent._parameter = XMSampler::CMD_E::E_SET_MIDI_MACRO | (embeddedData->SFx[(param & 0xF)][5]-'0');
								}
							}
							break;
					}
					break;
				case CMD::SET_SONG_TEMPO:
					pent._cmd = PatternCmd::SET_TEMPO;
					break;
				case CMD::FINE_VIBRATO:
					pent._cmd = XMSampler::CMD::FINE_VIBRATO;
					break;
				case CMD::SET_GLOBAL_VOLUME: 
					pent._cmd = XMSampler::CMD::SET_GLOBAL_VOLUME;
					break;
				case CMD::GLOBAL_VOLUME_SLIDE: // IT
					pent._cmd = XMSampler::CMD::GLOBAL_VOLUME_SLIDE;
					break;
				case CMD::SET_PANNING: // IT
					pent._cmd = XMSampler::CMD::PANNING;
					break;
				case CMD::PANBRELLO: // IT
					pent._cmd = XMSampler::CMD::PANBRELLO;
					break;
				case CMD::MIDI_MACRO:
					if ( param < 127)
					{
						pent._parameter = param;
					}
					pent._cmd = XMSampler::CMD::MIDI_MACRO;
					break;
				default:
					pent._cmd = XMSampler::CMD::NONE;
					break;
			}
		}

//////////////////////////////////////////////////////////////////////////
//     S3M Module Members


		bool ITModule2::LoadS3MModuleX(Song& song)
		{
			CExclusiveLock lock(&song.semaphore, 2, true);
			_pSong=&song;
			if (Read(&s3mFileH,sizeof(s3mFileH))==0 ) return 0;
			if (s3mFileH.tag != SCRM_ID || s3mFileH.type != 0x10 ) return 0;

			s3mFileH.songName[28]='\0';
			song.name = s3mFileH.songName;
			song.author = "";
			song.comments = "Imported from Scream Tracker 3 Module: ";
			song.comments.append(szName);

			song.CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, "sampulse",0);
			int connectionIdx = song.InsertConnectionNonBlocking(0,MASTER_INDEX,0,0,(s3mFileH.mVol&0x7F)/128.0f);
			song.seqBus=0;
			XMSampler* sampler = ((XMSampler*)song._pMachine[0]);

			song.BeatsPerMin(s3mFileH.iTempo);
			int extraticks=0;
			song.LinesPerBeat(XMSampler::CalcLPBFromSpeed(s3mFileH.iSpeed,extraticks));
			if (extraticks != 0) {
				//\todo: setup something...
			}

			sampler->IsAmigaSlides(true);
			sampler->GlobalVolume((s3mFileH.gVol&0x7F)*2);
			
			std::uint16_t j,i=0;
			for (j=0;j<s3mFileH.ordNum;j++)
			{
				song.playOrder[i]=ReadUInt8(); // 254 = ++ (skip), 255 = --- (end of tune).
				if (song.playOrder[i]!= 254 &&song.playOrder[i] != 255 ) i++;
			}
			song.playLength=i;
			if ( song.playLength == 0) // Add at least one pattern to the sequence.
			{
				song.playLength = 1;
				song.playOrder[0]=0;
			}

			unsigned short *pointersi = new unsigned short[s3mFileH.insNum];
			Read(pointersi,s3mFileH.insNum*sizeof(unsigned short));
			unsigned short * pointersp = new unsigned short[s3mFileH.patNum];
			Read(pointersp,s3mFileH.patNum*sizeof(unsigned short));
			unsigned char chansettings[32];
			if ( s3mFileH.defPan==0xFC )
			{
				Read(chansettings,sizeof(chansettings));
			}
			bool stereo=s3mFileH.mVol&0x80;
			if (stereo) {
				// Note that in stereo, the mastermul is internally multiplied by 11/8 inside the player since
				// there is generally more room in the output stream
				sampler->SetDestWireVolume(connectionIdx,((s3mFileH.mVol&0x7F)/128.0f)*(11.f/8.f));
			}
			int numchans=0;
			for (i=0;i<32;i++)
			{
				if ( s3mFileH.chanSet[i]==S3MChanType::ISUNUSED) 
				{
					sampler->rChannel(i).DefaultIsMute(true);
					sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
				}
				else
				{
					numchans=i+1; // topmost used channel.
					if(s3mFileH.chanSet[i]&S3MChanType::ISDISABLED)
					{
						sampler->rChannel(i).DefaultIsMute(true);
					}
					if (stereo && !(s3mFileH.chanSet[i]&S3MChanType::ISADLIBCHAN))
					{
						if (s3mFileH.defPan && chansettings[i]&S3MChanType::HASCUSTOMPOS)
						{
							float flttmp=XMSampler::E8VolMap[(chansettings[i]&0x0F)]/64.0f;
							sampler->rChannel(i).DefaultPanFactorFloat(flttmp,true);
						}
						else if (s3mFileH.chanSet[i]&S3MChanType::ISRIGHTCHAN)
							sampler->rChannel(i).DefaultPanFactorFloat(0.80f,true);
						else 
							sampler->rChannel(i).DefaultPanFactorFloat(0.20f,true);
					}
					else
						sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
				}
			}
			song.SONGTRACKS=std::max(numchans,4);

			for (i=0;i<s3mFileH.insNum;i++)
			{
				Seek(pointersi[i]<<4);
				XMInstrument instr;
				LoadS3MInstX(song, instr,i);
				song.xminstruments.SetInst(instr,i);
			}
			m_maxextracolumn=song.SONGTRACKS;
			for (i=0;i<s3mFileH.patNum;i++)
			{
				Seek(pointersp[i]<<4);
				LoadS3MPatternX(i);
			}
			song.SONGTRACKS=m_maxextracolumn;
			delete[] pointersi; pointersi = 0;
			delete[] pointersp; pointersp = 0;

			return true;
		}

		bool ITModule2::LoadS3MInstX(Song& song, XMInstrument &xins,std::uint16_t iInstIdx)
		{
			s3mInstHeader curH;
			Read(&curH,sizeof(curH));

			xins.Init();
			xins.Name(curH.sName);

			int i;
			XMInstrument::NotePair npair;
			npair.second=iInstIdx;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=i;
				xins.NoteToSample(i,npair);
			}
			
			if ( curH.type == 1) 
			{
				xins.IsEnabled(true);
				XMInstrument::WaveData wave;
				bool result = LoadS3MSampleX(wave,reinterpret_cast<s3mSampleHeader*>(&curH),iInstIdx,iInstIdx);
				song.samples.SetSample(wave,iInstIdx);
				return result;
			}
			else if ( curH.type != 0)
			{
/*					reinterpret_cast<s3madlibheader*>(&curH)
*
OFFSET              Count TYPE   Description
0000h                   1 byte   Instrument type
									2 - melodic instrument
									3 - bass drum
									4 - snare drum
									5 - tom tom
									6 - cymbal
									7 - hihat
0001h                  12 char   DOS file name
000Dh                   3 byte   reserved
0010h                   1 byte   Modulator description (bitmapped)
									0-3 - frequency multiplier
									4 - scale envelope
									5 - sustain
									6 - pitch vibrato
									7 - volume vibrato
0011h                   1 byte   Carrier description (same as modulator)
0012h                   1 byte   Modulator miscellaneous (bitmapped)
									0-5 - 63-volume
									6 - MSB of levelscale
									7 - LSB of levelscale
0013h                   1 byte   Carrier description (same as modulator)
0014h                   1 byte   Modulator attack / decay byte (bitmapped)
									0-3 - Decay
									4-7 - Attack
0015h                   1 byte   Carrier description (same as modulator)
0016h                   1 byte   Modulator sustain / release byte (bitmapped)
									0-3 - Release count
									4-7 - 15-Sustain
0017h                   1 byte   Carrier description (same as modulator)
0018h                   1 byte   Modulator wave select
0019h                   1 byte   Carrier wave select
001Ah                   1 byte   Modulator feedback byte (bitmapped)
									0 - additive synthesis on/off
									1-7 - modulation feedback
 */
			}
			return false;
		}

		bool ITModule2::LoadS3MSampleX(XMInstrument::WaveData& _wave,s3mSampleHeader *currHeader,std::uint16_t iInstIdx,std::uint16_t iSampleIdx)
		{
			bool bLoop=currHeader->flags&S3MSampleFlags::LOOP;
			bool bstereo=currHeader->flags&S3MSampleFlags::STEREO;
			bool b16Bit=currHeader->flags&S3MSampleFlags::IS16BIT;

			_wave.Init();

			_wave.WaveLoopStart(currHeader->loopb);
			_wave.WaveLoopEnd(currHeader->loope);
			if(bLoop) {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
			} else {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
			}

			_wave.WaveVolume(currHeader->vol * 2);

//			Older method. conversion from speed to tune. Replaced by using the samplerate directly
//			double tune = log(double(currHeader->c2speed)/8363.0f)/log(double(2));
//			double maintune = floor(tune*12);
//			double finetune = floor(((tune*12)-maintune)*100);

//			_wave.WaveTune(maintune);
//			_wave.WaveFineTune(finetune);
			_wave.WaveSampleRate(currHeader->c2speed);

			std::string sName = currHeader->sName;
			_wave.WaveName(sName);

			int newpos=((currHeader->hiMemSeg<<16)+currHeader->lomemSeg)<<4;
			Seek(newpos);
			LoadS3MSampleDataX(_wave,iInstIdx,iSampleIdx,currHeader->length,bstereo,b16Bit,currHeader->packed);

			return true;
		}

		bool ITModule2::LoadS3MSampleDataX(XMInstrument::WaveData& _wave,std::uint16_t iInstIdx,std::uint16_t iSampleIdx,std::uint32_t iLen,bool bstereo,bool b16Bit,bool packed)
		{
			if (!packed) // Looks like the packed format never existed.
			{
				char * smpbuf;
				if(b16Bit)
				{
					smpbuf = new char[iLen*2];
					Read(smpbuf,iLen*2);
				} else 	{
					smpbuf = new char[iLen];
					Read(smpbuf,iLen);
				}
				std::int16_t wNew;
				std::int16_t offset;
				if ( s3mFileH.trackerInf==1) offset=0; // 1=[VERY OLD] signed samples, 2=unsigned samples
				else offset=-32768;

				//+2=stereo (after Length bytes for LEFT channel, another Length bytes for RIGHT channel)
                //+4=16-bit sample (intel LO-HI byteorder) (+2/+4 not supported by ST3.01)
				if(b16Bit) {
					int out=0;
					if (bstereo) {
						_wave.AllocWaveData(iLen,true);
						unsigned int j;
						for(j=0;j<iLen*2;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataL()) + out) = wNew;
						}
						out=0;
						Read(smpbuf,iLen*2);
						for(j=0;j<iLen*2;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8) +offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataR()) + out) = wNew;
							out++;
						}   
					} else {
						_wave.AllocWaveData(iLen,false);
						unsigned int j;
						for(j=0;j<iLen*2;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataL()) + out) = wNew;
							out++;
						}
					}
				} else {// 8 bit sample
					if (bstereo) {
						_wave.AllocWaveData(iLen,true);
						unsigned int j;
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataL()) + j) = wNew; //| char(rand()); // Add dither;
						}
						Read(smpbuf,iLen);
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataR()) + j) = wNew; //| char(rand()); // Add dither;
						}
					} else {
						_wave.AllocWaveData(iLen,false);
						unsigned int j;
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataL()) + j) = wNew; //| char(rand()); // Add dither;
						}
					}
				}

				// cleanup
				delete[] smpbuf; smpbuf = 0;
				return true;
			}
			return false;
		}

		bool ITModule2::LoadS3MPatternX(std::uint16_t patIdx)
		{
			std::uint8_t newEntry;
			PatternEntry pempty;
			pempty._note=notecommands::empty; pempty._mach=255;pempty._inst=255;pempty._cmd=0;pempty._parameter=0;
			PatternEntry pent=pempty;

			Skip(2);//int packedSize=ReadInt(2);
			_pSong->AllocNewPattern(patIdx,"unnamed",64,false);
//			char* packedpattern = new char[packedsize];
//			Read(packedpattern,packedsize);
			for (int row=0;row<64;row++)
			{
				m_extracolumn=_pSong->SONGTRACKS;
				Read(&newEntry,1);
				while ( newEntry )
				{
					std::uint8_t channel=newEntry&31;
					std::uint8_t volume=255;
					if(newEntry&32)
					{
						std::uint8_t note=ReadUInt8();  // hi=oct, lo=note, 255=empty note,	254=key off
						if (note==254) pent._note = notecommands::release;
						else if (note==255) pent._note=255;
						else pent._note = ((note/16)*12+(note%16)+12);  // +12 since ST3 C-4 is Psycle's C-5
						pent._inst=ReadUInt8()-1;
						pent._mach=0;
					}
					if(newEntry&64)
					{
						std::uint8_t tmp=ReadUInt8();
						pent._mach =0;
						volume = (tmp<64)?tmp:63;
					}
					if(newEntry&128)
					{
						pent._mach=0;
						std::uint8_t command=ReadUInt8();
						std::uint8_t param=ReadUInt8();
						if ( command != 0 ) pent._parameter = param;
						ParseEffect(pent,patIdx,row,command,param,channel);
						if ( pent._cmd == PatternCmd::BREAK_TO_LINE )
						{
							pent._parameter = ((pent._parameter&0xF0)>>4)*10 + (pent._parameter&0x0F);
						}
						else if ( pent._cmd == XMSampler::CMD::SET_GLOBAL_VOLUME )
						{
							pent._parameter = (pent._parameter<0x40)?pent._parameter*2:0x80;
						}
						else if ( pent._cmd == XMSampler::CMD::PANNING )
						{
							if ( pent._parameter < 0x80) pent._parameter = pent._parameter*2;
							else if ( pent._parameter == 0x80 ) pent._parameter = 255;
							else if ( pent._parameter == 0xA4) 
							{
								pent._cmd = XMSampler::CMD::EXTENDED;
								pent._parameter = XMSampler::CMD_E::E9 | 1;
							}
						}
					}
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
					pent._volume = volume;
#else
					if(pent._cmd != 0 || pent._parameter != 0) {
						if(volume!=255) {
							PatternEntry* pData = reinterpret_cast<PatternEntry*>(_pSong->_ptrackline(patIdx,m_extracolumn,row));
							pData->_note = notecommands::midicc;
							pData->_inst = channel;
							pData->_mach = pent._mach;
							pData->_cmd = XMSampler::CMD::SENDTOVOLUME;
							pData->_parameter = volume;
							m_extracolumn++;
						}
					}
					else if(volume < 0x40) {
						pent._cmd = XMSampler::CMD::VOLUME;
						pent._parameter = volume*2;
					}
					else if(volume!=255) {
						pent._cmd = XMSampler::CMD::SENDTOVOLUME;
						pent._parameter = volume;
					}
	#endif
#endif
					if (channel < _pSong->SONGTRACKS) {
						PatternEntry* pData = reinterpret_cast<PatternEntry*>(_pSong->_ptrackline(patIdx,channel,row));
						*pData = pent;
					}
					pent=pempty;

					Read(&newEntry,1);
				}
				m_maxextracolumn = std::max(m_maxextracolumn,m_extracolumn);
			}
			return true;
		}
	}
}

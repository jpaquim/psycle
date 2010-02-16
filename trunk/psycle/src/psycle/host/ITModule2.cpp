#include "ITModule2.h"
#include "Configuration.hpp"

#include <algorithm>

#include <psycle/core/machinefactory.h>
#include <psycle/core/player.h>
#include <psycle/core/sequence.h>
#include <psycle/core/song.h>
#include <psycle/core/xmsampler.h>

using namespace psycle::core;

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

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
		bool ITModule2::BitsBlock::ReadBlock(RiffFile *pFile)
		{
			// block layout : uint16 size, <size> bytes data
			std::uint16_t size;
			pFile->Read(size);
			pdata = new unsigned char[size];
			if (!pdata) return false;
			if (!pFile->ReadArray(pdata,size))
			{
				delete[] pdata;
				return false;
			}
			rpos=pdata;
			rend=pdata+size;
			rembits=8;
			return true;
		}

		unsigned long ITModule2::BitsBlock::ReadBits(unsigned char bitwidth)
		{
			unsigned long val = 0;
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
		void ITModule2::LoadInstrumentFromFile(XMSampler & sampler, const int idx)
		{
			itFileH.flags=0;
			itInsHeader2x inshead;
			ReadHeader(inshead);
			Seek(0);
			s->rInstrument(idx).Init();
			LoadITInst(&sampler,idx);
			Skip(2);

			int curSample(0);
			unsigned char *sRemap = new unsigned char[inshead.noS];
			for (unsigned int i(0); i<inshead.noS; i++)
			{
				while (s->SampleData(curSample).WaveLength() > 0 && curSample < MAX_INSTRUMENTS-1) curSample++;
				LoadITSample(&sampler,curSample);
				// Only get REAL samples.
				if ( s->SampleData(curSample).WaveLength() > 0 && curSample < MAX_INSTRUMENTS-2 ) {	sRemap[i]=curSample; }
				else { sRemap[i]=MAX_INSTRUMENTS-1; }
			}

			int i;
			
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++)
			{
				XMInstrument::NotePair npair = s->rInstrument(idx).NoteToSample(i);
				npair.second=sRemap[(npair.second<inshead.noS)?npair.second:0];
				s->rInstrument(idx).NoteToSample(i,npair);
			}
			delete[] sRemap;

		}
		bool ITModule2::LoadITModule(Song* song)
		{
			s=song;
			if (ReadHeader(itFileH)==0 ) return false;
			if (itFileH.tag != IMPM_ID ) return false;

			s->setName(itFileH.songName);
			s->setAuthor("");
			std::string imported = "Imported from Impulse Tracker Module: ";
			imported.append(file_name());
			s->setComment(imported);

			song->SetReady(false);
			sampler = (XMSampler*) MachineFactory::getInstance().CreateMachine(InternalKeys::sampulse);
			s->AddMachine(sampler);
			s->InsertConnection(*sampler,*s->machine(MASTER_INDEX),0,0,(itFileH.mVol>128?128:itFileH.mVol)/128.0f);
			s->seqBus=sampler->id();

			song->BeatsPerMin(itFileH.iTempo);
			song->LinesPerBeat(sampler->Speed2LPB(itFileH.iSpeed));

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
				if (stereo)
				{
					if (itFileH.chanPan[i]==ChanFlags::IS_SURROUND )
					{
						sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
						sampler->rChannel(i).DefaultIsSurround(true);
					}
					else if ( !(itFileH.chanPan[i]&ChanFlags::IS_DISABLED) )
					{
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
				sampler->rChannel(i).DefaultFilterType(dsp::F_LOWPASS12);
			}

			song->sequence().removeAll();
			// here we add in one single Line the patterns
			SequenceLine* line = song->sequence().createNewLine();
			std::vector<int> seq_list;
			i=0;
			for (j=0;j<itFileH.ordNum && i<MAX_SONG_POSITIONS;j++)
			{
				seq_list.push_back(ReadUInt8());
/*				s->playOrder[i]=ReadUInt8(); // 254 = ++ (skip), 255 = --- (end of tune).
				if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;*/
			}
			Skip(itFileH.ordNum-j);
/*
			while (j<itFileH.ordNum)
			{
				char tmp=ReadInt(1);
				j++;
			}
*/
			/*s->playLength=i;
			if ( s->playLength == 0) // Add at least one pattern to the sequence.
			{
				s->playLength = 1;
				s->playOrder[0]=0;
			}*/

			unsigned long *pointersi = new unsigned long[itFileH.insNum];
			ReadArray(pointersi,itFileH.insNum);
			unsigned long * pointerss = new unsigned long[itFileH.sampNum];
			ReadArray(pointerss,itFileH.sampNum);
			unsigned long * pointersp = new unsigned long[itFileH.patNum];
			ReadArray(pointersp,itFileH.patNum);

			if ( itFileH.special&SpecialFlags::MIDIEMBEDDED)
			{
				embeddedData = new EmbeddedMIDIData;
				EmbeddedMIDIData mdata;
				short skipnum;
				Read(skipnum);
				Skip(skipnum*8); // This is some strange data. It is not documented.

				ReadHeader(mdata);

				for ( int i=0; i<128; i++ )
				{
					CString zxx = mdata.Zxx[i];
					CString zxx2 = zxx.Left(5);
					if ( strcmp("F0F00",zxx2) == 0)
					{
						int tmp=0;
						if (mdata.Zxx[i][5] >= '0' && mdata.Zxx[i][5] <= '9')
							tmp = mdata.Zxx[i][5] - '0';
						else if (mdata.Zxx[i][5] >= 'A' && mdata.Zxx[i][5] <= 'F')
							tmp = mdata.Zxx[i][5] - 'A' + 0xA;

						int mode = tmp;

						if (mdata.Zxx[i][6] >= '0' && mdata.Zxx[i][6] <= '9')
							tmp = (mdata.Zxx[i][6] - '0') * 10;
						else if (mdata.Zxx[i][6] >= 'A' && mdata.Zxx[i][6] <= 'F')
							tmp = (mdata.Zxx[i][6] - 'A' + 0xA) * 10;
						if (mdata.Zxx[i][7] >= '0' && mdata.Zxx[i][7] <= '9')
							tmp += (mdata.Zxx[i][7] - '0');
						else if (mdata.Zxx[i][7] >= 'A' && mdata.Zxx[i][7] <= 'F')
							tmp += (mdata.Zxx[i][7] - 'A' + 0xA);

						sampler->SetZxxMacro(i,mode,tmp);
					}
				}
			}
			else // initializing with the default midi.cfg values.
			{
				sampler->SetZxxMacro(0,1,0x00);
				sampler->SetZxxMacro(1,1,0x08);
				sampler->SetZxxMacro(2,1,0x10);
				sampler->SetZxxMacro(3,1,0x18);
				sampler->SetZxxMacro(4,1,0x20);
				sampler->SetZxxMacro(5,1,0x28);
				sampler->SetZxxMacro(6,1,0x30);
				sampler->SetZxxMacro(7,1,0x38);
				sampler->SetZxxMacro(8,1,0x40);
				sampler->SetZxxMacro(9,1,0x48);
				sampler->SetZxxMacro(10,1,0x50);
				sampler->SetZxxMacro(11,1,0x58);
				sampler->SetZxxMacro(12,1,0x60);
				sampler->SetZxxMacro(13,1,0x68);
				sampler->SetZxxMacro(14,1,0x70);
				sampler->SetZxxMacro(15,1,0x78);
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
				if (itFileH.ffv < 0x200 ) LoadOldITInst(sampler,i);
				else LoadITInst(sampler,i);
			}
			for (i=0;i<itFileH.sampNum;i++)
			{
				Seek(pointerss[i]);
				LoadITSample(sampler,i);
			}
			int numchans=0;
			for (i=0;i<itFileH.patNum;i++)
			{
				if (pointersp[i]==0)
				{
					Pattern* pat = new Pattern();
					pat->setName("unnamed");
					pat->setID(i);
					s->sequence().Add(pat);
				} else {
					Seek(pointersp[i]);
					LoadITPattern(i,numchans);
				}
			}
			song->setTracks(std::max(numchans+1,4));

			// now that we have loaded all the patterns, time to prepare them.
			double pos = 0;
			std::vector<int>::iterator it = seq_list.begin();
			for(; it < seq_list.end(); ++it) {
				Pattern* pat = song->sequence().FindPattern(*it);			
				if (pat) {
					line->createEntry(pat, pos);
					pos += pat->beats();
				}
			}

			delete[] pointersi;
			delete[] pointerss;
			delete[] pointersp;
			song->SetReady(true);
			return true;
		}

		bool ITModule2::LoadOldITInst(XMSampler *sampler,int iInstIdx)
		{
			itInsHeader1x curH;
			XMInstrument &xins = s->rInstrument(iInstIdx);
			Read(curH);

			std::string itname(curH.sName);
			xins.Name(itname);

			xins.VolumeFadeSpeed(curH.fadeout/512.0f);

			xins.NNA((XMInstrument::NewNoteAction::Type)curH.NNA);
			if ( curH.DNC )
			{	
				xins.DCT(XMInstrument::DCType::DCT_NOTE);
				xins.DCA(XMInstrument::NewNoteAction::STOP);
			}
			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=short(curH.notes[i].first);
				npair.second=short(curH.notes[i].second)-1;
				s->rInstrument(iInstIdx).NoteToSample(i,npair);
			}
			xins.AmpEnvelope()->Init();
			if(curH.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.AmpEnvelope()->IsEnabled(true);

				if(curH.flg& EnvFlags::USE_SUSTAIN){
					xins.AmpEnvelope()->SustainBegin(curH.sustainS);
					xins.AmpEnvelope()->SustainEnd(curH.sustainE);
				}

				if(curH.flg & EnvFlags::USE_LOOP){
					xins.AmpEnvelope()->LoopStart(curH.loopS);
					xins.AmpEnvelope()->LoopEnd(curH.loopE);
				}

/*
			// load IT 1xx volume envelope
				_mm_read_UBYTES (ih.volenv, 200, modreader);
				for (lp = 0; lp < ITENVCNT; lp++)
				{
					ih.oldvoltick[lp] = _mm_read_UBYTE (modreader);
					ih.volnode[lp] = _mm_read_UBYTE (modreader);
				}
				if (ih.volflg & 1)
				{
				for (u = 0; u < ITENVCNT; u++)
					{
						if (ih.oldvoltick[d->volpts] != 0xff)
						{
							d->volenv[d->volpts].val = (ih.volnode[d->volpts] << 2);
							d->volenv[d->volpts].pos = ih.oldvoltick[d->volpts];
							d->volpts++;
						}
						else break;
					}
				}
 */

			}
			xins.PanEnvelope()->Init();
			xins.PitchEnvelope()->Init();
			xins.FilterEnvelope()->Init();
			xins.IsEnabled(true);
			return true;
		}
		bool ITModule2::LoadITInst(XMSampler *sampler,int iInstIdx)
		{
			itInsHeader2x curH;
			XMInstrument &xins = s->rInstrument(iInstIdx);

            Read(curH);
			std::string itname(curH.sName);
			xins.Name(itname);

			xins.NNA((XMInstrument::NewNoteAction::Type)curH.NNA);
			xins.DCT((XMInstrument::DCType::Type)curH.DCT);
			switch (curH.DCA)
			{
			case 1:xins.DCA(XMInstrument::NewNoteAction::NOTEOFF);break;
			case 2:xins.DCA(XMInstrument::NewNoteAction::FADEOUT);break;
			case 0:
			default:xins.DCA(XMInstrument::NewNoteAction::STOP);break;
			}
			xins.DCA((XMInstrument::NewNoteAction::Type)curH.DCA);

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
				xins.FilterType(dsp::F_LOWPASS12);
				int fc = curH.inFC&0x7F;
				xins.FilterCutoff(fc);
			}
			if ((curH.inFR&0x80) != 0)
			{
					xins.FilterType(dsp::F_LOWPASS12);
				xins.FilterResonance(curH.inFR&0x7F);
			}


			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=curH.notes[i].first;
				npair.second=curH.notes[i].second-1;
				s->rInstrument(iInstIdx).NoteToSample(i,npair);
			}

			// volume envelope
			xins.AmpEnvelope()->Init();
			
			if(curH.volEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.AmpEnvelope()->IsEnabled(true);
				if(curH.volEnv.flg& EnvFlags::ENABLE_CARRY) xins.AmpEnvelope()->IsCarry(true);
				if(curH.volEnv.flg& EnvFlags::USE_SUSTAIN){
					xins.AmpEnvelope()->SustainBegin(curH.volEnv.sustainS);
					xins.AmpEnvelope()->SustainEnd(curH.volEnv.sustainE);
				}

				if(curH.volEnv.flg & EnvFlags::USE_LOOP){
					xins.AmpEnvelope()->LoopStart(curH.volEnv.loopS);
					xins.AmpEnvelope()->LoopEnd(curH.volEnv.loopE);
				}

				int envelope_point_num = curH.volEnv.numP;
				if(envelope_point_num > 25){
					envelope_point_num = 25;
				}

				for(int i = 0; i < envelope_point_num;i++){
					short envtmp = curH.volEnv.nodes[i].secondlo | (curH.volEnv.nodes[i].secondhi <<8);
					xins.AmpEnvelope()->Append(envtmp ,(float)curH.volEnv.nodes[i].first/ 64.0f);
				}

			} else {
				xins.AmpEnvelope()->IsEnabled(false);
			}

			// Pan envelope
			xins.PanEnvelope()->Init();

			if(curH.panEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.PanEnvelope()->IsEnabled(true);
				if(curH.panEnv.flg& EnvFlags::ENABLE_CARRY) xins.PanEnvelope()->IsCarry(true);
				if(curH.panEnv.flg& EnvFlags::USE_SUSTAIN){
					xins.PanEnvelope()->SustainBegin(curH.panEnv.sustainS);
					xins.PanEnvelope()->SustainEnd(curH.panEnv.sustainE);
				}

				if(curH.panEnv.flg & EnvFlags::USE_LOOP){
					xins.PanEnvelope()->LoopStart(curH.panEnv.loopS);
					xins.PanEnvelope()->LoopEnd(curH.panEnv.loopE);
				}

				int envelope_point_num = curH.panEnv.numP;
				if(envelope_point_num > 25){ // Max number of envelope points in Impulse format is 25.
					envelope_point_num = 25;
				}

				for(int i = 0; i < envelope_point_num;i++){
					short pantmp = curH.panEnv.nodes[i].secondlo | (curH.panEnv.nodes[i].secondhi <<8);
					xins.PanEnvelope()->Append(pantmp,(float)(curH.panEnv.nodes[i].first)/ 32.0f);
				}

			} else {
				xins.PanEnvelope()->IsEnabled(false);
			}
			// Pitch/Filter envelope
			xins.PitchEnvelope()->Init();
			xins.FilterEnvelope()->Init();

			if(curH.pitchEnv.flg & EnvFlags::USE_ENVELOPE){// enable pitch/filter envelope
				int envelope_point_num = curH.pitchEnv.numP;
				if(envelope_point_num > 25){ // Max number of envelope points in Impulse format is 25.
					envelope_point_num = 25;
				}

				if (curH.pitchEnv.flg & EnvFlags::ISFILTER)
				{
					xins.FilterType(dsp::F_LOWPASS12);
					xins.FilterEnvelope()->IsEnabled(true);
					xins.PitchEnvelope()->IsEnabled(false);
					if(curH.pitchEnv.flg& EnvFlags::ENABLE_CARRY) xins.FilterEnvelope()->IsCarry(true);
					if(curH.pitchEnv.flg& EnvFlags::USE_SUSTAIN){
						xins.FilterEnvelope()->SustainBegin(curH.pitchEnv.sustainS);
						xins.FilterEnvelope()->SustainEnd(curH.pitchEnv.sustainE);
					}

					if(curH.pitchEnv.flg & EnvFlags::USE_LOOP){
						xins.FilterEnvelope()->LoopStart(curH.pitchEnv.loopS);
						xins.FilterEnvelope()->LoopEnd(curH.pitchEnv.loopE);
					}

					for(int i = 0; i < envelope_point_num;i++){
						short pitchtmp = curH.pitchEnv.nodes[i].secondlo | (curH.pitchEnv.nodes[i].secondhi <<8);
						xins.FilterEnvelope()->Append(pitchtmp,(float)(curH.pitchEnv.nodes[i].first+32)/ 64.0f);
					}
					if ( xins.FilterCutoff() < 127 )
					{
						xins.FilterEnvAmount((-1)*xins.FilterCutoff());
					} else { xins.FilterEnvAmount(-128); }
				} else {
					xins.PitchEnvelope()->IsEnabled(true);
					xins.FilterEnvelope()->IsEnabled(false);
					if(curH.pitchEnv.flg& EnvFlags::ENABLE_CARRY) xins.PitchEnvelope()->IsCarry(true);
					if(curH.pitchEnv.flg& EnvFlags::USE_SUSTAIN){
						xins.PitchEnvelope()->SustainBegin(curH.pitchEnv.sustainS);
						xins.PitchEnvelope()->SustainEnd(curH.pitchEnv.sustainE);
					}

					if(curH.pitchEnv.flg & EnvFlags::USE_LOOP){
						xins.PitchEnvelope()->LoopStart(curH.pitchEnv.loopS);
						xins.PitchEnvelope()->LoopEnd(curH.pitchEnv.loopE);
					}

					for(int i = 0; i < envelope_point_num;i++){
						short pitchtmp = curH.pitchEnv.nodes[i].secondlo | (curH.pitchEnv.nodes[i].secondhi <<8);
						xins.PitchEnvelope()->Append(pitchtmp,(float)(curH.pitchEnv.nodes[i].first)/ 32.0f);
					}
				}
			} else {
				xins.PitchEnvelope()->IsEnabled(false);
				xins.FilterEnvelope()->IsEnabled(false);
			}

			xins.IsEnabled(true);
			return true;
		}

		bool ITModule2::LoadITSample(XMSampler *sampler,int iSampleIdx)
		{
			itSampleHeader curH;
			Read(curH);
			XMInstrument::WaveData& _wave = s->SampleData(iSampleIdx);

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


			// If this MOD doesn't use Instruments, we need to map the notes manually.
			if ( !(itFileH.flags & Flags::USEINSTR)) 
			{
				int i;
				XMInstrument::NotePair npair;
				npair.second=iSampleIdx;
				for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
					npair.first=i;
					s->rInstrument(iSampleIdx).NoteToSample(i,npair);
				}
				s->rInstrument(iSampleIdx).IsEnabled(true);
			}


			if (curH.flg&SampleFlags::HAS_SAMPLE)
			{
				_wave.Init();
				_wave.AllocWaveData(curH.length,bstereo);
				_wave.WaveLength(curH.length);

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

				double tune = log(double(curH.c5Speed)/8363.0f)/log(double(2));
				double maintune = floor(tune*12);
				double finetune = floor(((tune*12)-maintune)*256);

				int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
					XMInstrument::WaveData::WaveForms::SAWDOWN,
					XMInstrument::WaveData::WaveForms::SQUARE,
					XMInstrument::WaveData::WaveForms::RANDOM
				};
				_wave.WaveTune(maintune);
				_wave.WaveFineTune(finetune);
				std::string sName = curH.sName;
				_wave.WaveName(sName);
				_wave.PanEnabled(curH.dfp&0x80);
				_wave.PanFactor((curH.dfp&0x7F)/64.0f);
				_wave.VibratoAttack(curH.vibR);
				_wave.VibratoSpeed(curH.vibS);
				_wave.VibratoDepth(curH.vibD);
				_wave.VibratoType(exchwave[curH.vibT&3]);

				if (curH.length == 0)
					curH.flg &= ~SampleFlags::HAS_SAMPLE;
				else {
					Seek(curH.smpData);
					if (bcompressed) LoadITCompressedData(sampler,iSampleIdx,curH.length,b16Bit,curH.cvt);
					else LoadITSampleData(sampler,iSampleIdx,curH.length,bstereo,b16Bit,curH.cvt);
				}
			}

			return true;
		}

		bool ITModule2::LoadITSampleData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit, unsigned char convert)
		{
			XMInstrument::WaveData& _wave = s->SampleData(iSampleIdx);

			signed short wNew,wTmp;
			int offset=(convert & SampleConvert::IS_SIGNED)?0:-32768;
			int lobit=(convert & SampleConvert::IS_MOTOROLA)?8:0;
			int hibit=8-lobit;
			unsigned int j,out;

			if (b16Bit) iLen*=2;
			unsigned char * smpbuf = new unsigned char[iLen];
			//Array read raw. Later it is interpreted as little endian if needed.
			ReadArray(smpbuf,iLen);

			out=0;wNew=0;
			if (b16Bit) {
				for (j = 0; j < iLen; j+=2) {
					wTmp= ((smpbuf[j]<<lobit) | (smpbuf[j+1]<<hibit))+offset;
					wNew=(convert& SampleConvert::IS_DELTA)?wNew+wTmp:wTmp;
					*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew ^65535;
					out++;
				}
				if (bstereo) {
					ReadArray(smpbuf,iLen);
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
					ReadArray(smpbuf,iLen);
					for (j = 0; j < iLen; j++) {
						wNew=(convert& SampleConvert::IS_DELTA)?wNew+smpbuf[j]:smpbuf[j];
						*(const_cast<signed short*>(_wave.pWaveDataR()) + j) = ((wNew<<8)+offset) ^65535;
					}
				}
			}
			delete [] smpbuf; smpbuf = 0;
			return true;
		}

		bool ITModule2::LoadITCompressedData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool b16Bit,unsigned char convert)
		{
			unsigned char bitwidth,packsize,maxbitsize;
			unsigned long topsize, val,j;
			short d1, d2,wNew;
			char d18,d28;

			bool deltapack=(itFileH.ffv>=0x215 && convert&SampleConvert::IS_DELTA); // Impulse Tracker 2.15 added a delta packed compressed sample format
			
			if (b16Bit)	{
				topsize=0x4000;		packsize=4;	maxbitsize=16;
			} else {
				topsize=0x8000;		packsize=3;	maxbitsize=8;
			}
			XMInstrument::WaveData& _wave = s->SampleData(iSampleIdx);
			
			j=0;
			while(j<iLen) // While we haven't decompressed the whole sample
			{	
				BitsBlock block;
				if (!block.ReadBlock(this)) return false;

				// Size of the block of data to process, in blocks of max size=0x8000bytes ( 0x4000 samples if 16bits)
				int blocksize=(iLen-j<topsize)?iLen-j:topsize;
				long blockpos=0;

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

			PatternEvent pempty;
			pempty.setNote(notecommands::empty);
			pempty.setMachine(255);
			pempty.setInstrument(255);
			pempty.setCommand(0);
			pempty.setParameter(0);
			PatternEvent pent=pempty;

			Skip(2); // packedSize
			std::int16_t rowCount=ReadInt16();
			Skip(4); // unused
			if (rowCount > MAX_LINES ) rowCount=MAX_LINES;
			Pattern* pat = new Pattern();
			pat->setName("unnamed");
			pat->setID(patIdx);
			s->sequence().Add(pat);
			//char* packedpattern = new char[packedSize];
			//Read(packedpattern, packedSize);
			for (int row=0;row<rowCount;row++)
			{
				Read(newEntry);
				while ( newEntry )
				{
					unsigned char channel=(newEntry-1)&0x3F;
					if (newEntry&0x80) mask[channel]=ReadUInt8();
					if(mask[channel]&1)
					{
						unsigned char note=ReadUInt8();
						if (note==255) pent.setNote(notecommands::release);
						else if (note==254) pent.setNote(notecommands::release); //\todo: Attention ! Psycle doesn't have a note-cut note.
						else pent.setNote(note);
						pent.setMachine(0);
						lastnote[channel]=pent.note();
					}
					if (mask[channel]&2)
					{
						pent.setInstrument(ReadUInt8()-1);
						pent.setMachine(0);
						lastinst[channel]=pent.instrument();
					}
					if (mask[channel]&4 || mask[channel]&0x40)
					{
						unsigned char tmp;
						pent.setMachine(0);
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
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN || PSYCLE__CONFIGURATION__USE_PSYCORE
						if ( tmp<=64)
						{
							pent.setVolume(tmp<64?tmp:63);
						}
						else if (tmp<75)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_FINEVOLSLIDEUP | (tmp-65));
						}
						else if (tmp<85)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_FINEVOLSLIDEDOWN | (tmp-75));
						}
						else if (tmp<95)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_VOLSLIDEUP | (tmp-85));
						}
						else if (tmp<105)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_VOLSLIDEDOWN | (tmp-95));
						}
						else if (tmp<115)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_PITCH_SLIDE_DOWN | (tmp-105));
						}
						else if (tmp<125)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_PITCH_SLIDE_UP | (tmp-115));
						}
						else if (tmp<193)
						{
							tmp= (tmp==192)?15:(tmp-128)/4;
							pent.setVolume(XMSampler::CMD_VOL::VOL_PANNING | tmp);
						}
						else if (tmp<203)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_TONEPORTAMENTO | (tmp-193));
						}
						else if (tmp<213)
						{
							pent.setVolume(XMSampler::CMD_VOL::VOL_VIBRATO | ( tmp-203 ));
						}
	#else
						if ( tmp<=64)
						{
							pent.setCommand(0x0C);
							pent.setParameter(tmp*2);
						}
	#endif
#endif
					}
					if(mask[channel]&8)
					{
						pent.setMachine(0);
						std::uint8_t command=ReadUInt8();
						std::uint8_t param=ReadUInt8();
						if ( command != 0 ) pent.setParameter(param);
						ParseEffect(pent,command,param,channel);
						lastcom[channel]=pent.command();
						lasteff[channel]=pent.parameter();

					}
					if (mask[channel]&0x10) { pent.setNote(lastnote[channel]); pent.setMachine(0); }
					if (mask[channel]&0x20) { pent.setInstrument(lastinst[channel]); pent.setMachine(0); }
					if ( mask[channel]&0x80 )
					{
						pent.setCommand(lastcom[channel]);
						pent.setParameter(lasteff[channel]);
					}

					pent.set_track(channel);
					double beat = row / static_cast<float>(sampler->Speed2LPB(itFileH.iSpeed));
					if (!pent.empty())
						pat->insert(beat,pent);
					pent=pempty;
					numchans = std::max(static_cast<int>(channel),numchans);
					Read(newEntry);
				}
			}
			return true;
		}

		void ITModule2::ParseEffect(PatternEvent&pent, int command,int param,int channel)
		{
			int exchwave[4]={XMInstrument::WaveData::WaveForms::SINUS,
				XMInstrument::WaveData::WaveForms::SAWDOWN,
				XMInstrument::WaveData::WaveForms::SQUARE,
				XMInstrument::WaveData::WaveForms::RANDOM
			};
			switch(command){
				case ITModule2::CMD::SET_SPEED:
					pent.setCommand(PatternCmd::EXTENDED);
					pent.setParameter(24 / ((param == 0)?6:param));
					break;
				case ITModule2::CMD::JUMP_TO_ORDER:
					pent.setCommand(PatternCmd::JUMP_TO_ORDER);
					break;
				case ITModule2::CMD::BREAK_TO_ROW:
					pent.setCommand(PatternCmd::BREAK_TO_LINE);
					break;
				case ITModule2::CMD::VOLUME_SLIDE:
					pent.setCommand(XMSampler::CMD::VOLUMESLIDE);
					break;
				case ITModule2::CMD::PORTAMENTO_DOWN:
					pent.setCommand(XMSampler::CMD::PORTAMENTO_DOWN);
					break;
				case ITModule2::CMD::PORTAMENTO_UP:
					pent.setCommand(XMSampler::CMD::PORTAMENTO_UP);
					break;
				case ITModule2::CMD::TONE_PORTAMENTO:
					pent.setCommand(XMSampler::CMD::PORTA2NOTE);
					break;
				case ITModule2::CMD::VIBRATO:
					pent.setCommand(XMSampler::CMD::VIBRATO);
					break;
				case ITModule2::CMD::TREMOR:
					pent.setCommand(XMSampler::CMD::TREMOR);
					break;
				case ITModule2::CMD::ARPEGGIO:
					pent.setCommand(XMSampler::CMD::ARPEGGIO);
					break;
				case ITModule2::CMD::VOLSLIDE_VIBRATO:
					pent.setCommand(XMSampler::CMD::VIBRATOVOL);
					break;
				case ITModule2::CMD::VOLSLIDE_TONEPORTA:
					pent.setCommand(XMSampler::CMD::TONEPORTAVOL);
					break;
				case CMD::SET_CHANNEL_VOLUME: // IT
					pent.setCommand(XMSampler::CMD::SET_CHANNEL_VOLUME);
					break;
				case CMD::CHANNEL_VOLUME_SLIDE: // IT
					pent.setCommand(XMSampler::CMD::CHANNEL_VOLUME_SLIDE);
					break;
				case CMD::SET_SAMPLE_OFFSET:
					pent.setCommand(XMSampler::CMD::OFFSET | highOffset[channel]);
					break;
				case ITModule2::CMD::PANNING_SLIDE: // IT
					pent.setCommand(XMSampler::CMD::PANNINGSLIDE);
					break;
				case ITModule2::CMD::RETRIGGER_NOTE:
					pent.setCommand(XMSampler::CMD::RETRIG);
					break;
				case ITModule2::CMD::TREMOLO:
					pent.setCommand(XMSampler::CMD::TREMOLO);
					break;
				case ITModule2::CMD::S:
					switch(param & 0xf0){
						case CMD_S::S_SET_FILTER:
							pent.setCommand(XMSampler::CMD::NONE);
							break;
						case CMD_S::S_SET_GLISSANDO_CONTROL:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_GLISSANDO_TYPE | (param & 0xf));
							break;
						case CMD_S::S_FINETUNE:
							pent.setCommand(XMSampler::CMD::NONE);
							break;
						case CMD_S::S_SET_VIBRATO_WAVEFORM:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_VIBRATO_WAVE | exchwave[(param & 0x3)]);
							break;
						case CMD_S::S_SET_TREMOLO_WAVEFORM:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_TREMOLO_WAVE | exchwave[(param & 0x3)]);
							break;
						case CMD_S::S_SET_PANBRELLO_WAVEFORM: // IT
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_PANBRELLO_WAVE | exchwave[(param & 0x3)]);
							break;
						case CMD_S::S_FINE_PATTERN_DELAY: // IT
							break;
						case CMD_S::S7: // IT
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::EE | (param&0x0F));
							break;
						case CMD_S::S_SET_PAN:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_SET_PAN | (param & 0xf));
							break;
						case CMD_S::S9: // IT
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E9 | (param&0x0F));
							break;
						case CMD_S::S_SET_HIGH_OFFSET: // IT
							highOffset[channel] = param &0x0F;
							break;
						case CMD_S::S_PATTERN_LOOP:
							pent.setCommand(PatternCmd::EXTENDED);
							pent.setParameter(PatternCmd::PATTERN_LOOP | (param & 0xf));
							break;
						case CMD_S::S_DELAYED_NOTE_CUT:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_DELAYED_NOTECUT  | (param & 0xf));
							break;
						case CMD_S::S_NOTE_DELAY:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							pent.setParameter(XMSampler::CMD_E::E_NOTE_DELAY | ( param & 0xf));
							break;
						case CMD_S::S_PATTERN_DELAY:
							pent.setCommand(PatternCmd::EXTENDED);
							pent.setParameter(PatternCmd::PATTERN_DELAY | (param & 0xf));
							break;
						case CMD_S::S_SET_MIDI_MACRO:
							pent.setCommand(XMSampler::CMD::EXTENDED);
							if ( embeddedData)
							{
								//\todo: SFx is never initialized. I need to check why I did it this way also. Leaving the defaults for now.
							//	pent.setParameter(XMSampler::CMD_E::E_SET_MIDI_MACRO | (embeddedData->SFx[(param & 0xf)][5]-48));
								pent.setParameter(XMSampler::CMD_E::E_SET_MIDI_MACRO | (param & 0xf));
							}
							break;
					}
					break;
				case CMD::SET_SONG_TEMPO:
					pent.setCommand(PatternCmd::SET_TEMPO);
					break;
				case CMD::FINE_VIBRATO:
					pent.setCommand(XMSampler::CMD::FINE_VIBRATO);
					break;
				case CMD::SET_GLOBAL_VOLUME: 
					pent.setCommand(XMSampler::CMD::SET_GLOBAL_VOLUME);
					break;
				case CMD::GLOBAL_VOLUME_SLIDE: // IT
					pent.setCommand(XMSampler::CMD::GLOBAL_VOLUME_SLIDE);
					break;
				case CMD::SET_PANNING: // IT
					pent.setCommand(XMSampler::CMD::PANNING);
					break;
				case CMD::PANBRELLO: // IT
					pent.setCommand(XMSampler::CMD::PANBRELLO);
					break;
				case CMD::MIDI_MACRO:
					if ( param < 127)
					{
						pent.setParameter(param);
					}
					pent.setCommand(XMSampler::CMD::MIDI_MACRO);
					break;
				default:
					pent.setCommand(XMSampler::CMD::NONE);
					break;
			}
		}
		bool ITModule2::ReadHeader(itHeader& header) {
			Read(header.tag);
			ReadArray(header.songName,sizeof(header.songName));
			Read(header.pHiligt);
			Read(header.ordNum);
			Read(header.insNum);
			Read(header.sampNum);
			Read(header.patNum);
			Read(header.trackerV);
			Read(header.ffv);
			Read(header.flags);
			Read(header.special);
			Read(header.gVol);
			Read(header.mVol);
			Read(header.iSpeed);
			Read(header.iTempo);
			Read(header.panSep);
			Read(header.PWD);
			Read(header.msgLen);
			Read(header.msgOffset);
			Read(header.reserved);
			ReadArray(header.chanPan, sizeof(header.chanPan));
			return ReadArray(header.chanVol, sizeof(header.chanVol));
		}
		bool ITModule2::ReadHeader(EmbeddedMIDIData& header) {
			ReadArray(header.Start, sizeof(header.Start));
			ReadArray(header.Stop, sizeof(header.Stop));
			ReadArray(header.Tick, sizeof(header.Tick));
			ReadArray(header.NoteOn, sizeof(header.NoteOn));
			ReadArray(header.NoteOff, sizeof(header.NoteOff));
			ReadArray(header.Volume, sizeof(header.Volume));
			ReadArray(header.Pan, sizeof(header.Pan));
			ReadArray(header.BankChange, sizeof(header.BankChange));
			ReadArray(header.ProgramChange, sizeof(header.ProgramChange));
			for(int i=0; i < 16; i++) ReadArray(header.SFx[i], sizeof(header.SFx[i]));
			for(int i=0; i < 128; i++) ReadArray(header.Zxx[i], sizeof(header.Zxx[i]));
			return true;
		}
		bool ITModule2::ReadHeader(itInsHeader2x& header) {
			Read(header.tag);
			ReadArray(header.fileName,sizeof(header.fileName));
			Read(header.NNA);
			Read(header.DCT);
			Read(header.DCA);
			Read(header.fadeout);
			Read(header.pPanSep);
			Read(header.pPanCenter);
			Read(header.gVol);
			Read(header.defPan);
			Read(header.randVol);
			Read(header.randPan);
			Read(header.trackerV);
			Read(header.noS);
			Read(header.unused);
			ReadArray(header.sName,sizeof(header.sName));
			Read(header.inFC);
			Read(header.inFR);
			Read(header.mChn);
			Read(header.mPrg);
			Read(header.mBnk);
			for (int i=0; i < 120; i++) Read(header.notes[i]);
			ReadHeader(header.volEnv);
			ReadHeader(header.panEnv);
			return ReadHeader(header.pitchEnv);
		}
		bool ITModule2::ReadHeader(ITEnvStruct& header) {
			Read(header.flg);
			Read(header.numP);
			Read(header.loopS);
			Read(header.loopE);
			Read(header.sustainS);
			Read(header.sustainE);
			for (int i=0; i < 25; i++) Read(header.nodes[i]);
			return Read(header.unused);
		}

		bool ITModule2::ReadHeader(ITNotePair& header) {
			Read(header.first);
			return Read(header.second);
		}

//////////////////////////////////////////////////////////////////////////
//     S3M Module Members


		bool ITModule2::LoadS3MModuleX(Song *song)
		{
			s=song;
			if (ReadHeader(s3mFileH)==0 ) return 0;
			if (s3mFileH.tag != SCRM_ID || s3mFileH.type != 0x10 ) return 0;

			s3mFileH.songName[28]='\0';
			s->setName(s3mFileH.songName);
			s->setAuthor("");
			std::string imported = "Imported from Scream Tracker 3 Module: ";
			imported.append(file_name());
			s->setComment(imported);

			song->SetReady(false);
			sampler = (XMSampler*) MachineFactory::getInstance().CreateMachine(InternalKeys::sampulse);
			s->AddMachine(sampler);
			s->InsertConnection(*sampler,*s->machine(MASTER_INDEX),0,0,(s3mFileH.mVol&0x7F)/128.0f);
			s->seqBus=sampler->id();



			song->BeatsPerMin(s3mFileH.iTempo);
			song->LinesPerBeat(sampler->Speed2LPB(s3mFileH.iSpeed));
			sampler->IsAmigaSlides(true);
			sampler->GlobalVolume((s3mFileH.gVol&0x7F)*2);
			
			song->sequence().removeAll();
			// here we add in one single Line the patterns
			SequenceLine* line = song->sequence().createNewLine();
			std::vector<int> seq_list;
			int i=0;
			for (int j=0;j<s3mFileH.ordNum ;j++)
			{
				seq_list.push_back(ReadUInt8());
/*				s->playOrder[i]=ReadUInt8(); // 254 = ++ (skip), 255 = --- (end of tune).
				if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;*/
			}

/*			int j,i=0;
			for (j=0;j<s3mFileH.ordNum;j++)
			{
				s->playOrder[i]=ReadUInt8(); // 254 = ++ (skip), 255 = --- (end of tune).
				if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;
			}
			s->playLength=i;
			if ( s->playLength == 0) // Add at least one pattern to the sequence.
			{
				s->playLength = 1;
				s->playOrder[0]=0;
			}`*/

			unsigned short *pointersi = new unsigned short[s3mFileH.insNum];
			ReadArray(pointersi,s3mFileH.insNum);
			unsigned short * pointersp = new unsigned short[s3mFileH.patNum];
			ReadArray(pointersp,s3mFileH.patNum);

			bool stereo=s3mFileH.mVol&0x80;
			int numchans=0;
			for (i=0;i<32;i++)
			{
				if (stereo)
				{
					if (s3mFileH.chanSet[i]&S3MChanType::ISRIGHTCHAN)
						sampler->rChannel(i).DefaultPanFactorFloat(0.75f,true);
					else if ( !(s3mFileH.chanSet[i]&S3MChanType::ISADLIBCHAN))
						sampler->rChannel(i).DefaultPanFactorFloat(0.25f,true);
					else 
						sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
				}
				else
					sampler->rChannel(i).DefaultPanFactorFloat(0.5f,true);
				if ( s3mFileH.chanSet[i]!=S3MChanType::ISUNUSED) 
				{
					numchans=i+1; // topmost used channel.
					if((s3mFileH.chanSet[i]&S3MChanType::ISDISABLED))
					{
						sampler->rChannel(i).DefaultIsMute(true);
					}
				}
				else
				{
					sampler->rChannel(i).DefaultIsMute(true);
				}
			}
			s->setTracks(std::max(numchans,4));

			unsigned char chansettings[32];
			if ( s3mFileH.defPan==0xFC )
			{
				ReadArray(chansettings,32);
				if (stereo)
				{
					for (i=0;i<32;i++)
					{
						if (chansettings[i]&S3MChanType::HASCUSTOMPOS)
						{
							float flttmp=(chansettings[i]&0x0F)/15.0f;
							sampler->rChannel(i).DefaultPanFactorFloat(flttmp,true);
						}
					}
				}
			}

			for (i=0;i<s3mFileH.insNum;i++)
			{
				Seek(pointersi[i]<<4);
				LoadS3MInstX(sampler,i);
			}
			for (i=0;i<s3mFileH.patNum;i++)
			{
				Seek(pointersp[i]<<4);
				LoadS3MPatternX(i);
			}

			// now that we have loaded all the patterns, time to prepare them.
			double pos = 0;
			std::vector<int>::iterator it = seq_list.begin();
			for(; it < seq_list.end(); ++it) {
				Pattern* pat = song->sequence().FindPattern(*it);			
				if (pat) {
					line->createEntry(pat, pos);
					pos += pat->beats();
				}
			}

			delete [] pointersi; pointersi = 0;
			delete [] pointersp; pointersp = 0;
			song->SetReady(true);
			return true;
		}

		bool ITModule2::LoadS3MInstX(XMSampler *sampler,int iInstIdx)
		{
			s3mInstHeader curH;
			ReadHeader(curH);

			s->rInstrument(iInstIdx).Name(curH.sName);

			int i;
			XMInstrument::NotePair npair;
			npair.second=iInstIdx;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=i;
				s->rInstrument(iInstIdx).NoteToSample(i,npair);
			}
			
			if ( curH.type == 1) 
			{
				s->rInstrument(iInstIdx).IsEnabled(true);
				return LoadS3MSampleX(sampler,reinterpret_cast<s3mSampleHeader*>(&curH),iInstIdx,iInstIdx);
			}
			else if ( curH.type != 0)
			{
/*
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

		bool ITModule2::LoadS3MSampleX(XMSampler *sampler,s3mSampleHeader *currHeader,int iInstIdx,int iSampleIdx)
		{

			XMInstrument::WaveData& _wave = s->SampleData(iSampleIdx);
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

			double tune = log(double(currHeader->c2speed)/8363.0f)/log(double(2));
			double maintune = floor(tune*12);
			double finetune = floor(((tune*12)-maintune)*256);

			_wave.WaveTune(maintune);
			_wave.WaveFineTune(finetune);

			std::string sName = currHeader->sName;
			_wave.WaveName(sName);

/*|4|
			+-+In your MOD loader, when loading in the fine tune value, convert it to
			C2SPD.  You do this by looking up what finetune matches what C2SPD.
			i.e.  Like this. (this is a switch statement I use that is fed the value
			read from the file, and returns the C2SPD to match it)

			The value is a signed char, ranging from -8 to 7 , so actually,  8= -8 , 9 = -7 ....

			case  0 : return 8363; 0
			case  1 : return 8413; 1
			case  2 : return 8463; 2
			case  3 : return 8529; 3
			case  4 : return 8581; 4
			case  5 : return 8651; 5
			case  6 : return 8723; 6
			case  7 : return 8757; 7 
			case  8 : return 7895; -8
			case  9 : return 7941; -7
			case 10 : return 7985; -6
			case 11 : return 8046; -5
			case 12 : return 8107; -4
			case 13 : return 8169; -3
			case 14 : return 8232; -2
			case 15 : return 8280; -1
			default : return 8363; 0

			In Scream Tracker, the command S2x can use these values, but they are shifted ( S20 = -8 , S21 = -7 .. S2F =7)
*/
			int newpos=((currHeader->hiMemSeg<<16)+currHeader->lomemSeg)<<4;
			Seek(newpos);
			LoadS3MSampleDataX(sampler,iInstIdx,iSampleIdx,currHeader->length,bstereo,b16Bit,currHeader->packed);

			return true;
		}

		bool ITModule2::LoadS3MSampleDataX(XMSampler *sampler,int iInstIdx,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit,bool packed)
		{
			if (!packed) // Looks like the packed format never existed.
			{
				XMInstrument::WaveData& _wave =  s->SampleData(iSampleIdx);
				char * smpbuf;
				if(b16Bit)
				{
					smpbuf = new char[iLen*2];
					ReadArray(smpbuf,iLen*2);
				} else 	{
					smpbuf = new char[iLen];
					ReadArray(smpbuf,iLen);
				}
				std::int16_t wNew;
				std::int16_t offset;
				if ( s3mFileH.trackerInf==1) offset=0; // 1=[VERY OLD] signed samples, 2=unsigned samples
				else offset=-32768;

				if(b16Bit) {
					int out=0;
					if (bstereo) { // \todo : The storage of stereo samples needs to be checked.
						_wave.AllocWaveData(iLen,true);
						unsigned int j;
						for(j=0;j<iLen*2;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)+offset;
							*(const_cast<std::int16_t*>(_wave.pWaveDataL()) + out) = wNew;
						}
						out=0;
						ReadArray(smpbuf,iLen*2);
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
							*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
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
							*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew; //| char(rand()); // Add dither;
						}
						ReadArray(smpbuf,iLen);
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataR()) + j) = wNew; //| char(rand()); // Add dither;
						}
					} else {
						_wave.AllocWaveData(iLen,false);
						unsigned int j;
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew; //| char(rand()); // Add dither;
						}
					}
				}

				// cleanup
				delete [] smpbuf; smpbuf = 0;
				return true;
			}
			return false;
		}

		bool ITModule2::LoadS3MPatternX(int patIdx)
		{
			unsigned char newEntry;
			PatternEvent pempty;
			pempty.setNote(notecommands::empty); pempty.setMachine(255);pempty.setInstrument(255);pempty.setCommand(0);pempty.setParameter(0);
			PatternEvent pent=pempty;

			Skip(2);//int packedSize=ReadInt(2);
			Pattern* pat = new Pattern();
			pat->setName("unnamed");
			pat->setID(patIdx);
			s->sequence().Add(pat);
//			char* packedpattern = new char[packedsize];
//			Read(packedpattern,packedsize);
			for (int row=0;row<64;row++)
			{
				Read(newEntry);
				while ( newEntry )
				{
					char channel=newEntry&31;
					if(newEntry&32)
					{
						std::uint8_t note=ReadUInt8();  // hi=oct, lo=note, 255=empty note,	254=key off
						if (note==254) pent.setNote(notecommands::release);
						else if (note==255) pent.setNote(255);
						else pent.setNote(((note/16)*12+(note%16)+12));  // +12 since ST3 C-4 is Psycle's C-5
						pent.setInstrument(ReadUInt8()-1);
						pent.setMachine(0);
					}
					if(newEntry&64)
					{
						std::uint8_t tmp=ReadUInt8();
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN || PSYCLE__CONFIGURATION__USE_PSYCORE
						if ( tmp<=64)
						{
							pent.setMachine(0);
							pent.setVolume((tmp<64)?tmp:63);
						}
	#else
						if ( tmp<=64)
						{
							pent.setMachine(0);
							pent.setCommand(0x0C);
							pent.setParameter(tmp*2);
						}
	#endif
#endif

					}
					if(newEntry&128)
					{
						pent.setMachine(0);
						std::uint8_t command=ReadUInt8();
						std::uint8_t param=ReadUInt8();
						if ( command != 0 ) pent.setParameter(param);
						ParseEffect(pent,command,param,channel);
						if ( pent.command() == PatternCmd::BREAK_TO_LINE )
						{
							pent.setParameter(((pent.parameter()&0xF0)>>4)*10 + (pent.parameter()&0x0F));
						}
						else if ( pent.command() == XMSampler::CMD::SET_GLOBAL_VOLUME )
						{
							pent.setParameter((pent.parameter()<0x40)?pent.parameter()*2:0x80);
						}
						else if ( pent.command() == XMSampler::CMD::PANNING )
						{
							if ( pent.parameter() < 0x80) pent.setParameter(pent.parameter()*2);
							else if ( pent.parameter() == 0x80) pent.setParameter(255);
							else if ( pent.parameter() == 0xA4)
							{
								pent.setCommand(XMSampler::CMD::EXTENDED);
								pent.setParameter(XMSampler::CMD_E::E9 | 1);
							}
						}
					}
					pent.set_track(channel);
					double beat = row / static_cast<float>(sampler->Speed2LPB(s3mFileH.iSpeed));
					if (!pent.empty())
						pat->insert(beat, pent);
					pent=pempty;
					Read(newEntry);
				}
			}
			return true;
		}
		bool ITModule2::ReadHeader(s3mHeader& header) {
			ReadArray(header.songName,sizeof(header.songName));
			Read(header.end);
			Read(header.type);
			Read(header.unused1);
			Read(header.ordNum);
			Read(header.insNum);
			Read(header.patNum);
			Read(header.flags);
			Read(header.trackerV);
			Read(header.trackerInf);
			Read(header.tag);
			Read(header.gVol);
			Read(header.iSpeed);
			Read(header.iTempo);
			Read(header.mVol);
			Read(header.uClick);
			Read(header.defPan);
			ReadArray(header.unused2,sizeof(header.unused2));
			Read(header.pSpecial);
			return ReadArray(header.chanSet, sizeof(header.chanSet));
		}
		bool ITModule2::ReadHeader(s3mInstHeader& header) {
			Read(header.type);
			ReadArray(header.fileName,sizeof(header.fileName));
			ReadArray(header.data, sizeof(header.data));
			ReadArray(header.sName,sizeof(header.sName));
			return Read(header.tag);
		}

	}
}

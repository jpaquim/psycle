#include "project.private.hpp"
#include ".\itmodule2.h"
#include "Configuration.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "XMSampler.hpp"

namespace psycle
{
	namespace host
	{
		ITModule2::ITModule2(void)
		{
		}

		ITModule2::~ITModule2(void)
		{
		}
		bool ITModule2::BitsBlock::ReadBlock(OldPsyFile *pFile)
		{
			int size;
			size=pFile->ReadInt(2);  // block layout : word size, <size> bytes data

			pdata = new unsigned char[size];
			if (!pdata)
				return false;
			if (!pFile->Read(pdata,size))
			{
				zapArray(pdata);
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

		bool ITModule2::LoadITModule(Song *song)
		{
			s=song;
			if (Read(&itFileH,sizeof(itFileH))==0 ) return 0;
			if (itFileH.tag != IMPM_ID ) return 0;

			strcpy(s->Name,itFileH.songName);
			strcpy(s->Author,"");
			strcpy(s->Comment,"Imported from Impulse Tracker Module: ");
			strcat(s->Comment,szName.c_str());

			s->CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, _T(""),0);
			s->InsertConnection(0,MASTER_INDEX,itFileH.gVol/128.0f);
			s->seqBus=0;
			XMSampler* sampler = ((XMSampler*)s->_pMachine[0]);

			sampler->BPM(itFileH.iTempo);
			if(itFileH.iSpeed == 0){
				sampler->TicksPerRow(6);
			} else {
				sampler->TicksPerRow(itFileH.iSpeed);
			}
			sampler->CalcBPMAndTick();
			sampler->IsLinearFreq(itFileH.flags&Flags::LINEARSLIDES?true:false);
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
			int numchans=0;
			for (i=0;i<64;i++)
			{
				if (stereo)
				{
					if (itFileH.chanPan[i]==ChanFlags::IS_SURROUND )
					{
						sampler->rChannel(i).IsSurround(true);
						sampler->rChannel(i).DefaultPanFactor(32);
					}
					else sampler->rChannel(i).DefaultPanFactor(itFileH.chanPan[i]&0x7F);

				}
				else
					sampler->rChannel(i).DefaultPanFactor(32);
				
				if ( !(itFileH.chanPan[i]&ChanFlags::IS_DISABLED) ) 
				{
					numchans=i+1; // topmost used channel.
				}
				else
				{
					; //  Disable channel.
				}
				sampler->rChannel(i).DefaultVolume(itFileH.chanVol[i]);
			}
			song->SONGTRACKS=numchans;


			i=0;
			for (j=0;j<itFileH.ordNum && i<MAX_SONG_POSITIONS;j++)
			{
				s->playOrder[i]=ReadInt(1); // 254 = ++ (skip), 255 = --- (end of tune).
				if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;
			}
			while (j<itFileH.ordNum)
			{
				char tmp=ReadInt(1);
				j++;
			}

			s->playLength=i;
			if ( s->playLength == 0) // Add at least one pattern to the sequence.
			{
				s->playLength = 1;
				s->playOrder[0]=0;
			}

			unsigned long *pointersi = new unsigned long[itFileH.insNum];
			Read(pointersi,itFileH.insNum*sizeof(unsigned long));
			unsigned long * pointerss = new unsigned long[itFileH.sampNum];
			Read(pointerss,itFileH.sampNum*sizeof(unsigned long));
			unsigned long * pointersp = new unsigned long[itFileH.patNum];
			Read(pointersp,itFileH.patNum*sizeof(unsigned long));



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
			for (i=0;i<itFileH.patNum;i++)
			{
				if (pointersp[i]==0)
				{
					//Create/use an empty pattern.
				} else {
					Seek(pointersp[i]);
					LoadITPattern(i);
				}
			}
			zapArray(pointersi);
			zapArray(pointerss);
			zapArray(pointersp);
			
			return true;
		}

		bool ITModule2::LoadOldITInst(XMSampler *sampler,int iInstIdx)
		{
			itInsHeader1x curH;
			XMInstrument &xins = sampler->rInstrument(iInstIdx);
			Read(&curH,sizeof(curH));

			std::string itname(curH.sName);
			xins.Name(itname);

			xins.VolumeFadeSpeed(curH.fadeout*64);

			xins.NNA(XMInstrument::NewNoteAction(curH.NNA));
			if ( curH.DNC )
			{	
				xins.DCT(XMInstrument::DCType::DCT_NOTE);
				xins.DCA(XMInstrument::DCAction::DCA_STOP);
			}
			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=short(curH.notes[i].first);
				npair.second=short(curH.notes[i].second)-1;
				sampler->rInstrument(iInstIdx).NoteToSample(i,npair);
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
*
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
				if (ih.oldvoltick[d->volpts] != 0xff)
				{
				d->volenv[d->volpts].val = (ih.volnode[d->volpts] << 2);
				d->volenv[d->volpts].pos = ih.oldvoltick[d->volpts];
				d->volpts++;
				}
				else
				break;
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
			XMInstrument &xins = sampler->rInstrument(iInstIdx);

            Read(&curH,sizeof(curH));

			if (!(curH.defPan&128)) xins.Pan(curH.defPan);
			xins.NNA(XMInstrument::NewNoteAction(curH.NNA));
			xins.DCT(XMInstrument::DCType(curH.DCT));
			xins.DCA(XMInstrument::DCAction(curH.DCA));

			xins.Pan((curH.defPan & 0x7F)/64);
			xins.PanEnabled((curH.defPan & 0x80)?false:true);
			xins.PitchPanCenter(curH.pPanCenter);
			xins.PitchPanSep(curH.pPanSep);
			xins.GlobVol(curH.gVol/127.0f);
			xins.VolumeFadeSpeed(curH.fadeout*32);
			xins.RandomVolume(curH.randVol);
			xins.RandomPanning(curH.randPan);
			xins.FilterCutoff(curH.inFC);
			xins.FilterResonance(curH.inFR);

			// MIDI

			XMInstrument::NotePair npair;
			int i;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=curH.notes[i].first;
				npair.second=curH.notes[i].second-1;
				sampler->rInstrument(iInstIdx).NoteToSample(i,npair);
			}
//////////////////////////////////////////////////////////////////////////

			// volume envelope
			xins.AmpEnvelope()->Init();
			
			if(curH.volEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.AmpEnvelope()->IsEnabled(true);

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
					short envtmp = curH.volEnv.nodes[i].secondlo | curH.volEnv.nodes[i].secondhi <<8;
					xins.AmpEnvelope()->Append(envtmp ,(float)curH.volEnv.nodes[i].first/ 64.0f);
				}

			} else {
				xins.AmpEnvelope()->IsEnabled(false);
			}

			// volume envelope
			xins.PanEnvelope()->Init();

			if(curH.panEnv.flg & EnvFlags::USE_ENVELOPE){// enable volume envelope
				xins.PanEnvelope()->IsEnabled(true);

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
					short pantmp = curH.panEnv.nodes[i].secondlo || curH.panEnv.nodes[i].secondhi <<8;
					xins.PanEnvelope()->Append(pantmp,(float)(curH.panEnv.nodes[i].first+32)/ 64.0f);
				}

			} else {
				xins.PanEnvelope()->IsEnabled(false);
			}
			xins.PitchEnvelope()->Init();
			xins.FilterEnvelope()->Init();
//////////////////////////////////////////////////////////////////////////
			xins.IsEnabled(true);

			return true;
		}
		bool ITModule2::LoadITSample(XMSampler *sampler,int iSampleIdx)
		{
			itSampleHeader curH;
			Read(&curH,sizeof(curH));
//\TODO: THIS NEEDS TO BE CHANGED!!!
			XMInstrument::WaveData& _wave = sampler->SampleData(iSampleIdx);

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
					sampler->rInstrument(iSampleIdx).NoteToSample(i,npair);
				}
				sampler->rInstrument(iSampleIdx).IsEnabled(true);
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

				_wave.WaveVolume(curH.vol * 4);
				_wave.WaveGlobVolume(curH.gVol /64.0f);

				double tune = log(double(curH.c5Speed)/8363.0f)/log(double(2));
				double maintune = floor(tune*12);
				double finetune = floor(((tune*12)-maintune)*256);

				_wave.WaveTune(maintune);
				_wave.WaveFineTune(finetune);

				if (curH.length == 0)
					curH.flg &= ~SampleFlags::HAS_SAMPLE;
				else {
					Seek(curH.smpData);
					if (bcompressed) LoadITCompressedData(sampler,iSampleIdx,curH.length,b16Bit);
					else LoadITSampleData(sampler,iSampleIdx,curH.length,bstereo,b16Bit,curH.cvt);
				}
			}

			return true;
		}
		bool ITModule2::LoadITSampleData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit, unsigned char convert)
		{
//\TODO: THIS NEEDS TO BE CHANGED!!!
			XMInstrument::WaveData& _wave = sampler->SampleData(iSampleIdx);

			signed short wNew,wTmp;
			int offset=(convert & SampleConvert::IS_SIGNED)?0:-32768;
			int lobit=(convert & SampleConvert::IS_MOTOROLA)?8:0;
			int hibit=8-lobit;
			unsigned int j,out;

			if (b16Bit) iLen*=2;
			unsigned char * smpbuf = new unsigned char[iLen];
			Read(smpbuf,iLen);

			out=0;wNew=0;
			if (b16Bit) {
				for (j = 0; j < iLen; j+=2) {
					wTmp= ((smpbuf[j]<<lobit) | (smpbuf[j+1]<<hibit))+offset;
					wNew=(convert& SampleConvert::IS_DELTA)?wNew+wTmp:wTmp;
					*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
					out++;
				}
				if (bstereo) {
					Read(smpbuf,iLen);
					out=0;
					for (j = 0; j < iLen; j+=2) {
						wTmp= ((smpbuf[j]<<lobit) | (smpbuf[j+1]<<hibit))+offset;
						wNew=(convert& SampleConvert::IS_DELTA)?wNew+wTmp:wTmp;
						*(const_cast<signed short*>(_wave.pWaveDataR()) + out) = wNew;
						out++;
					}
				}
			} else {
				for (j = 0; j < iLen; j++) {
					wNew=(convert& SampleConvert::IS_DELTA)?wNew+smpbuf[j]:smpbuf[j];
					*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = (wNew<<8)+offset;
				}
				if (bstereo) {
					Read(smpbuf,iLen);
					for (j = 0; j < iLen; j++) {
						wNew=(convert& SampleConvert::IS_DELTA)?wNew+smpbuf[j]:smpbuf[j];
						*(const_cast<signed short*>(_wave.pWaveDataR()) + j) = (wNew<<8)+offset;
					}
				}
			}
			zapArray(smpbuf);
			return true;
		}

		bool ITModule2::LoadITCompressedData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool b16Bit)
		{
			unsigned char bitwidth,packsize,maxbitsize;
			unsigned long topsize, val,j;
			short d1, d2,wNew;
			char d18,d28;

			bool deltapack=(itFileH.ffv==0x215); // Impulse Tracker 2.15 added a delta packed compressed sample format
			
			if (b16Bit)	{
				topsize=0x4000;		packsize=4;	maxbitsize=16;
			} else {
				topsize=0x8000;		packsize=3;	maxbitsize=8;
			}
			//\TODO: THIS NEEDS TO BE CHANGED!!!
			XMInstrument::WaveData& _wave = sampler->SampleData(iSampleIdx);
			
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
						if (val == (1 << (bitwidth - 1))) { // if the value == topmost bit set.
							val = block.ReadBits(packsize) + 1;
							bitwidth = (val < bitwidth) ? val : val + 1;
							continue;
						}
					} else if (bitwidth < maxbitsize+1) { //Method 2
						unsigned short border = (((1<<maxbitsize)-1) >> (maxbitsize+1 - bitwidth)) - (maxbitsize/2);

						if ((val > border) && (val <= (border + maxbitsize))) {
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
							//(It always has to end with integration doesn't it ? ;-)
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
		bool ITModule2::LoadITPattern(int patIdx)
		{
			unsigned char newEntry;
			unsigned char lastnote[64];
			unsigned char lastinst[64];
			unsigned char lastvol[64];
			unsigned char lastcom[64];
			unsigned char lasteff[64];
			unsigned char mask[64];
			memset(lastnote,255,sizeof(char)*64);
			memset(lastinst,255,sizeof(char)*64);
			memset(lastvol,255,sizeof(char)*64);
			memset(lastcom,255,sizeof(char)*64);
			memset(lasteff,255,sizeof(char)*64);
			memset(mask,255,sizeof(char)*64);

			PatternEntry pempty;
			pempty._note=255; pempty._mach=255;pempty._inst=255;pempty._cmd=0;pempty._parameter=0;
			PatternEntry pent=pempty;

			int packedSize=ReadInt(2);
			int rowCount=ReadInt(2);
			int unused=ReadInt();
//			char* packedpattern = new char[packedSize];
//			Read(packedpattern,packedSize);
			if (rowCount > MAX_LINES ) rowCount=MAX_LINES;
			for (int row=0;row<rowCount;row++)
			{
				Read(&newEntry,1);
				while ( newEntry )
				{
					unsigned char channel=(newEntry-1)&63;
					if (newEntry&128) mask[channel]=ReadChar();
					if(mask[channel]&1)
					{
						unsigned char note=ReadChar();
						if (note==255) pent._note = 120;
						else if (note==254) pent._note=120; //\todo: Attention ! Psycle doesn't have a note-cut note.
						else pent._note = note;
						pent._mach=0;
						lastnote[channel]=pent._note;
					}
					if (mask[channel]&2)
					{
						pent._inst=ReadChar()-1;
						pent._mach=0;
						lastinst[channel]=pent._inst;
					}
					if (mask[channel]&4 || mask[channel]&64)
					{
						unsigned char tmp;
						if (mask[channel]&64 ) tmp=lastvol[channel];
						else tmp=ReadChar();
						if ( tmp<=64)
						{
							tmp=tmp*255/64;
							pent._mach=0;
							pent._cmd=0x0C;
							pent._parameter=unsigned char(tmp);
						}
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
						lastvol[channel]=tmp;
					}
					if(mask[channel]&8)
					{
						pent._mach=0;
						unsigned char command=ReadInt(1);
						unsigned char param=ReadInt(1);
						if ( command != 0 ) pent._parameter = param;
						ParseEffect(pent,command,param);
						lastcom[channel]=pent._cmd;
						lasteff[channel]=pent._parameter;

					}
					if (mask[channel]&16) { pent._note=lastnote[channel]; pent._mach=0; }
					if (mask[channel]&32) { pent._inst=lastinst[channel]; pent._mach=0; }
					if ( mask[channel]&128 )
					{
						pent._cmd = lastcom[channel];
						pent._parameter = lasteff[channel];
					}

					PatternEntry* pData = (PatternEntry*) s->_ptrackline(patIdx,channel,row);

					*pData = pent;
					pent=pempty;

					Read(&newEntry,1);
				}
			}
			return true;
		}

		void ITModule2::ParseEffect(PatternEntry&pent, int command,int param)
		{
			switch(command){
				case ITModule2::CMD::SET_SPEED:
					// This function will not always work.
					pent._cmd=Player::CMD::EXTENDED;
					pent._parameter = 24 / ((param == 0)?6:param);;
					break;
				case ITModule2::CMD::JUMP_TO_ORDER:
					pent._cmd = Player::CMD::JUMP_TO_ORDER;
					break;
				case ITModule2::CMD::BREAK_TO_ROW:
					pent._cmd = Player::CMD::BREAK_TO_LINE;
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
					pent._cmd = XMSampler::CMD::OFFSET; //\todo: + mem[thischannel].highoffset; 
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
							break;
						case CMD_S::S_SET_GLISSANDO_CONTROL:
							pent._cmd = XMSampler::CMD_E::E_GLISSANDO_TYPE;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_FINETUNE:
							pent._cmd = XMSampler::CMD_E::E_FINETUNE;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_SET_VIBRATO_WAVEFORM:
							pent._cmd = XMSampler::CMD_E::E_VIBRATO_WAVE;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_SET_TREMOLO_WAVEFORM:
							pent._cmd = XMSampler::CMD_E::E_TREMOLO_WAVE;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_SET_PANBRELLO_WAVEFORM: // IT
							break;
						case CMD_S::S_FINE_PATTERN_DELAY: // IT
							break;
						case CMD_S::S7: // IT
							break;
						case CMD_S::S_SET_PAN:
							pent._cmd = XMSampler::CMD_E::E_SET_PAN;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S9: // IT
							break;
						case CMD_S::S_SET_HIGH_OFFSET: // IT
							//\todo : memorize high offset. and use it in offset command.
							// param & 0xf;
							break;
						case CMD_S::S_PATTERN_LOOP:
							pent._cmd = Player::CMD::PATTERN_LOOP;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_DELAYED_NOTE_CUT:
							pent._cmd = XMSampler::CMD_E::E_DELAYED_NOTECUT;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_NOTE_DELAY:
							pent._cmd = XMSampler::CMD_E::E_NOTE_DELAY;
							pent._parameter = param & 0xf;
							break;
						case CMD_S::S_PATTERN_DELAY:
							pent._cmd = Player::CMD::PATTERN_DELAY;
							pent._parameter = param & 0xf;
							break;
					}
					break;
				case CMD::SET_SONG_TEMPO:
					pent._cmd = Player::CMD::SET_TEMPO;
					break;
				case CMD::FINE_VIBRATO:
					pent._cmd = XMSampler::CMD::FINE_VIBRATO;
					break;
				case CMD::SET_GLOBAL_VOLUME: 
/*					pent._cmd = Player::CMD::SET_VOLUME;
					if (param== 64)
					{	
						pent._parameter=255;
					} else pent._parameter = param * 4;
*/
					pent._cmd = XMSampler::CMD::NONE;
					break;
				case CMD::GLOBAL_VOLUME_SLIDE: // IT
					//\todo: implement when it is done.
					break;
				case CMD::SET_PANNING: // IT
					pent._cmd = XMSampler::CMD::PANNING;
					break;
				case CMD::PANBRELLO: // IT
					pent._cmd = XMSampler::CMD::PANBRELLO;
					break;
				default:
					pent._cmd = XMSampler::CMD::NONE;
					break;
			}
		}

//////////////////////////////////////////////////////////////////////////
//     S3M Module Members


		bool ITModule2::LoadS3MModuleX(Song *song)
		{
			s=song;
			if (Read(&s3mFileH,sizeof(s3mFileH))==0 ) return 0;
			if (s3mFileH.tag != SCRM_ID || s3mFileH.type != 0x10 ) return 0;

			s3mFileH.songName[28]='\0';
			strcpy(s->Name,s3mFileH.songName);
			strcpy(s->Author,"");
			strcpy(s->Comment,"Imported from Scream Tracker 3 Module: ");
			strcat(s->Comment,szName.c_str());

			s->CreateMachine(MACH_XMSAMPLER, rand()/64, rand()/80, _T(""),0);
			s->InsertConnection(0,MASTER_INDEX,s3mFileH.gVol/128.0f);
			s->seqBus=0;
			XMSampler* sampler = ((XMSampler*)s->_pMachine[0]);

			sampler->BPM(s3mFileH.iTempo);
			if(s3mFileH.iSpeed == 0){
				sampler->TicksPerRow(6);
			} else {
				sampler->TicksPerRow(s3mFileH.iSpeed);
			}
			sampler->CalcBPMAndTick();
			sampler->IsLinearFreq(false);
			
			int j,i=0;
			for (j=0;j<s3mFileH.ordNum;j++)
			{
				s->playOrder[i]=ReadInt(1); // 254 = ++ (skip), 255 = --- (end of tune).
				if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;
			}
			s->playLength=i;
			if ( s->playLength == 0) // Add at least one pattern to the sequence.
			{
				s->playLength = 1;
				s->playOrder[0]=0;
			}

			unsigned short *pointersi = new unsigned short[s3mFileH.insNum];
			Read(pointersi,s3mFileH.insNum*sizeof(unsigned short));
			unsigned short * pointersp = new unsigned short[s3mFileH.patNum];
			Read(pointersp,s3mFileH.patNum*sizeof(unsigned short));

			bool stereo=s3mFileH.mVol&0x80;
			int numchans=0;
			for (i=0;i<32;i++)
			{
				if (stereo)
				{
					if (s3mFileH.chanSet[i]&S3MChanType::ISRIGHTCHAN)
						sampler->rChannel(i).DefaultPanFactor(48);
					else if ( !(s3mFileH.chanSet[i]&S3MChanType::ISADLIBCHAN))
						sampler->rChannel(i).DefaultPanFactor(12);
					else 
						sampler->rChannel(i).DefaultPanFactor(32);
				}
				else
					sampler->rChannel(i).DefaultPanFactor(32);
				if ( !(s3mFileH.chanSet[i]&S3MChanType::ISDISABLED) && s3mFileH.chanSet[i]!=S3MChanType::ISUNUSED) 
				{
					numchans=i+1; // topmost used channel.
				}
				else
				{
					; //  Disable channel.
				}
			}
			s->SONGTRACKS=numchans;

			unsigned char chansettings[32];
			if ( s3mFileH.defPan==0xFC )
			{
				Read(chansettings,sizeof(chansettings));
				if (stereo)
				{
					for (i=0;i<32;i++)
					{
						if (chansettings[i]&S3MChanType::HASCUSTOMPOS)
						{
							float flttmp=(chansettings[i]&0x0F)/15.0f;
							sampler->rChannel(i).DefaultPanFactor(int(flttmp*64.0f));
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
			zapArray(pointersi);
			zapArray(pointersp);

			return true;
		}

		bool ITModule2::LoadS3MInstX(XMSampler *sampler,int iInstIdx)
		{
			s3mInstHeader curH;
			Read(&curH,sizeof(curH));

			sampler->rInstrument(iInstIdx).Name(curH.sName);

			int i;
			XMInstrument::NotePair npair;
			npair.second=iInstIdx;
			for(i = 0;i < XMInstrument::NOTE_MAP_SIZE;i++){
				npair.first=i;
				sampler->rInstrument(iInstIdx).NoteToSample(i,npair);
			}
			sampler->rInstrument(iInstIdx).IsEnabled(true);

			if ( curH.type == 1) return LoadS3MSampleX(sampler,reinterpret_cast<s3mSampleHeader*>(&curH),iInstIdx,0);
			else
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

			XMInstrument::WaveData& _wave = sampler->SampleData(iInstIdx);
			bool bLoop=currHeader->flags&S3MSampleFlags::LOOP;
			bool bstereo=currHeader->flags&S3MSampleFlags::STEREO;
			bool b16Bit=currHeader->flags&S3MSampleFlags::IS16BIT;

			_wave.Init();
			_wave.AllocWaveData(currHeader->length,bstereo);
			_wave.WaveLength(currHeader->length);

			_wave.WaveLoopStart(currHeader->loopb);
			_wave.WaveLoopEnd(currHeader->loope);
			if(bLoop) {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::NORMAL);
			} else {
				_wave.WaveLoopType(XMInstrument::WaveData::LoopType::DO_NOT);
			}

			_wave.WaveVolume(currHeader->vol * 4);

			double tune = log(double(currHeader->c2speed)/8363.0f)/log(double(2));
			double maintune = floor(tune*12);
			double finetune = floor(((tune*12)-maintune)*256);

			_wave.WaveTune(maintune);
			_wave.WaveFineTune(finetune);

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
			if (!packed) // Looks like the packed format has never existed.
			{
				XMInstrument::WaveData& _wave =  sampler->SampleData(iInstIdx);
				char * smpbuf = new char[iLen];
				Read(smpbuf,iLen);
				signed short wNew;
				signed short offset;
				if ( s3mFileH.trackerInf==1) offset=0; // 1=[VERY OLD] signed samples, 2=unsigned samples
				else offset=-32768;

				if(b16Bit) {
					int out=0;
					if (bstereo) { // \todo : The storage of stereo samples needs to be checked.
						unsigned int j;
						for(j=0;j<iLen;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
							out++;
						}
						out=0;
						for(j=0;j<iLen;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8) +offset;				
							*(const_cast<signed short*>(_wave.pWaveDataR()) + out) = wNew;
							out++;
						}   
					} else {
						unsigned int j;
						for(j=0;j<iLen;j+=2)
						{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew;
							out++;
						}   
					}
				} else {// 8 bit sample
					if (bstereo) {
						int out=0;
						unsigned int j;
						for(j=0;j<iLen;j+=2)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataL()) + out) = wNew; //| char(rand()); // Add dither;
							wNew = (smpbuf[j+1]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataR()) + out) = wNew; //| char(rand()); // Add dither;
							out++;
						}
					} else {
						unsigned int j;
						for(j=0;j<iLen;j++)
						{			
							wNew = (smpbuf[j]<<8)+offset;
							*(const_cast<signed short*>(_wave.pWaveDataL()) + j) = wNew; //| char(rand()); // Add dither;
						}
					}
				}

				// cleanup
				zapArray(smpbuf);
				return true;
			}
			return false;
		}

		bool ITModule2::LoadS3MPatternX(int patIdx)
		{
			unsigned char newEntry;
			PatternEntry pempty;
			pempty._note=255; pempty._mach=255;pempty._inst=255;pempty._cmd=0;pempty._parameter=0;
			PatternEntry pent=pempty;

			int packedsize=ReadInt(2);
//			char* packedpattern = new char[packedsize];
//			Read(packedpattern,packedsize);
			for (int row=0;row<64;row++)
			{
				Read(&newEntry,1);
				while ( newEntry )
				{
					char channel=newEntry&31;
					if(newEntry&32)
					{
						int note=ReadInt(1);  // hi=oct, lo=note, 255=empty note,	254=key off
						if (note==254) pent._note = 120;
						else if (note==255) pent._note=255;
						else pent._note = ((note/16)*12+(note%16)+12);  // +12 since ST3 C-4 is Psylce's C-5
						pent._inst=ReadInt(1)-1;
						pent._mach=0;
					}
					if(newEntry&64)
					{
						int tmp=ReadInt(1);
						if ( tmp<=64)
						{
							tmp=tmp*255/64;
							pent._mach=0;
							pent._cmd=0x0C;
							pent._parameter=unsigned char(tmp);

						}
					}
					if(newEntry&128)
					{
						pent._mach=0;
						unsigned char command=ReadInt(1);
						unsigned char param=ReadInt(1);
						if ( command != 0 ) pent._parameter = param;
						ParseEffect(pent,command,param);
					}
					PatternEntry* pData = (PatternEntry*) s->_ptrackline(patIdx,channel,row);

					*pData = pent;
					pent=pempty;

					Read(&newEntry,1);
				}
			}
			return true;
		}
	}
}
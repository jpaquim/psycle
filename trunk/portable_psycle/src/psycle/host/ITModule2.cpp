#include "project.private.hpp"
#include ".\itmodule2.h"
#include "Song.hpp"

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
		char * ITModule2::AllocReadStr(LONG size, LONG start)
			{
			// allocate space
			char * pData = new char[size+1];
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

			zapArray(pData);
			return NULL;
			}
		bool ITModule2::LoadITModule(Song *s)
			{
				int insnum=0,sampnum=0,patnum=0,flags=0,special=0,stereosep=0;
				int customData=0,headerpointerpos=0,pinst=0,ppat=0;
				char *TAG;
				TAG=AllocReadStr(4); // SCRM
				zapArray(TAG);
				char * pSongName=AllocReadStr(26);
				if(pSongName==NULL)
					return 0;
				strcpy(s->Name,pSongName);	
				strcpy(s->Author,"");
				strcpy(s->Comment,"Imported from Scream Tracker 3 module.");
				zapArray(pSongName);

				Skip(2); // pHighlight

				s->playLength=ReadInt(2);

				insnum=ReadInt(2);
				sampnum=ReadInt(2);
				patnum=ReadInt(2);

				Skip(2); // created with tracker
				Skip(2); // fileformat version.
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
				flags=ReadInt(2);
				special=ReadInt(2);
				Skip(1);//globvol=ReadInt(1);  // Real Master volume
				Skip(1);//mastvol=ReadInt(1);		// mix volume
				s->TicksPerBeat(int(24/ReadInt(1)));
				s->BeatsPerMin(ReadInt(1));
				//stereo=upperbits(mastvol,1);
				//mastvol=lowerbits(mastvol,7);
				Skip(1);//uclickchannels=ReadInt(1);
				Skip(8);
				customData=ReadInt(2); // pointer to some custom data.

				//		Channel settings (byte per channel):
				//		bit 8: channel enabled
				//			bit 0-7: channel type
				//			0..7   : Left Sample Channel 1-8
				//			8..15  : Right Sample Channel 1-8
				//			16..31 : Adlib channels (9 melody + 5 drums)

				Skip(32);//chansettings[32]=ReadInt(32);

				int i=0,j=0;
				for (;j<s->playLength;j++)
					{
					s->playOrder[i]=ReadInt(1); // 254 = ++ (skip), 255 = --- (end of tune).
					if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;
					}
				s->playLength=i;

				headerpointerpos=GetPos();
				for (i=0;i<insnum;i++)
					{
					pinst=ReadInt(2);
					Seek(pinst*16);
					LoadS3MInst(s,i);
					headerpointerpos+=2;
					Seek(headerpointerpos);
					}
				for (i=0;i<patnum;i++)
					{
					ppat=ReadInt(2);
					Seek(ppat*16);
					LoadS3MPattern(s,i);
					headerpointerpos+=2;
					Seek(headerpointerpos);
					}
				return true;
			}






















		bool ITModule2::LoadS3MModule(Song *s)
			{
			int insnum=0,patnum=0,flags=0,hasdefaultpan=0;
			int customData=0,headerpointerpos=0,pinst=0,ppat=0;
			char * pSongName=AllocReadStr(28);
			if(pSongName==NULL)
				return 0;
			strcpy(s->Name,pSongName);	
			strcpy(s->Author,"");
			strcpy(s->Comment,"Imported from Scream Tracker 3 module.");
			zapArray(pSongName);
				
				Skip(1); // 1AH
				Skip(1);//type=ReadInt(1);  // 16 = S3M
				Skip(2);
				s->playLength=ReadInt(2);

				insnum=ReadInt(2);
				patnum=ReadInt(2);
				flags=ReadInt(2); // 8,16,64, 128
				Skip(2);//trackerversion=ReadInt(2); // 0x1300 , 0x1301 0x1303 0x1320
				Skip(2);//fileformatinfo=ReadInt(2); // 1 signed samples (old), 2 unsigned
				char *TAG;
				TAG=AllocReadStr(4); // SCRM
				zapArray(TAG);
				Skip(1);//globvol=ReadInt(1);  // Real Master volume
				s->TicksPerBeat(int(24/ReadInt(1)));
				s->BeatsPerMin(ReadInt(1));
				Skip(1);//mastvol=ReadInt(1);		// soundblasters's volume // Note that in stereo, the mastermul is	internally multiplied by 11/8 inside the player since		there is generally more room in the output stream.
				//stereo=upperbits(mastvol,1);
				//mastvol=lowerbits(mastvol,7);
				Skip(1);//uclickchannels=ReadInt(1);
				hasdefaultpan=ReadInt(1); // 252 = have default pan
				Skip(8);
				customData=ReadInt(2); // pointer to some custom data.

		//		Channel settings (byte per channel):
		//		bit 8: channel enabled
		//			bit 0-7: channel type
		//			0..7   : Left Sample Channel 1-8
		//			8..15  : Right Sample Channel 1-8
		//			16..31 : Adlib channels (9 melody + 5 drums)

				Skip(32);//chansettings[32]=ReadInt(32);

				int i=0,j=0;
				for (;j<s->playLength;j++)
					{
					s->playOrder[i]=ReadInt(1); // 254 = ++ (skip), 255 = --- (end of tune).
					if (s->playOrder[i]!= 254 &&s->playOrder[i] != 255 ) i++;
					}
				s->playLength=i;

				headerpointerpos=GetPos();
				for (i=0;i<insnum;i++)
					{
						pinst=ReadInt(2);
						Seek(pinst*16);
						LoadS3MInst(s,i);
						headerpointerpos+=2;
						Seek(headerpointerpos);
					}
				for (i=0;i<patnum;i++)
					{
						ppat=ReadInt(2);
						Seek(ppat*16);
						LoadS3MPattern(s,i);
						headerpointerpos+=2;
						Seek(headerpointerpos);
					}
				if ( hasdefaultpan==252 )
					{
					for (i=0;i<32;i++)
						{
						}
					}
			return true;
			}
		bool ITModule2::LoadS3MInst(Song *s,int iInstIdx)
			{
			int type=ReadInt(1);
				if ( type == 1) return LoadS3MSample(s,iInstIdx,0);
				else
					{
		/*				//    [T]ype  = 2:amel 3:abd 4:asnare 5:atom 6:acym 7:ahihat
						filename=Read(12); // add null ending.
						Skip(3); // 00H
						
						Skip(12);
						volume=ReadInt(1);
						Skip(3);
						c2speed=ReadInt(4);
						Skip(12);
						samplename=Read(28); // null ended?
						TAG=Read(4); // SCRI
		*/
					}
				return false;
			}
		bool ITModule2::LoadS3MSample(Song *s,int iInstrIdx,int iSampleIdx)
			{
				char *sName=AllocReadStr(12); // add null ending.
				int pSamp=ReadInt(3)>>4;
				int iLen=ReadInt(4);
				int iLoopStart=ReadInt(4);
				int iLoopEnd=ReadInt(4); //means the first not looped point
				int iVol=ReadInt(1); // 0..64
				Skip(1);
				int packed=ReadInt(1);
				int flags=ReadInt(1);
				bool bLoop=flags&1;
				bool bstereo=flags&2;
				bool b16Bit=flags&4;

				int c2speed=ReadInt(4);
				Skip(12);
				char *sInstrName=AllocReadStr(28);
				char *TAG=AllocReadStr(4); // SCRS

				strcpy(s->_pInstrument[iInstrIdx]->_sName,sInstrName);
				
				// alloc wave memory
				if(b16Bit)
					s->WavAlloc(iInstrIdx,iSampleIdx,bstereo,iLen/2,sName);
				else
					s->WavAlloc(iInstrIdx,iSampleIdx,bstereo,iLen,sName);

				if(bLoop)
					{
					s->_pInstrument[iInstrIdx]->waveLoopType[iSampleIdx]=true;
					if(b16Bit)
						{
						s->_pInstrument[iInstrIdx]->waveLoopStart[iSampleIdx]=iLoopStart/2;
						s->_pInstrument[iInstrIdx]->waveLoopEnd[iSampleIdx]=(iLoopEnd)/2;
						}
					else
						{
						s->_pInstrument[iInstrIdx]->waveLoopStart[iSampleIdx]=iLoopStart;
						s->_pInstrument[iInstrIdx]->waveLoopEnd[iSampleIdx]=iLoopEnd;
						}
					}
				s->_pInstrument[iInstrIdx]->waveVolume[iSampleIdx]= iVol;

				double tune = log(double(c2speed)/44100)/log(double(2));
				double maintune;
				double finetune;

				maintune= floor(tune*12);
				finetune= floor(((tune*12)-maintune)*100);

				s->_pInstrument[iInstrIdx]->waveTune[iSampleIdx] = maintune;
				s->_pInstrument[iInstrIdx]->waveFinetune[iSampleIdx] = finetune;

				Seek(pSamp);
				LoadS3MSampleData(s,iInstrIdx,iSampleIdx,iLen,bstereo,b16Bit,packed);

				zapArray(sName);
				zapArray(sInstrName);
				zapArray(TAG);

				return true;
			}
		bool ITModule2::LoadS3MSampleData(Song *s,int iInstrIdx,int iSampleIdx,int iLen,bool bstereo,bool b16Bit,bool packed)
			{
			if (!packed)
				{
				char * smpbuf = new char[iLen];
				Read(smpbuf,iLen);
				signed short wNew;

				// unpack sample data
				if(b16Bit)
					{				
					int out=0;
					if (bstereo)
						{
						for(int j=0;j<iLen;j+=4)
							{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)-32768;
							s->_pInstrument[iInstrIdx]->waveDataL[iSampleIdx][out] = wNew;
							wNew = 0xFF & smpbuf[j] | smpbuf[j+1]<<8;				
							s->_pInstrument[iInstrIdx]->waveDataR[iSampleIdx][out] = wNew;
							out++;
							}   
						}
					else
						{
						for(int j=0;j<iLen;j+=2)
							{
							wNew = (0xFF & smpbuf[j] | smpbuf[j+1]<<8)-32768;				
							s->_pInstrument[iInstrIdx]->waveDataL[iSampleIdx][out] = wNew;
							out++;
							}   
						}
					}
				else
					{
					// 8 bit mono sample
					if (bstereo)
						{
						int out=0;
						for(int j=0;j<iLen;j+=2)
							{			
							wNew = (smpbuf[j]<<8)-32768;// | char(rand())); // scale + dither
							s->_pInstrument[iInstrIdx]->waveDataL[iSampleIdx][out] = wNew;
							wNew = (smpbuf[j]<<8)-32768;// | char(rand())); // scale + dither
							s->_pInstrument[iInstrIdx]->waveDataR[iSampleIdx][out] = wNew;
							out++;
							}
						}
					else
						{
						for(int j=0;j<iLen;j++)
							{			
							wNew = (smpbuf[j]<<8)-32768;// | char(rand())); // scale + dither
							s->_pInstrument[iInstrIdx]->waveDataL[iSampleIdx][j] = wNew;
							}
						}
					}

				// cleanup
				zapArray(smpbuf);
				return true;
				}
			return false;
			}


		bool ITModule2::LoadS3MPattern(Song *s,int patIdx)
			{
				int packedsize=ReadInt(2);
				packedsize=packedsize; // assing to make compiler happy
//				char* packedpattern = new char[packedsize];
//				Read(packedpattern,packedsize);
				unsigned char what;
				PatternEntry pempty;
				pempty._note=255; pempty._mach=255;pempty._inst=255;pempty._cmd=0;pempty._parameter=0;
				PatternEntry pent=pempty;
				for (int row=0;row<64;row++)
					{
					Read(&what,1);
					while ( what )
						{
						char channel=what&31;
						if(what&32)
							{
							int note=ReadInt(1);  // hi=oct, lo=note, 255=empty note,	254=key off
							if (note==254) pent._note = 120;
							else if (note==255) pent._note=255;
							else pent._note = (note/16)*12+(note%16);
							pent._inst=ReadInt(1)-1;
							pent._mach=0;
							}
						if(what&64)
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
						if(what&128)
							{
							//pent._mach=0;
							//pent._cmd=ReadInt(1);
							//pent._parameter=ReadInt(1);
							Skip(2);
							}
						PatternEntry* pData = (PatternEntry*) s->_ptrackline(patIdx,channel,row);

						*pData = pent;
						pent=pempty;

						Read(&what,1);
						}
					}
				return true;
			}
		}
	}
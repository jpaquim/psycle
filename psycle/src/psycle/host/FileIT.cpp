/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief
#include <project.private.hpp>
#include "FileIT.hpp"
#include "Song.hpp"
namespace psycle
{
	namespace host
	{
		static const char * IT_HEADER = "IMPM";

		CFileIT::CFileIT()
		{
			m_iInstrCnt=0;
		}

		CFileIT::~CFileIT()
		{

		}

		bool CFileIT::Import(Song *s)
		{	
			// check validity
			if(!IsValid())
				return false;

			// clear existing data
		//	s->DeleteAllPatterns(); // [JAZ] : we have just made a song->New()

			LONG iInstrStart = ImportPatterns(s);
			ImportInstruments(s,iInstrStart);

			return true;
		}



		bool CFileIT::IsValid()
		{			
			bool bIsValid = false;

			// check header
			bIsValid = (!stricmp(mod.header.headerID, IT_HEADER));

			return bIsValid;
		}

		long CFileIT::ImportPatterns(Song *s)
		{

			// get song name
			strcpy(s->Name, mod.header.songName);	
			strcpy(s->Author,"");
			strcpy(s->Comment,"Imported from Impulse Tracker module.");

			for(int i=0; i<MAX_SONG_POSITIONS && i<256 && i<mod.header.numOrders; i++)
			{
				if ( mod.header.orders[i] < MAX_PATTERNS ) s->playOrder[i] = mod.header.orders[i];
				else s->playOrder[i] = 0;
			}
			if ( mod.header.numOrders > MAX_PATTERNS ) s->playLength = MAX_PATTERNS;
 			else s->playLength = mod.header.numOrders;
			s->SONGTRACKS = 32;
			
			// tempo
			s->BeatsPerMin(6 * mod.header.initialTempo / mod.header.initialSpeed);

			// instr count
			m_iInstrCnt = mod.header.numIns;

			// get pattern data
			int nextPatStart = 0;
			for(int j=0;j<mod.header.numPatterns&&nextPatStart>0;j++)
				nextPatStart = ImportSinglePattern(s,j,32);

			return nextPatStart;
		}

		// import instruments
		bool CFileIT::ImportInstruments(Song *s, LONG iInstrStart)
		{	
			for(int i=1;i<=m_iInstrCnt;i++)
				iInstrStart = ImportInstrument(s,iInstrStart,i);

			return true;
		}

		char * CFileIT::AllocReadStr(LONG size, LONG start)
		{
		/*	// allocate space
			char * pData = new char[size+1];
			if(pData==NULL)
				return false;

			// null terminate
			pData[size]=0;
			
			// go to offset
			if(start>=0)
				Seek(start);

			// read data
			if(Read(pData,size))
				return pData;

			delete[] pData;
			ASSERT(0);*/
			return NULL;
		}





		// return address of next pattern, 0 for invalid
		LONG CFileIT::ImportSinglePattern(Song *s, int patIdx, int iTracks)
		{

			int iNumRows = mod.Patterns(patIdx).numRows;

			s->patternLines[patIdx] = iNumRows;

			PatternEntry e;

			int iPackedSize = 0;
			if(iPackedSize==0)
			{
				// build empty PatternEntry
				e._note=255;
				e._inst=255;
				e._mach=255;
				e._cmd=0;
				e._parameter=0;

				// build empty pattern
				for(int row=0;row<iNumRows;row++)
					for(int col=0;col<iTracks;col++)
						WritePatternEntry(s,patIdx,row,col,e);	
			}	
			else
			{
				// get next values
				for(int row=0;row<iNumRows;row++)
					for(int col=0;col<iTracks;col++)
					{	
						// reset
						unsigned char note=255;
						unsigned char instr=255;
						unsigned char vol=0;
						unsigned char type=0;
						unsigned char param=0;

						// read note 

						note = mod.Patterns(patIdx).Note(col, row);
						instr = mod.Patterns(patIdx).Instrument(col, row);
						//vol = this->Patterns(patIdx).VolPan(col, row);
						vol = 64;
						type = mod.Patterns(patIdx).Command(col, row);
						param = mod.Patterns(patIdx).CommandValue(col, row);

						// translate
						e._inst = instr;			
						e._mach = 0;				

						// volume/command
						if(vol >= 0x10 && vol <= 0x50)
						{
							// translate volume
							e._cmd = 0x0C;
							e._parameter = 4*(vol-0x10);
						}
						else
						{
							e._cmd = type;
							e._parameter =  param;					
						}									

						// instrument/note
						// note = note & 0x7f;
						switch(note)
						{
							case 0x00: 
								e._note=255;
								break;// no note

							case 0x61:
								e._note=120;
								e._inst=255;
								e._mach=0;
								break;// noteoff		
							
							default: 
								if(note>=96||note<0)
									TRACE("huh?\n");
								
								// force note into range
								if (note>=30)
									e._note = note - 30;
								else if (note>=30-12)
									e._note = note - (30-12);
								else if (note>=30-24)
									e._note = note - (30-24);
								else if (note>=30-36)
									e._note = note - (30-36);

								break;	// transpose
						}


						WritePatternEntry(s, patIdx, row, col, e);	

					}
			}

			//int z = ftell(_file);
			return 0;

		}


		BOOL CFileIT::WritePatternEntry(Song * s, int patIdx, int row, int col,PatternEntry &e)
		{
			// don't overflow song buffer 
			if(patIdx>=MAX_PATTERNS) return false;

			PatternEntry* pData = (PatternEntry*) s->_ptrackline(patIdx,col,row);

			*pData = e;

			return true;
		}	

		LONG CFileIT::ImportInstrument(Song *s, LONG iStart, int idx)
		{
			/*// read header
			Seek(iStart);
			int iInstrSize = ReadInt4();
			ASSERT(iInstrSize==0x107||iInstrSize==0x21);

			char * sInstrName = AllocReadStr(22);
			int iInstrType = ReadInt1();
			int iSampleCount = ReadInt2();

			if(iSampleCount>1)
 				TRACE("samplecount = %d\n",iSampleCount);

			// store instrument name
			strcpy(s->_instruments[idx]._sName,sInstrName);
			delete [] sInstrName;

			int iSampleHeader = ReadInt4();
			ASSERT(iSampleHeader==0x28);
			// exit if empty

			iStart += iInstrSize;
			
			if(iSampleCount==0)
				return iStart;

			// read instrument data	
			
			// load individual samples
			for(int i=0;i<iSampleCount;i++)
				iStart = ImportSampleHeader(s,iStart,idx,i);
			for(i=0;i<iSampleCount;i++)
				iStart = ImportSampleData(s,iStart,idx,i);*/

			return iStart;
		}

		LONG CFileIT::ImportSampleHeader(Song *s, LONG iStart, int iInstrIdx, int iSampleIdx)
		{/*
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
			char iPanning = ReadInt1();
			char iRelativeNote = ReadInt1();
			char iReserved = ReadInt1();	

			// sample name
			char * sName = AllocReadStr(22);
			
			// parse
			BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
			BOOL bPingPong = iFlags & 0x02;
			BOOL b16Bit = iFlags & 0x10;

			// alloc wave memory
			if(b16Bit)
				s->WavAlloc(iInstrIdx,iSampleIdx,false,iLen/2,sName);
			else
				s->WavAlloc(iInstrIdx,iSampleIdx,false,iLen,sName);
			delete[] sName;

			if(bLoop)
			{
				s->waveLoopType[iInstrIdx][iSampleIdx]=true;
				if(b16Bit)
				{
					s->waveLoopStart[iInstrIdx][iSampleIdx]=iLoopStart/2;
					s->waveLoopEnd[iInstrIdx][iSampleIdx]=(iLoopLength+iLoopStart)/2;
				}
				else
				{
					s->waveLoopStart[iInstrIdx][iSampleIdx]=iLoopStart;
					s->waveLoopEnd[iInstrIdx][iSampleIdx]=iLoopLength+iLoopStart;
				}
			}

			s->waveVolume[iInstrIdx][iSampleIdx]= iVol;
			s->waveTune[iInstrIdx][iSampleIdx] = iRelativeNote;	
			s->waveFinetune[iInstrIdx][iSampleIdx] = int((100.0*iFineTune)/128);

			smpLen[iSampleIdx] = iLen;
			smpFlags[iSampleIdx] = iFlags;
		*/
			return iStart+40;

		}

		LONG CFileIT::ImportSampleData(Song *s, LONG iStart, int iInstrIdx, int iSampleIdx)
		{/*
			// parse
			
			BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
			
			short wNew=0;

			// cache sample data
			Seek(iStart);
			char * smpbuf = new char[smpLen[iSampleIdx]];
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
					s->waveDataL[iInstrIdx][iSampleIdx][out] = wNew;
					out++;
				}   
			}
			else
			{
				// 8 bit mono sample
				for(int j=0;j<sampleCnt;j++)
				{			
					wNew += (smpbuf[j]<<8);// | char(rand())); // scale + dither
					s->waveDataL[iInstrIdx][iSampleIdx][j] = wNew;
				}
			}

			// cleanup
			delete[]smpbuf;

			// complete			
			iStart += smpLen[iSampleIdx]; */
			return iStart;
		}
	}
}
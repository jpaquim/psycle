//////////////////////////////////////////////////////////////////////
// Keyboard Handler sourcecode

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	int const km=appConfig.KEYBOARDMODE;

	int maxpoly=0;
	int const mgn=SONG->busMachine[SONG->seqBus];
	int const ps=SONG->playOrder[SONG->editPosition];
	int const nl=SONG->patternLines[ps];
	
	
	if(mgn!=255)
	maxpoly=SONG->machine[mgn]->numSubTracks;
	
	int const maxtrac=SONG->SONGTRACKS;
	
	int const displace=(ps*MULTIPLY2)+(patLine*MULTIPLY)+(SONG->patTrack*5);
	unsigned char *toffset=SONG->pPatternData+displace;

	// Octave shifting keyshortcuts handlers
	if(viewMode==2)
	{
	if(nChar==VK_MULTIPLY)ShiftOctave(1);
	if(nChar==VK_DIVIDE)ShiftOctave(-1);
	
	if(nChar==VK_F1 && ::GetKeyState(VK_CONTROL)<0)patTranspose(-1);
	if(nChar==VK_F2 && ::GetKeyState(VK_CONTROL)<0)patTranspose(1);
	
	if(nChar==VK_F3 && ::GetKeyState(VK_CONTROL)<0)patCut();
	if(nChar==VK_F4 && ::GetKeyState(VK_CONTROL)<0)patCopy();
	if(nChar==VK_F5 && ::GetKeyState(VK_CONTROL)<0)patPaste();
	if(nChar=='B'	&& ::GetKeyState(VK_CONTROL)<0)StartBlock(SONG->patTrack,patLine);
	if(nChar=='E'	&& ::GetKeyState(VK_CONTROL)<0)EndBlock(SONG->patTrack,patLine);
	if(nChar=='C'	&& ::GetKeyState(VK_CONTROL)<0)CopyBlock(false);
	if(nChar=='X'	&& ::GetKeyState(VK_CONTROL)<0)CopyBlock(true);	
	if(nChar=='V'	&& ::GetKeyState(VK_CONTROL)<0)PasteBlock(SONG->patTrack,patLine);
	
	if(nChar==VK_TAB && !(::GetKeyState(VK_SHIFT)<0))AdvanceTrack(maxtrac);
	if(nChar==VK_TAB && ::GetKeyState(VK_SHIFT)<0)PrevTrack(maxtrac);
	
	}

	if(viewMode==2 && !(::GetKeyState(VK_CONTROL)<0))
	{
		switch(patRow)
		{
		case 0: // Note Row

			outnote=-1;
			//char buf[32];sprintf(buf,"%d",nChar);MessageBox(buf);

			switch(nChar)
			{
				case 'A':
					if(km==1)
					outnote=12;
				break;

				case 'Z':
					if(km==0)
					outnote=0;
				
					if(km==1)
					outnote=14;
				break;
				
				case 'S':			outnote=1;break;
				case 'X':			outnote=2;break;
				case 'D':			outnote=3;break;
				case 'C':			outnote=4;break;
				case 'V':			outnote=5;break;
				case 'G':			outnote=6;break;
				case 'B':			outnote=7;break;
				case 'H':			outnote=8;break;
				case 'N':			outnote=9;break;
				case 'J':			outnote=10;break;
				
				case 'M':			
					if(km==0)
					outnote=11;
 
					if(km==1)
					outnote=15;
				break;				

				case 188:
					if(km==0)
					outnote=12;
				
					if(km==1)
					outnote=11;
				break;				
				
				case 190:
					if(km==0)
					outnote=14;

					if(km==1)
					outnote=12;
				break;				
							
				case 'L':
					outnote=13;
				break;

				case 191:
					if(km==1)
					outnote=14;
				break;				
				
				case 223:
					if(km==1)
					outnote=16;
				break;				
			
				case 'Q':
					if(km==0)
					outnote=12;
				break;
				case '2':			outnote=13;break;
				
				case 'W':			
					if(km==0)
					outnote=14;
				
					if(km==1)
					outnote=0;
				break;
				
				case '3':			outnote=15;break;
				case 'E':			outnote=16;break;
				case 'R':			outnote=17;break;
				case '5':			outnote=18;break;
				case 'T':			outnote=19;break;
				case '6':			outnote=20;break;
				case 'Y':			outnote=21;break;
				case '7':			outnote=22;break;
				case 'U':			outnote=23;break;
				case 'I':			outnote=24;break;
				case '9':			outnote=25;break;
				case 'O':			outnote=26;break;
				case '0':			outnote=27;break;
				case 'P':			outnote=28;break;
				case '1':			outnote=120;break;
				case 220:			outnote=121;break;
				
				case VK_DELETE: if(!(::GetKeyState(VK_CONTROL)<0))outnote=255;break;
			}// nchar
			
			if(outnote!=-1)
			{
				// Note On
				if(outnote<120)
				{
					int finalnote=outnote+SONG->currentOctave*12;
					if(finalnote>119)finalnote=119;
					*toffset=finalnote;
					*(toffset+1)=SONG->instSelected;
					*(toffset+2)=SONG->seqBus;
					
					psyGear *tmac=SONG->machine[mgn];

					// DLL Plugin Generator

					if(mgn!=255)
					{
						if(tmac->type==0 && tmac->machineMode==8)
						tmac->mi->SeqTick(SONG->patTrack,finalnote,255,0,0);

						if(tmac->type==0 && tmac->machineMode==9)
						tmac->ovst.SeqTick(SONG->patTrack,finalnote,255,0,0);

						// Sampler
						if(tmac->machineMode==3)
						{
							int fstrk=SONG->machine[mgn]->SubTrack[SONG->patTrack];
					
							if(SONG->machine[mgn]->trackObj[SONG->patTrack][fstrk].Tick(finalnote,SONG->instSelected,0,0)==TR_TRIGGERED)
							{
							SONG->machine[mgn]->IsolateSubTracks(SONG->patTrack,fstrk,SONG->instSelected);
							if(++SONG->machine[mgn]->SubTrack[SONG->patTrack]>=maxpoly)
							SONG->machine[mgn]->SubTrack[SONG->patTrack]=0;
							}
						}
					}
				}
				// Note Off
				if(outnote==120)
				{	
					*toffset=120;
					*(toffset+1)=255;
					*(toffset+2)=SONG->seqBus;
				}

				// Tweak Form
				if(outnote==121)
				{	
					*toffset=121;
					*(toffset+1)=255;
					*(toffset+2)=SONG->seqBus;
				}

				// Delete Note
				if(outnote==255)
				{
					*toffset=255;
					*(toffset+1)=255;
					*(toffset+2)=255;
				}
				
				if(::GetKeyState(VK_SHIFT)<0)
				AdvanceTrack(maxtrac);
				else
				AdvanceLine(patStep,true);
				
				updateMode=1;				
				Invalidate(false);
			}
		break;

		case 1: // Inst Row [MSB]
			MSBPut(nChar,toffset+1,0,true);
		break;
		
		case 2: // Inst Row [LSB]
			MSBPut(nChar,toffset+1,1,true);
		break;
		
		case 3: // Volu Row [MSB]
			MSBPut(nChar,toffset+2,0,true);
		break;
		
		case 4: // Volu Row [LSB]
			MSBPut(nChar,toffset+2,1,true);
		break;
		
		case 5: // Cmd. Row [MSB]
			MSBPut(nChar,toffset+3,0,false);
		break;
		
		case 6: // Cmd. Row [LSB]
			MSBPut(nChar,toffset+3,1,false);
		break;
		
		case 7: // Val. Row [MSB]
			MSBPut(nChar,toffset+4,0,false);
		break;
		
		case 8: // Val. Row [LSB]
			MSBPut(nChar,toffset+4,1,false);
		break;
		
		}
	}

	switch(viewMode)
	{
//////////////////////////////////////////////////////////////////////
// Pattern view
	case 2:

		switch(nChar)
		{
		case VK_RETURN:
			if(::GetKeyState(VK_CONTROL) < 0)
			{
				updateMode=0;
				ShowPatternDlg();
			}
		break;

		case VK_LEFT:
	
			if(--patRow<0)
			{
				if (--SONG->patTrack<0)SONG->patTrack=maxtrac-1;
				patRow=8;
			}
			updateMode=1;				
			Invalidate(false);
		break;

		case VK_RIGHT:
			if(++patRow>8)
			{	
				if (++SONG->patTrack==maxtrac)SONG->patTrack=0;
				patRow=0;
			}

			updateMode=1;				
			Invalidate(false);
		break;
		

		case VK_PRIOR:	if(patLine>0)PrevLine(16,false);break;
		case VK_UP:		PrevLine(1,true);break;
		case VK_DOWN:	AdvanceLine(1,true);break;
		case VK_NEXT:	AdvanceLine(16,false);break;
		
		case VK_ADD:
			if(::GetKeyState(VK_CONTROL) < 0)
			IncPosition();
			else
			IncCurPattern();

		break;
		
		case VK_SUBTRACT:
			if(::GetKeyState(VK_CONTROL) < 0)
			DecPosition();
			else
			DecCurPattern();
		break;
		
		case VK_DELETE:
		break;

		}// PATTERN VIEW
	break;

	}
}

//////////////////////////////////////////////////////////////////////
// Preview pattern cursor editor lines

void CChildView::PrevLine(int x, bool mode)
{
	int ps=SONG->playOrder[SONG->editPosition];

		int nl=SONG->patternLines[ps];
		
			patLine-=x;
			
			
			if(patLine<0)
			{
				if (mode)
				patLine=nl-1;
				else
				patLine=0;
			}

			updateMode=1;
			Invalidate(false);
}

//////////////////////////////////////////////////////////////////////
// Advance pattern cursor editor lines

void CChildView::AdvanceLine(int x,bool mode)
{
	int ps=SONG->playOrder[SONG->editPosition];;

	int nl=SONG->patternLines[ps];
	
			patLine+=x;
			

			if(patLine>=nl)
			{
				if(mode)
				{
					patLine=0;
					patOffset=0;
				}
				else
				{
					patLine=nl-1;
				}
			}

			updateMode=1;
			Invalidate(false);
		
}

//////////////////////////////////////////////////////////////////////
// Function that shift the current editing octave

void CChildView::ShiftOctave(int x)
{
if(x>0 && SONG->currentOctave<8)
{
SONG->currentOctave+=x;
if(SONG->currentOctave>8)SONG->currentOctave=8;
updateMode=4;
Invalidate(false);
}

if(x<0 && SONG->currentOctave>0)
{
SONG->currentOctave+=x;
if(SONG->currentOctave<0)SONG->currentOctave=0;
updateMode=4;
Invalidate(false);
}
}

bool CChildView::MSBPut(int nChar,unsigned char *offset,int mode,bool trflag)
{
	bool rValue=false;
	int sValue=-1;
	
	switch(nChar)
	{
	case '0':	sValue=0;	break;
	case '1':	sValue=1;	break;
	case '2':	sValue=2;	break;
	case '3':	sValue=3;	break;
	case '4':	sValue=4;	break;
	case '5':	sValue=5;	break;
	case '6':	sValue=6;	break;
	case '7':	sValue=7;	break;
	case '8':	sValue=8;	break;
	case '9':	sValue=9;	break;
	case 'A':	sValue=10;	break;
	case 'B':	sValue=11;	break;
	case 'C':	sValue=12;	break;
	case 'D':	sValue=13;	break;
	case 'E':	sValue=14;	break;
	case 'F':	sValue=15;	break;
	case VK_DELETE:	sValue=16;	break;
	
	}
	
	int oldValue=*offset;

	if(sValue>=0)
	{
		// User entered 0-9 / A-F
		if(sValue<16)
		{
			if(trflag && oldValue==255)
			oldValue=0;

			switch(mode)
			{
			case 0:
			*offset=(oldValue&0xF)+(sValue<<4);
			break;

			case 1:	
			*offset=(oldValue&0xF0)+(sValue);
			break;
			}
		}

		if(sValue==16)
		{
			// User entered DELETE
			if(trflag)
				*offset=255;
			else
				*offset=0;
		}

	AdvanceLine(1,true);
	rValue=true;
	}

	return rValue;
}

void CChildView::patCut()
{
	int ps=SONG->playOrder[SONG->editPosition];;
	int displace=ps*MULTIPLY2;

	patBufferLines=SONG->patternLines[ps];
	patBufferCopy=true;

	unsigned char *soffset=SONG->pPatternData+ps;
	unsigned char *toffset=patBufferData;
	int length=patBufferLines*5*MAX_TRACKS;

	for	(int c=0;c<length;c++)
	*toffset++=*soffset++;
	
	soffset=SONG->pPatternData+displace;
	
	for	(c=0;c<length;c+=5)
	{
	*soffset=255;
	*(soffset+1)=255;
	*(soffset+2)=255;
	*(soffset+3)=0;
	*(soffset+4)=0;
	soffset+=5;
	}

	updateMode=1;
	Invalidate(false);
}

void CChildView::patCopy()
{
	int ps=SONG->playOrder[SONG->editPosition];;
	int displace=ps*MULTIPLY2;

	patBufferLines=SONG->patternLines[ps];
	patBufferCopy=true;

	unsigned char *soffset=SONG->pPatternData+displace;
	unsigned char *toffset=patBufferData;
	int length=patBufferLines*5*MAX_TRACKS;

	for	(int c=0;c<length;c++)
	*toffset++=*soffset++;

}

void CChildView::patPaste()
{
	int ps=SONG->playOrder[SONG->editPosition];;
	int displace=ps*MULTIPLY2;

	if(patBufferCopy && viewMode==2)
	{
	unsigned char *soffset=patBufferData;
	unsigned char *toffset=SONG->pPatternData+displace;
	SONG->patternLines[ps]=patBufferLines;

	int tpatBufferLines=patBufferLines;

	int length=tpatBufferLines*5*MAX_TRACKS;
	for	(int c=0;c<length;c++)
	*toffset++=*soffset++;
	
	Repaint();
	}
}

void CChildView::patTranspose(int trp)
{
	int ps=SONG->playOrder[SONG->editPosition];;
	int displace=ps*MULTIPLY2;

	if(viewMode==2)
	{
	unsigned char *toffset=SONG->pPatternData+displace;
	int pLines=SONG->patternLines[ps];
	
	int length=pLines*5*MAX_TRACKS;
	
	for	(int c=patLine*5*MAX_TRACKS;c<length;c+=5)
	{
		int not=*(toffset+c);

		if(not<120)
		{
			not+=trp;
			if(not<0)not=0;
			if(not>119)not=119;
			*(toffset+c)=unsigned char(not);
		}
	}

	updateMode=1;
	Invalidate(false);
	}
}

CChildView::StartBlock(int track,int line)
{
	blockSelected=true;
	blockTrackStart=track;
	blockLineStart=line;
	
	if(blockLineEnd<blockLineStart)
	blockLineEnd=blockLineStart;

	if(blockTrackEnd<blockTrackStart)
	blockTrackEnd=blockTrackStart;

	updateMode=1;
	Invalidate(false);
}

CChildView::EndBlock(int track,int line)
{
	blockSelected=true;
	blockTrackEnd=track;
	blockLineEnd=line;
	
	if(blockLineStart>blockLineEnd)
	blockLineStart=blockLineEnd;

	if(blockTrackStart>blockTrackEnd)
	blockTrackStart=blockTrackEnd;

	updateMode=1;
	Invalidate(false);
}

void CChildView::AdvanceTrack(int nt)
{
	SONG->patTrack++;
	patRow=0;

	if(SONG->patTrack>=nt)
	SONG->patTrack=0;

	updateMode=1;
	Invalidate(false);
}

void CChildView::PrevTrack(int nt)
{
	SONG->patTrack--;
	patRow=0;

	if(SONG->patTrack<0)
	SONG->patTrack=nt-1;

	updateMode=1;
	Invalidate(false);
}

CChildView::CopyBlock(bool cutit)
{
	if(blockSelected)
	{
		isBlockCopied=true;
		blockNTracks=(blockTrackEnd-blockTrackStart)+1;
		blockNLines=(blockLineEnd-blockLineStart)+1;

		int ps=SONG->playOrder[SONG->editPosition];;
		int displace=ps*MULTIPLY2;

		int ls=0;
		int ts=0;

		for (int t=blockTrackStart;t<blockTrackEnd+1;t++)
		{
			ls=0;
			for (int l=blockLineStart;l<blockLineEnd+1;l++)
			{
				int const displace2=t*5+l*MULTIPLY;
				int const displace3=ts*5+ls*MULTIPLY;
				
				unsigned char *offset_source=SONG->pPatternData+displace+displace2;				
				unsigned char *offset_target=blockBufferData+displace3;				
				
				*(offset_target)	=*offset_source;
				*(offset_target+1)	=*(offset_source+1);
				*(offset_target+2)	=*(offset_source+2);
				*(offset_target+3)	=*(offset_source+3);
				*(offset_target+4)	=*(offset_source+4);

				if(cutit)
				{
				*(offset_source)	=255;
				*(offset_source+1)	=255;
				*(offset_source+2)	=255;
				*(offset_source+3)	=0;
				*(offset_source+4)	=0;
				}

			++ls;
			}
		++ts;
		}
	}

	if(cutit)
	{
		blockSelected=false;
		updateMode=1;
		Invalidate(false);
	}
}

CChildView::PasteBlock(int tx,int lx)
{
	if(isBlockCopied)
	{
	int ps=SONG->playOrder[SONG->editPosition];;
	int displace=ps*MULTIPLY2;
		
	int ls=0;
	int ts=0;

	for (int t=tx;t<tx+blockNTracks;t++)
	{
		ls=0;
		for (int l=lx;l<lx+blockNLines;l++)
			{
				if(l<MAX_LINES && t<MAX_TRACKS)
				{
					int const displace2=t*5+l*MULTIPLY;
					int const displace3=ts*5+ls*MULTIPLY;
					unsigned char* offset_source=blockBufferData+displace3;
					unsigned char* offset_target=SONG->pPatternData+displace+displace2;
					*(offset_target)=*offset_source;
					*(offset_target+1)=*(offset_source+1);
					*(offset_target+2)=*(offset_source+2);
					*(offset_target+3)=*(offset_source+3);
					*(offset_target+4)=*(offset_source+4);
				}

			++ls;
			}
		++ts;
		}
	
	updateMode=1;
	Invalidate(false);
	}
	
}

CChildView::IncCurPattern()
{
	if(SONG->playOrder[SONG->editPosition]<(MAX_PATTERNS-1))
	{
	++SONG->playOrder[SONG->editPosition];
	pParentMain->UpdatePlayOrder(true);
	Repaint();
	}
}


CChildView::DecCurPattern()
{
	if(SONG->playOrder[SONG->editPosition]>0)
	{
	--SONG->playOrder[SONG->editPosition];
	pParentMain->UpdatePlayOrder(true);
	Repaint();
	}
}

CChildView::DecPosition()
{
	if(SONG->editPosition>0)
	{
		--SONG->editPosition;
		pParentMain->UpdatePlayOrder(false);
		Repaint();
	}
}

CChildView::IncPosition()
{
	if(SONG->editPosition<(MAX_SONG_POSITIONS-1))
	{
		++SONG->editPosition;
		pParentMain->UpdatePlayOrder(false);
		Repaint();
	}
}

#include "ctrack.cpp"

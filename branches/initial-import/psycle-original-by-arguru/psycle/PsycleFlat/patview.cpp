void CChildView::DrawPatEditor(CPaintDC *devc)
{
psyGear *tmpmac;

CRect rClient;
GetClientRect(&rClient);
int CW=rClient.Width();
int CH=rClient.Height();
int DEPTHROWS=CH/13;
int WIDEROWS=CW/111;
int yOffset=0;
int c=0;
int snt=SONG->SONGTRACKS;

DEPTHROWS-=3;
if(DEPTHROWS<2)DEPTHROWS=2;

if(--WIDEROWS<2)WIDEROWS=2;

int cpat=SONG->playOrder[SONG->editPosition];
int plines=SONG->patternLines[cpat];

int bIndex=-1;
int numDTracks=SONG->SONGTRACKS;

bIndex=SONG->busMachine[SONG->seqBus];

if(bIndex!=255)
tmpmac=SONG->machine[bIndex];

if(updateMode==5 && SONG->playPosition!=SONG->editPosition)return;

//////////////////////////////////////////////////////////////////////
// Draw pattern editor status bar

if(updateMode==4 || updateMode==0)
{
devc->SetBkColor(0x00CCCCCC);
char buffer[256];

sprintf(buffer," Pattern Editor [%.2X: %s]   Octave: %d",cpat,SONG->patternName[cpat],SONG->currentOctave);

TXT(devc,buffer,0,0,CW,18,false);
}

//////////////////////////////////////////////////////////////////////
// Draw pattern editor lines

devc->SelectObject(&seqFont);

if(updateMode==0)
{
	devc->MoveTo(0,18);
	devc->LineTo(CW,18);
	devc->MoveTo(0,36);
	devc->LineTo(CW,36);
	devc->MoveTo(0,35);
	devc->LineTo(CW,35);
	devc->SetBkColor(0x00DDDDDD);
	BOX(devc,-1,18,44,18);
	TXT(devc,"Line",0,19,42,16,false);
	yOffset=36;

	int xo=43;
	
	BOX(devc,42,18,1,CW);
	for(int t=0;t<WIDEROWS+1;t++)
	{
	BOX(devc,xo,18,1,CW);
	xo+=111;
	}
}

if(numDTracks)
{
yOffset=37;
int flimit=plines-DEPTHROWS;
if(flimit<0)flimit=0;

patOffset=patLine-(DEPTHROWS/2);
if(patOffset<0)patOffset=0;
if(patOffset>flimit)patOffset=flimit;
int tpatOffset=patOffset;
unsigned char *tdata=SONG->pPatternData+cpat*MULTIPLY2;

int to=SONG->patTrack-(WIDEROWS/2);

if(to>snt-WIDEROWS)
to=snt-WIDEROWS;
if(to<0)to=0;

///////////////////////////////////////////////////////////////////////
// Draw Track Labels

CDC memDC;
memDC.CreateCompatibleDC(devc);
memDC.SelectObject(&stuffbmp);

if(updateMode!=5)
{
	int xOffset=44;
	devc->SetBkColor(0x00BCBDC4);
	
	for(int t=to;t<to+WIDEROWS;t++)
	{
		if(t<snt)
		{	
		int const displace=t/10;
		int const displace2=t-(displace*10);

		// BLIT [DESTX,DESTY,SIZEX,SIZEY,SOURBMPX,SOURBMPY)

		devc->BitBlt(xOffset,19,109,16,&memDC,148,65,SRCCOPY);
		devc->BitBlt(xOffset+34,21,7,12,&memDC,148+displace*7,81,SRCCOPY);
		devc->BitBlt(xOffset+41,21,7,12,&memDC,148+displace2*7,81,SRCCOPY);
		
		if(!SONG->track_st[t])
		devc->BitBlt(xOffset+70,23,9,9,&memDC,257,48,SRCCOPY);
	
		xOffset+=111;	
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Draw Pattern data

for(c=0;c<DEPTHROWS;c++)
{
	int	n=0;

	if(tpatOffset%16)n-=0x00110102;
	if(tpatOffset%4)n-=0x00120102;

	if(SONG->PlayMode && tpatOffset==SONG->LineCounter && SONG->playPosition==SONG->editPosition)
	n-=0x00112343;

	devc->SetBkColor(0x00C0C0C0+n);

	if(c<plines)
	{
	char tBuf[32];
	sprintf(tBuf,"%d",tpatOffset);
	TXT(devc,tBuf,0,yOffset,41,12,true);
	
		int xOffset=44;
			
		if(updateMode!=5)
		{
		for(int t=to;t<to+WIDEROWS;t++)
		{
			if(t<snt)
			{
			int	n=0;
		
			
			if(t%2)n-=0x00111111;
			if(tpatOffset%16)n-=0x00110102;
			if(tpatOffset%4)n-=0x00120102;

			if(!SONG->track_st[t])n-=0x00081828;

			//if(SONG->PlayMode && tpatOffset==SONG->LineCounter && SONG->playPosition==SONG->editPosition)
			//n-=0x00051020;

			if(blockSelected 
				&& tpatOffset>=blockLineStart
				&& tpatOffset<=blockLineEnd
				&& t>=blockTrackStart
				&& t<=blockTrackEnd)
			{

				devc->SetBkColor(0x00CCCC20+n);

			}
			else
			{
				devc->SetBkColor(0x00CCCCCC+n);
			}

			unsigned char *poffset=(tdata+tpatOffset*MULTIPLY+t*5);

			OutNote(devc,xOffset,yOffset,*poffset);
			OutData(devc,xOffset+29,yOffset,*++poffset,true);
			OutData(devc,xOffset+50,yOffset,*++poffset,true);
			OutData(devc,xOffset+71,yOffset,*++poffset,false);
			OutData(devc,xOffset+90,yOffset,*++poffset,false);
			
			xOffset+=111;
			}
		}// Cierra for bucle
		}// Condicion updatemode!=5
	}
	
	++tpatOffset;
	yOffset+=13;
}

//////////////////////////////////////////////////////////////////////
// Draw pattern cursor

if(updateMode!=5)
{		
	devc->SetROP2(R2_NOT);
	int xRow=44+(SONG->patTrack-to)*111;
	int yRow=37+((patLine-patOffset)*13);

	int const nsrx=28;
	int const nsry=12;
	int const csrx=10;
	int const csry=12;

	switch (patRow)
	{
	case 0:
	// Note
	BOX(devc,xRow,yRow,nsrx,nsry);
	break;

	case 1:
	// Instrument [Row 1]
	BOX(devc,xRow+29,yRow,csrx,csry);
	break;

	case 2:
	// Instrument [Row 2]
	BOX(devc,xRow+39,yRow,csrx,csry);
	break;

	case 3:
	// Volumen [Row 1]
	BOX(devc,xRow+50,yRow,csrx,csry);
	break;

	case 4:
	// Volumen [Row 2]
	BOX(devc,xRow+60,yRow,csrx,csry);
	break;

	case 5:
	// Effect Row [Row 1]
	BOX(devc,xRow+71,yRow,csrx,csry);
	break;

	case 6:
	// Effect Row [Row 2]
	BOX(devc,xRow+81,yRow,csrx,csry);
	break;

	case 7:
	// Arg Row [Row 1]
	BOX(devc,xRow+90,yRow,csrx,csry);
	break;

	case 8:
	// Arg Row [Row 2]
	BOX(devc,xRow+100,yRow,csrx,csry);
	break;
	}

	devc->SetROP2(R2_BLACK);
	}

}// Exist the pattern

}

//////////////////////////////////////////////////////////////////////
// Pattern data display functions

void CChildView::OutData(CPaintDC *devc,int x,int y,unsigned char data, bool trflag)
{
	int const srx=10;
	int const sry=12;

	if (trflag && data==255)
	{
		TXT(devc,".",x,y,srx,sry,true);
		TXT(devc,".",x+srx,y,srx,sry,true);
		return;
	}
	int left=data>>4;
	int right=data&0xf;

	switch(left)
	{
	case 0x0: TXT(devc,"0",x,y,srx,sry,true);break;
	case 0x1: TXT(devc,"1",x,y,srx,sry,true);break;
	case 0x2: TXT(devc,"2",x,y,srx,sry,true);break;
	case 0x3: TXT(devc,"3",x,y,srx,sry,true);break;
	case 0x4: TXT(devc,"4",x,y,srx,sry,true);break;
	case 0x5: TXT(devc,"5",x,y,srx,sry,true);break;
	case 0x6: TXT(devc,"6",x,y,srx,sry,true);break;
	case 0x7: TXT(devc,"7",x,y,srx,sry,true);break;
	case 0x8: TXT(devc,"8",x,y,srx,sry,true);break;
	case 0x9: TXT(devc,"9",x,y,srx,sry,true);break;
	case 0xA: TXT(devc,"A",x,y,srx,sry,true);break;
	case 0xB: TXT(devc,"B",x,y,srx,sry,true);break;
	case 0xC: TXT(devc,"C",x,y,srx,sry,true);break;
	case 0xD: TXT(devc,"D",x,y,srx,sry,true);break;
	case 0xE: TXT(devc,"E",x,y,srx,sry,true);break;
	case 0xF: TXT(devc,"F",x,y,srx,sry,true);break;
	}

	x+=srx;

	switch(right)
	{
	case 0x0: TXT(devc,"0",x,y,srx,sry,true);break;
	case 0x1: TXT(devc,"1",x,y,srx,sry,true);break;
	case 0x2: TXT(devc,"2",x,y,srx,sry,true);break;
	case 0x3: TXT(devc,"3",x,y,srx,sry,true);break;
	case 0x4: TXT(devc,"4",x,y,srx,sry,true);break;
	case 0x5: TXT(devc,"5",x,y,srx,sry,true);break;
	case 0x6: TXT(devc,"6",x,y,srx,sry,true);break;
	case 0x7: TXT(devc,"7",x,y,srx,sry,true);break;
	case 0x8: TXT(devc,"8",x,y,srx,sry,true);break;
	case 0x9: TXT(devc,"9",x,y,srx,sry,true);break;
	case 0xA: TXT(devc,"A",x,y,srx,sry,true);break;
	case 0xB: TXT(devc,"B",x,y,srx,sry,true);break;
	case 0xC: TXT(devc,"C",x,y,srx,sry,true);break;
	case 0xD: TXT(devc,"D",x,y,srx,sry,true);break;
	case 0xE: TXT(devc,"E",x,y,srx,sry,true);break;
	case 0xF: TXT(devc,"F",x,y,srx,sry,true);break;
	}
}

void CChildView::OutNote(CPaintDC *devc,int x,int y,int note)
{
int const srx=28;
int const sry=12;

switch(note)
{
case 255:TXT(devc,"---",x,y,srx,sry,true);break;
case 0:   TXT(devc,"C-0",x,y,srx,sry,true);break;
case 1:   TXT(devc,"C#0",x,y,srx,sry,true);break;
case 2:   TXT(devc,"D-0",x,y,srx,sry,true);break;
case 3:   TXT(devc,"D#0",x,y,srx,sry,true);break;
case 4:   TXT(devc,"E-0",x,y,srx,sry,true);break;
case 5:   TXT(devc,"F-0",x,y,srx,sry,true);break;
case 6:   TXT(devc,"F#0",x,y,srx,sry,true);break;
case 7:   TXT(devc,"G-0",x,y,srx,sry,true);break;
case 8:   TXT(devc,"G#0",x,y,srx,sry,true);break;
case 9:   TXT(devc,"A-0",x,y,srx,sry,true);break;
case 10:  TXT(devc,"A#0",x,y,srx,sry,true);break;
case 11:  TXT(devc,"B-0",x,y,srx,sry,true);break;
case 12:  TXT(devc,"C-1",x,y,srx,sry,true);break;
case 13:  TXT(devc,"C#1",x,y,srx,sry,true);break;
case 14:  TXT(devc,"D-1",x,y,srx,sry,true);break;
case 15:  TXT(devc,"D#1",x,y,srx,sry,true);break;
case 16:  TXT(devc,"E-1",x,y,srx,sry,true);break;
case 17:  TXT(devc,"F-1",x,y,srx,sry,true);break;
case 18:  TXT(devc,"F#1",x,y,srx,sry,true);break;
case 19:  TXT(devc,"G-1",x,y,srx,sry,true);break;
case 20:  TXT(devc,"G#1",x,y,srx,sry,true);break;
case 21:  TXT(devc,"A-1",x,y,srx,sry,true);break;
case 22:  TXT(devc,"A#1",x,y,srx,sry,true);break;
case 23:  TXT(devc,"B-1",x,y,srx,sry,true);break;
case 24:  TXT(devc,"C-2",x,y,srx,sry,true);break;
case 25:  TXT(devc,"C#2",x,y,srx,sry,true);break;
case 26:  TXT(devc,"D-2",x,y,srx,sry,true);break;
case 27:  TXT(devc,"D#2",x,y,srx,sry,true);break;
case 28:  TXT(devc,"E-2",x,y,srx,sry,true);break;
case 29:  TXT(devc,"F-2",x,y,srx,sry,true);break;
case 30:  TXT(devc,"F#2",x,y,srx,sry,true);break;
case 31:  TXT(devc,"G-2",x,y,srx,sry,true);break;
case 32:  TXT(devc,"G#2",x,y,srx,sry,true);break;
case 33:  TXT(devc,"A-2",x,y,srx,sry,true);break;
case 34:  TXT(devc,"A#2",x,y,srx,sry,true);break;
case 35:  TXT(devc,"B-2",x,y,srx,sry,true);break;
case 36:  TXT(devc,"C-3",x,y,srx,sry,true);break;
case 37:  TXT(devc,"C#3",x,y,srx,sry,true);break;
case 38:  TXT(devc,"D-3",x,y,srx,sry,true);break;
case 39:  TXT(devc,"D#3",x,y,srx,sry,true);break;
case 40:  TXT(devc,"E-3",x,y,srx,sry,true);break;
case 41:  TXT(devc,"F-3",x,y,srx,sry,true);break;
case 42:  TXT(devc,"F#3",x,y,srx,sry,true);break;
case 43:  TXT(devc,"G-3",x,y,srx,sry,true);break;
case 44:  TXT(devc,"G#3",x,y,srx,sry,true);break;
case 45:  TXT(devc,"A-3",x,y,srx,sry,true);break;
case 46:  TXT(devc,"A#3",x,y,srx,sry,true);break;
case 47:  TXT(devc,"B-3",x,y,srx,sry,true);break;
case 48:  TXT(devc,"C-4",x,y,srx,sry,true);break;
case 49:  TXT(devc,"C#4",x,y,srx,sry,true);break;
case 50:  TXT(devc,"D-4",x,y,srx,sry,true);break;
case 51:  TXT(devc,"D#4",x,y,srx,sry,true);break;
case 52:  TXT(devc,"E-4",x,y,srx,sry,true);break;
case 53:  TXT(devc,"F-4",x,y,srx,sry,true);break;
case 54:  TXT(devc,"F#4",x,y,srx,sry,true);break;
case 55:  TXT(devc,"G-4",x,y,srx,sry,true);break;
case 56:  TXT(devc,"G#4",x,y,srx,sry,true);break;
case 57:  TXT(devc,"A-4",x,y,srx,sry,true);break;
case 58:  TXT(devc,"A#4",x,y,srx,sry,true);break;
case 59:  TXT(devc,"B-4",x,y,srx,sry,true);break;
case 60:  TXT(devc,"C-5",x,y,srx,sry,true);break;
case 61:  TXT(devc,"C#5",x,y,srx,sry,true);break;
case 62:  TXT(devc,"D-5",x,y,srx,sry,true);break;
case 63:  TXT(devc,"D#5",x,y,srx,sry,true);break;
case 64:  TXT(devc,"E-5",x,y,srx,sry,true);break;
case 65:  TXT(devc,"F-5",x,y,srx,sry,true);break;
case 66:  TXT(devc,"F#5",x,y,srx,sry,true);break;
case 67:  TXT(devc,"G-5",x,y,srx,sry,true);break;
case 68:  TXT(devc,"G#5",x,y,srx,sry,true);break;
case 69:  TXT(devc,"A-5",x,y,srx,sry,true);break;
case 70:  TXT(devc,"A#5",x,y,srx,sry,true);break;
case 71:  TXT(devc,"B-5",x,y,srx,sry,true);break;
case 72:  TXT(devc,"C-6",x,y,srx,sry,true);break;
case 73:  TXT(devc,"C#6",x,y,srx,sry,true);break;
case 74:  TXT(devc,"D-6",x,y,srx,sry,true);break;
case 75:  TXT(devc,"D#6",x,y,srx,sry,true);break;
case 76:  TXT(devc,"E-6",x,y,srx,sry,true);break;
case 77:  TXT(devc,"F-6",x,y,srx,sry,true);break;
case 78:  TXT(devc,"F#6",x,y,srx,sry,true);break;
case 79:  TXT(devc,"G-6",x,y,srx,sry,true);break;
case 80:  TXT(devc,"G#6",x,y,srx,sry,true);break;
case 81:  TXT(devc,"A-6",x,y,srx,sry,true);break;
case 82:  TXT(devc,"A#6",x,y,srx,sry,true);break;
case 83:  TXT(devc,"B-6",x,y,srx,sry,true);break;
case 84:  TXT(devc,"C-7",x,y,srx,sry,true);break;
case 85:  TXT(devc,"C#7",x,y,srx,sry,true);break;
case 86:  TXT(devc,"D-7",x,y,srx,sry,true);break;
case 87:  TXT(devc,"D#7",x,y,srx,sry,true);break;
case 88:  TXT(devc,"E-7",x,y,srx,sry,true);break;
case 89:  TXT(devc,"F-7",x,y,srx,sry,true);break;
case 90: TXT(devc,"F#7",x,y,srx,sry,true);break;
case 91: TXT(devc,"G-7",x,y,srx,sry,true);break;
case 92: TXT(devc,"G#7",x,y,srx,sry,true);break;
case 93: TXT(devc,"A-7",x,y,srx,sry,true);break;
case 94: TXT(devc,"A#7",x,y,srx,sry,true);break;
case 95: TXT(devc,"B-7",x,y,srx,sry,true);break;
case 96: TXT(devc,"C-8",x,y,srx,sry,true);break;
case 97: TXT(devc,"C#8",x,y,srx,sry,true);break;
case 98: TXT(devc,"D-8",x,y,srx,sry,true);break;
case 99: TXT(devc,"D#8",x,y,srx,sry,true);break;
case 100: TXT(devc,"E-8",x,y,srx,sry,true);break;
case 101: TXT(devc,"F-8",x,y,srx,sry,true);break;
case 102: TXT(devc,"F#8",x,y,srx,sry,true);break;
case 103: TXT(devc,"G-8",x,y,srx,sry,true);break;
case 104: TXT(devc,"G#8",x,y,srx,sry,true);break;
case 105: TXT(devc,"A-8",x,y,srx,sry,true);break;
case 106: TXT(devc,"A#8",x,y,srx,sry,true);break;
case 107: TXT(devc,"B-8",x,y,srx,sry,true);break;
case 108: TXT(devc,"C-9",x,y,srx,sry,true);break;
case 109: TXT(devc,"C#9",x,y,srx,sry,true);break;
case 110: TXT(devc,"D-9",x,y,srx,sry,true);break;
case 111: TXT(devc,"D#9",x,y,srx,sry,true);break;
case 112: TXT(devc,"E-9",x,y,srx,sry,true);break;
case 113: TXT(devc,"F-9",x,y,srx,sry,true);break;
case 114: TXT(devc,"F#9",x,y,srx,sry,true);break;
case 115: TXT(devc,"G-9",x,y,srx,sry,true);break;
case 116: TXT(devc,"G#9",x,y,srx,sry,true);break;
case 117: TXT(devc,"A-9",x,y,srx,sry,true);break;
case 118: TXT(devc,"A#9",x,y,srx,sry,true);break;
case 119: TXT(devc,"B-9",x,y,srx,sry,true);break;
case 120: TXT(devc,"off",x,y,srx,sry,true);break;
case 121: TXT(devc,"twk",x,y,srx,sry,true);break;
}
}



void CChildView::BOX(CPaintDC *devc,int x,int y, int w, int h)
{
	CRect rect;
	rect.left=x;
	rect.top=y;
	rect.right=x+w;
	rect.bottom=y+h;

	devc->Rectangle(rect);
}

void CChildView::TXT(CPaintDC *devc,char *txt, int x,int y,int w,int h,bool mode)
{
	int *sizes;
	
	if(mode)
	sizes=FLATSIZES;
	else
	sizes=NULL;

	CRect Rect;
	Rect.left=x;
	Rect.top=y;
	Rect.right=x+w;
	Rect.bottom=y+h;

	devc->ExtTextOut(x+2,y,ETO_OPAQUE | ETO_CLIPPED ,Rect,txt,sizes);
}


			
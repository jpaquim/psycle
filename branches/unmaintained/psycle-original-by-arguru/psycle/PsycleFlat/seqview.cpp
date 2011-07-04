//////////////////////////////////////////////////////////////////////
// Seq view GDI operations

void CChildView::DrawSeqEditor(CPaintDC *devc)
{

CRect rClient;
GetClientRect(&rClient);
int CW=rClient.Width();
int CH=rClient.Height();
int yOffset;
int xOffset;
int pListx;
int DEPTHROWS=CH/17;
int WIDEROWS=(CW/43)-1;
int cnt;
int w;
int nb=SONG->numBuses;
int ledge=0;
if(SONG->seqBus>=nb)SONG->seqBus--;

DEPTHROWS--;
if(DEPTHROWS<2)DEPTHROWS=2;

WIDEROWS-=4;
if(WIDEROWS<2)WIDEROWS=2;

//////////////////////////////////////////////////////////////////////
// General view

if(updateMode==0)
{
devc->SetBkColor(0x00CCEEFF);
TXT(devc," Sequencer View",0,0,CW,18,false);
devc->MoveTo(0,18);
devc->LineTo(CW,18);
devc->MoveTo(0,36);
devc->LineTo(CW,36);
devc->MoveTo(0,35);
devc->LineTo(CW,35);

devc->SetBkColor(0x00DDEFFF);
BOX(devc,-1,18,88,18);
TXT(devc,"Generators",0,19,85,16,false);
}

devc->SelectObject(&seqFont);

ledge=(WIDEROWS*43)+87;

//////////////////////////////////////////////////////////////////////
// Draw Cells

if(updateMode==0)
{
	devc->SetBkColor(0x00C0E0F0);
	yOffset=36;
	for (w=0;w<nb;w++)
	{
	BOX(devc,0,yOffset,87,18);
	xOffset=87;
	
	for(int z=0;z<WIDEROWS;z++)
	{
		BOX(devc,xOffset,yOffset,44,18);
		xOffset+=43;
	}

	yOffset+=17;
	
	}
	BOX(devc,0,yOffset+2,ledge+1,16);
}


//////////////////////////////////////////////////////////////////////
// Draw Pattern Names box

if(updateMode==4 || updateMode==0)
{
	pListx=ledge;
	int pListW=CW-pListx;

	int PDMsize=(CH-35)/17;
	if(PDMsize>26)PDMsize=26;

	if(updateMode==0)BOX(devc,pListx,18,pListW,PDMsize*17+20);
	devc->SetBkColor(0x0099AABB);
	char buffer[32];
	sprintf(buffer,"Patterns [%d/15]",SONG->patPage/16);
	TXT(devc,buffer,pListx+2,20,pListW-4,16,false);

	yOffset=37;
	cnt=0;
	int MAX_PD=SONG->patPage+MAX_PATTERNS;
	
	for(int ps=SONG->patPage;ps<SONG->patPage+PDMsize;ps++)
	{
	if(ps<256)
	{
	int nl=SONG->patternLines[ps];
	
		char buf[4];
		sprintf(buf,"%c",cnt+97);
		devc->SetBkColor(0x00AACCDD);
		TXT(devc,buf,pListx+2,yOffset,14,16,false);
		
		sprintf(buf,"%d",ps);
		devc->SetBkColor(0x0099BBCC);
		TXT(devc,buf,pListx+17,yOffset,28,16,false);
		
		devc->SetBkColor(0x00CCEEFF);
		TXT(devc,SONG->patternName[ps],pListx+46,yOffset,pListW-48,16,false);
		
		yOffset+=17;
		cnt++;
	}// Ps<256
	}
}// UpdateMode different than (4,1)

//////////////////////////////////////////////////////////////////////
// Draw sequence position numbers

seqOffset=seqRow-(seqStep*(WIDEROWS/2));

if(seqOffset<0)seqOffset=0;

yOffset=36;
xOffset=88;

int lechefrita=seqOffset/seqStep;
int tseqOffset=seqOffset;

devc->SetBkColor(0x00CCAA88);

int n=0;
for(int z=0;z<WIDEROWS;z++)
{
		char buffer[32];
		sprintf(buffer,"%d",tseqOffset);
		
		n=0;
		if(!((lechefrita+z)%4))n-=0x00222222;
		if(!((lechefrita+z)%8))n-=0x00222222;
		
		if(tseqOffset<SONG->loopBeg || tseqOffset>=SONG->loopEnd)
		devc->SetBkColor(0x00CCDDEE+n);
		else
		devc->SetBkColor(0x00AAFFFF+n);
		
		TXT(devc,buffer,xOffset,20,42,14,false);
		
		tseqOffset+=seqStep;

		xOffset+=43;
}

//////////////////////////////////////////////////////////////////////
// Draw Pattern Sequencing data

	oldXSC=-2;
	DrawTimeLine();

	devc->SetBkColor(0x00B0D0C0);

	yOffset=37;

//////////////////////////////////////////////////////////////////////
// Draw Sequencer Buses names

yOffset=37;
xOffset=0;

devc->SetBkColor(0x00B0D0E0);

for (w=0;w<nb;w++)
{
	char buffer[32];
	sprintf(buffer,"%s",SONG->machine[SONG->busMachine[w]]->editName);
	TXT(devc,buffer,xOffset,yOffset,86,16,false);
	yOffset+=17;
}

devc->SetBkColor(0x00D0C0F0);
TXT(devc,"TimeLine",xOffset,yOffset+2,85,14,false);

//////////////////////////////////////////////////////////////////////
// Draw Sequencer Cursor
if (nb>0)
{
int xcur=88+((seqRow-seqOffset)/seqStep)*43;
int ycur=37+SONG->seqBus*17;
devc->SetROP2(R2_NOT);
BOX(devc,xcur,ycur,42,16);
devc->SetROP2(R2_BLACK);
}



}

//////////////////////////////////////////////////////////////////////
// Machine view GDI operations
void CChildView::DrawMachineVumeters(CClientDC *devc)
{
	// Draw machine boxes
	for(int c=1;c<MAX_MACHINES;c++)
	{
		if(childSong->Activemachine[c])
		DrawMachineVol(childSong->machine[c]->x,childSong->machine[c]->y, devc, childSong->machine[c]->VolumeCounter);
	}
}

void CChildView::DrawMachineEditor(CPaintDC *devc)
{
	// Draw wire [connections]
	for(int c=0;c<MAX_MACHINES;c++)
	{
		if(childSong->Activemachine[c])
		{
			psyGear *tmac=childSong->machine[c];
			int oriX=tmac->x+74;
			int oriY=tmac->y+24;

			for(int w=0;w<MAX_CONNECTIONS;w++)
			{
				if(tmac->conection[w])
				{
					int desX=childSong->machine[childSong->machine[c]->outputDest[w]]->x+74;
					int desY=childSong->machine[childSong->machine[c]->outputDest[w]]->y+24;
					
					int const f1=(desX+oriX)/2;
					int const f2=(desY+oriY)/2;

					amosDraw(devc,oriX,oriY,desX,desY);
					
					double modX=double(desX-oriX);
					double modY=double(desY-oriY);
					double modT=sqrt(modX*modX+modY*modY);
					
					modX=modX/modT;
					modY=modY/modT;
							
					CPoint pol[4];
					
					pol[0].x=f1-f2i(modX*10);
					pol[0].y=f2-f2i(modY*10);
					pol[1].x=pol[0].x+f2i(modX*20);
					pol[1].y=pol[0].y+f2i(modY*20);
					pol[2].x=pol[0].x-f2i(modY*12);
					pol[2].y=pol[0].y+f2i(modX*12);
					pol[3].x=pol[0].x+f2i(modY*12);
					pol[3].y=pol[0].y-f2i(modX*12);

					devc->Polygon(&pol[1],3);
					
					tmac->connectionPoint[w].x=f1-10;
					tmac->connectionPoint[w].y=f2-10;
				}
			}
		}// Machine actived
	}

	// Draw machine boxes
	for(c=0;c<MAX_MACHINES;c++)
	{
	if(childSong->Activemachine[c]){
		DrawMachine(childSong->machine[c]->x,childSong->machine[c]->y,childSong->machine[c]->editName, devc, childSong->machine[c]->panning,childSong->machine[c]->type);
		}// Machine exist
	}

	if(wiresource!=-1)
	{
	devc->SetROP2(R2_NOT);
	amosDraw(devc,wireSX,wireSY,wireDX,wireDY);
	
	devc->SetROP2(R2_BLACK);
	}
}

//////////////////////////////////////////////////////////////////////
// Draws a single machine

void CChildView::DrawMachineVol(int x,int y,CClientDC *devc, int volu)
{
	CDC memDC;
	memDC.CreateCompatibleDC(devc);
	memDC.SelectObject(&stuffbmp);

	int size=volu/222;
	
	if(size>96)
	size=96;

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,SOURBMPX,SOURBMPY)
	devc->BitBlt(x+8,y+3,size,5,&memDC,0,96,SRCCOPY);
	devc->BitBlt(x+8+size,y+3,96-size,5,&memDC,8,51,SRCCOPY);

}

void CChildView::DrawMachine(int x,int y, char *name, CPaintDC *devc, int panning, int type)
{
	CRect rec;
	rec.left=x;
	rec.top=y;
	rec.right=x+148;
	rec.bottom=y+48;
	
	CDC memDC;
	memDC.CreateCompatibleDC(devc);
	memDC.SelectObject(&stuffbmp);

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,SOURBMPX,SOURBMPY)
		
	switch(type)
	{
	case 0:devc->BitBlt(x,y,148,48,&memDC,0,48,SRCCOPY);break;	// Generator
	case 1:devc->BitBlt(x,y,148,48,&memDC,148,0,SRCCOPY);break;	// Effect
	case 2:devc->BitBlt(x,y,148,48,&memDC,0,0,SRCCOPY);break;	// Master
	case 3:devc->BitBlt(x,y,148,48,&memDC,148,0,SRCCOPY);break;	// Plugin
	}

	devc->SetBkMode(TRANSPARENT);

	// Drawing knobs
	devc->TextOut(x+8,y+10,name);

	rec.left=x+4;
	rec.top=y+34;
	rec.right-=4;
	rec.bottom-=2;

	// Draw pan
	if(type!=2)
	{
	panning-=3;
	if(panning<6)panning=6;
	if(panning>117)panning=117;

	devc->BitBlt(x+panning,y+36,24,8,&memDC,258,65,SRCCOPY);
	}
}

void CChildView::Draw_BackSkin()
{
	CRect rClient;

	GetClientRect(&rClient);
	int CW=rClient.Width();
	int CH=rClient.Height();

	UpdateCanvas cv(this->m_hWnd);

	int sx;
	int sy;

	Blitter* blit;

	switch(viewMode)
	{
	case 0:
		blit=new Blitter (mv_bg);
		mv_bg.GetSize(sx,sy);
	break;
	
	case 1:
		blit=new Blitter (sv_bg);
		sv_bg.GetSize(sx,sy);
	break;
	
	case 2:
		blit=new Blitter (pv_bg);
		pv_bg.GetSize(sx,sy);
	break;
	
	}

	
	for(int cx=0;cx<CW;cx+=sx)
	{
		for(int cy=0;cy<CH;cy+=sy)
		{
		blit->SetDest(cx,cy);
		blit->BlitTo(cv);
		}
	}

	delete blit;
}	

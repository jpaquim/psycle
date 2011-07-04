//////////////////////////////////////////////////////////////////////
// Machine view GDI operations
void CChildView::DrawMachineVumeters(CClientDC *devc)
{
	if (_pSong->_machineLock)
	{
		return;
	}
	// Draw machine boxes
	for (int c=1; c<MAX_MACHINES; c++)
	{
		if (_pSong->_machineActive[c])
		{
			DrawMachineVol(_pSong->_pMachines[c]->_x,
						   _pSong->_pMachines[c]->_y,
						   devc, _pSong->_pMachines[c]->_volumeCounter);
		}
	}
}

void CChildView::DrawMachineEditor(CDC *devc)
{
	if (_pSong->_machineLock)
	{
		return;
	}

	CRect rClient;
	GetClientRect(&rClient);
	devc->FillSolidRect(&rClient,Global::pConfig->mv_colour);

	// Draw wire [connections]
	for(int c=0;c<MAX_MACHINES;c++)
	{
		if(_pSong->_machineActive[c])
		{
			Machine *tmac=_pSong->_pMachines[c];
			int oriX = tmac->_x+74;
			int oriY = tmac->_y+24;

			for (int w=0; w<MAX_CONNECTIONS; w++)
			{
				if (tmac->_connection[w])
				{
					int desX = _pSong->_pMachines[_pSong->_pMachines[c]->_outputMachines[w]]->_x+74;
					int desY = _pSong->_pMachines[_pSong->_pMachines[c]->_outputMachines[w]]->_y+24;
					
					int const f1 = (desX+oriX)/2;
					int const f2 = (desY+oriY)/2;

					amosDraw(devc, oriX, oriY, desX, desY);
					
					double modX = double(desX-oriX);
					double modY = double(desY-oriY);
					double modT = sqrt(modX*modX+modY*modY);
					
					modX = modX/modT;
					modY = modY/modT;
							
					CPoint pol[4];
					
					pol[0].x = f1 - Dsp::F2I(modX*10);
					pol[0].y = f2 - Dsp::F2I(modY*10);
					pol[1].x = pol[0].x + Dsp::F2I(modX*20);
					pol[1].y = pol[0].y + Dsp::F2I(modY*20);
					pol[2].x = pol[0].x - Dsp::F2I(modY*12);
					pol[2].y = pol[0].y + Dsp::F2I(modX*12);
					pol[3].x = pol[0].x + Dsp::F2I(modY*12);
					pol[3].y = pol[0].y - Dsp::F2I(modX*12);

					devc->Polygon(&pol[1], 3);

					tmac->_connectionPoint[w].x = f1-10;
					tmac->_connectionPoint[w].y = f2-10;
				}
			}
		}// Machine actived
	}

	// Draw machine boxes
	for (c=0; c<MAX_MACHINES; c++)
	{
		if(_pSong->_machineActive[c])
		{
			DrawMachine(_pSong->_pMachines[c],c , devc);
		}// Machine exist
	}

	if (wiresource != -1)
	{
		int prevROP = devc->SetROP2(R2_NOT);
		amosDraw(devc, wireSX, wireSY, wireDX, wireDY);
		devc->SetROP2(prevROP);
	}
}

//////////////////////////////////////////////////////////////////////
// Draws a single machine

void CChildView::DrawMachineVol(int x,int y,CClientDC *devc, int volu)
{
	CDC memDC;
	memDC.CreateCompatibleDC(devc);
	memDC.SelectObject(&stuffbmp);

	int size = volu/256;
	
	if (size > 96)
	{
		size = 96;
	}

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,SOURBMPX,SOURBMPY)
	devc->BitBlt(x+8, y+3, size, 5, &memDC, 0, 96, SRCCOPY);
	devc->BitBlt(x+8+size, y+3, 96-size, 5, &memDC, 8, 51, SRCCOPY);
}

void CChildView::DrawMachine(Machine* mac,int macnum, CDC *devc)
{
	// BUGFIX by Dan: don't draw invalid machines 
	// valid machine?
	if(!mac)
	    return;

	int x=mac->_x;
	int y=mac->_y;

	CDC memDC;
	memDC.CreateCompatibleDC(devc);
	memDC.SelectObject(&stuffbmp);

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,SOURBMPX,SOURBMPY)

	switch (mac->_mode)
	{
	case MACHMODE_GENERATOR:
		devc->BitBlt(x, y, 148, 48, &memDC, 0, 48, SRCCOPY);
		break;
	case MACHMODE_FX:
		devc->BitBlt(x, y, 148, 48, &memDC, 148, 0, SRCCOPY);
		break;
	case MACHMODE_MASTER:
		devc->BitBlt(x, y, 148, 48, &memDC, 0, 0, SRCCOPY);
		break;
	case MACHMODE_PLUGIN:
		devc->BitBlt(x, y, 148, 48, &memDC, 148, 0, SRCCOPY);
		break;
	}
	if (mac->_mode != MACHMODE_MASTER)
	{
		int panning = mac->_panning;
		panning -= 3;

		if (panning < 6) panning = 6;
		if (panning > 117) panning = 117;

		// Draw pan
		devc->BitBlt(x+panning, y+36, 24, 8, &memDC, 258, 65, SRCCOPY);

		if (mac->_mute)
		{
			devc->BitBlt(x+136, y+3, 9, 9, &memDC, 257, 48, SRCCOPY);
		}
		else if (_pSong->machineSoloed > 0 && _pSong->machineSoloed == macnum )
		{
			devc->BitBlt(x+136, y+14, 9, 9, &memDC, 266, 48, SRCCOPY);
		}
		if (mac->_bypass)
		{
			devc->BitBlt(x+136, y+14, 9, 14, &memDC, 248, 48, SRCCOPY);
		}
	}

	// Draw text
	devc->SetBkMode(TRANSPARENT);
	devc->TextOut(x+8, y+10, mac->_editName);
	devc->SetBkMode(OPAQUE);

}


void CChildView::amosDraw(CDC *devc, int oX,int oY,int dX,int dY)
{
	if (oX == dX)
	{
		oX++;
	}
	if (oY == dY)
	{
		oY++;
	}
	devc->MoveTo(oX,oY);
	devc->LineTo(dX,dY);			
}

int CChildView::GetMachine(CPoint point)
{
	int tmac = -1;
	
	for (int c=MAX_MACHINES-1; c>=0; c--)
	{
		if (Global::_pSong->_machineActive[c])
		{
			int x1 = Global::_pSong->_pMachines[c]->_x;
			int y1 = Global::_pSong->_pMachines[c]->_y;
			int x2 = Global::_pSong->_pMachines[c]->_x+148;
			int y2 = Global::_pSong->_pMachines[c]->_y+48;
			
			if (point.x > x1 && point.x < x2 && point.y > y1 && point.y < y2)
			{
				tmac = c;
				break;
			}
		}
	}
	return tmac;
}

/*
void CChildView::Draw_BackSkin()
{
	CRect rClient;

	GetClientRect(&rClient);
	int CW=rClient.Width();
	int CH=rClient.Height();

	UpdateCanvas cv(m_hWnd);

	int sx;
	int sy;

	Blitter* blit;

	switch (viewMode)
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
	
	for (int cx=0; cx<CW; cx+=sx)
	{
		for (int cy=0; cy<CH; cy+=sy)
		{
			blit->SetDest(cx, cy);
			blit->BlitTo(cv);
		}
	}
	delete blit;
}	
*/
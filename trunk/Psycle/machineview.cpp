//////////////////////////////////////////////////////////////////////
// Machine view GDI operations
void CChildView::DrawMachineVumeters(CDC *devc)
{
	if (_pSong->_machineLock)
	{
		return;
	}
	// Draw machine boxes
	for (int c=0; c<MAX_MACHINES-1; c++)
	{
		Machine* pMac = _pSong->_pMachine[c];
		if (pMac)
		{
			pMac->_volumeMaxCounterLife--;
			if ((pMac->_volumeDisplay > pMac->_volumeMaxDisplay)
				|| (pMac->_volumeMaxCounterLife <= 0))
			{
				pMac->_volumeMaxDisplay = pMac->_volumeDisplay-1;
				pMac->_volumeMaxCounterLife = 60;
			}
			DrawMachineVol(pMac->_x,
						   pMac->_y,
						   devc, 
						   pMac->_volumeDisplay, 
						   pMac->_volumeMaxDisplay,
						   pMac->_mode);
		}
	}
}

void CChildView::DrawMachineEditor(CDC *devc)
{
	if (_pSong->_machineLock)
	{
		return;
	}

	CBrush fillbrush(Global::pConfig->mv_polycolour);
	CBrush *oldbrush = devc->SelectObject(&fillbrush);
	CRect rClient;
	GetClientRect(&rClient);
	devc->FillSolidRect(&rClient,Global::pConfig->mv_colour);

	if (Global::pConfig->mv_wireaa)
	{
		CPen linepen1( PS_SOLID, Global::pConfig->mv_wirewidth+(Global::pConfig->mv_wireaa*2), Global::pConfig->mv_wireaacolour);
		CPen linepen2( PS_SOLID, Global::pConfig->mv_wirewidth+(Global::pConfig->mv_wireaa), Global::pConfig->mv_wireaacolour2); 
		CPen linepen3( PS_SOLID, Global::pConfig->mv_wirewidth, Global::pConfig->mv_wirecolour); 
		CPen *oldpen = devc->SelectObject(&linepen1);
		// Draw wire [connections]
		for(int c=0;c<MAX_MACHINES;c++)
		{
			Machine *tmac=_pSong->_pMachine[c];
			if(tmac)
			{
				int oriX;
				int oriY;
				switch (tmac->_mode)
				{
				case MACHMODE_GENERATOR:
					oriX = tmac->_x+(MachineCoords.sGenerator.width/2);
					oriY = tmac->_y+(MachineCoords.sGenerator.height/2);
					break;
				case MACHMODE_FX:
				case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
					oriX = tmac->_x+(MachineCoords.sEffect.width/2);
					oriY = tmac->_y+(MachineCoords.sEffect.height/2);
					break;

				case MACHMODE_MASTER:
					oriX = tmac->_x+(MachineCoords.sMaster.width/2);
					oriY = tmac->_y+(MachineCoords.sMaster.height/2);
					break;
				}

				for (int w=0; w<MAX_CONNECTIONS; w++)
				{
					if (tmac->_connection[w])
					{
						int desX = 0;
						int desY = 0;
						Machine* pout = _pSong->_pMachine[tmac->_outputMachines[w]];
						if (pout)
						{
							switch (pout->_mode)
							{
							case MACHMODE_GENERATOR:
								desX = pout->_x+(MachineCoords.sGenerator.width/2);
								desY = pout->_y+(MachineCoords.sGenerator.height/2);
								break;
							case MACHMODE_FX:
							case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
								desX = pout->_x+(MachineCoords.sEffect.width/2);
								desY = pout->_y+(MachineCoords.sEffect.height/2);
								break;

							case MACHMODE_MASTER:
								desX = pout->_x+(MachineCoords.sMaster.width/2);
								desY = pout->_y+(MachineCoords.sMaster.height/2);
								break;
							}
						}
						
						int const f1 = (desX+oriX)/2;
						int const f2 = (desY+oriY)/2;

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

						devc->SelectObject(&linepen1);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);
						devc->SelectObject(&linepen2);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);
						devc->SelectObject(&linepen3);
						amosDraw(devc, oriX, oriY, desX, desY);
						devc->Polygon(&pol[1], 3);

						tmac->_connectionPoint[w].x = f1-10;
						tmac->_connectionPoint[w].y = f2-10;
					}
				}
			}// Machine actived
		}
		devc->SelectObject(oldpen);
		linepen1.DeleteObject();
		linepen2.DeleteObject();
		linepen3.DeleteObject();
	}
	else
	{
		CPen linepen( PS_SOLID, Global::pConfig->mv_wirewidth, Global::pConfig->mv_wirecolour); 
		CPen *oldpen = devc->SelectObject(&linepen);
		// Draw wire [connections]
		for(int c=0;c<MAX_MACHINES;c++)
		{
			Machine *tmac=_pSong->_pMachine[c];
			if(tmac)
			{
				int oriX;
				int oriY;
				switch (tmac->_mode)
				{
				case MACHMODE_GENERATOR:
					oriX = tmac->_x+(MachineCoords.sGenerator.width/2);
					oriY = tmac->_y+(MachineCoords.sGenerator.height/2);
					break;
				case MACHMODE_FX:
				case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
					oriX = tmac->_x+(MachineCoords.sEffect.width/2);
					oriY = tmac->_y+(MachineCoords.sEffect.height/2);
					break;

				case MACHMODE_MASTER:
					oriX = tmac->_x+(MachineCoords.sMaster.width/2);
					oriY = tmac->_y+(MachineCoords.sMaster.height/2);
					break;
				}

				for (int w=0; w<MAX_CONNECTIONS; w++)
				{
					if (tmac->_connection[w])
					{
						int desX = 0;
						int desY = 0;
						Machine* pout = _pSong->_pMachine[tmac->_outputMachines[w]];
						if (pout)
						{
							switch (pout->_mode)
							{
							case MACHMODE_GENERATOR:
								desX = pout->_x+(MachineCoords.sGenerator.width/2);
								desY = pout->_y+(MachineCoords.sGenerator.height/2);
								break;
							case MACHMODE_FX:
							case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
								desX = pout->_x+(MachineCoords.sEffect.width/2);
								desY = pout->_y+(MachineCoords.sEffect.height/2);
								break;

							case MACHMODE_MASTER:
								desX = pout->_x+(MachineCoords.sMaster.width/2);
								desY = pout->_y+(MachineCoords.sMaster.height/2);
								break;
							}
						}
						
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
		devc->SelectObject(oldpen);
		linepen.DeleteObject();
	}


	// Draw machine boxes
	for (int c=0; c<MAX_MACHINES; c++)
	{
		if(_pSong->_pMachine[c])
		{
			DrawMachine(_pSong->_pMachine[c],c , devc);
		}// Machine exist
	}

	// draw vumeters
	DrawMachineVumeters((CClientDC*)devc);

	if (wiresource != -1)
	{
		int prevROP = devc->SetROP2(R2_NOT);
		amosDraw(devc, wireSX, wireSY, wireDX, wireDY);
		devc->SetROP2(prevROP);
	}
	devc->SelectObject(oldbrush);
	fillbrush.DeleteObject();
}

//////////////////////////////////////////////////////////////////////
// Draws a single machine

void CChildView::DrawMachineVol(int x,int y,CDC *devc, int vol, int max, int mode)
{
	CDC memDC;
	CBitmap* oldbmp;
	memDC.CreateCompatibleDC(devc);
	oldbmp=memDC.SelectObject(&machineskin);

	switch (mode)
	{
	case MACHMODE_GENERATOR:
		// scale our volumes
		vol *= MachineCoords.dGeneratorVu.width;
		vol /= 96;

		max *= MachineCoords.dGeneratorVu.width;
		max /= 96;

		if (MachineCoords.bHasTransparency)
		{
			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
				if (MachineCoords.sGeneratorVu0.width)
				{
					vol /= MachineCoords.sGeneratorVu0.width;// restrict to leds
					vol *= MachineCoords.sGeneratorVu0.width;
				}
			}
			else
			{
				vol = 0;
			}

			RECT r;
			r.left = x+vol+MachineCoords.dGeneratorVu.x;
			r.top = y+MachineCoords.dGeneratorVu.y;
			r.right = r.left + MachineCoords.dGeneratorVu.width-vol;
			r.bottom = r.top + MachineCoords.sGeneratorVu0.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);

			TransparentBlt(devc,
						r.left, 
						r.top, 
						MachineCoords.dGeneratorVu.width-vol, 
						MachineCoords.sGeneratorVu0.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sGenerator.x+MachineCoords.dGeneratorVu.x, 
						MachineCoords.sGenerator.y+MachineCoords.dGeneratorVu.y);

			if (max > 0)
			{
				if (MachineCoords.sGeneratorVuPeak.width)
				{
					max /= MachineCoords.sGeneratorVuPeak.width;// restrict to leds
					max *= MachineCoords.sGeneratorVuPeak.width;
					TransparentBlt(devc,
								x+max+MachineCoords.dGeneratorVu.x, 
								y+MachineCoords.dGeneratorVu.y, 
								MachineCoords.sGeneratorVuPeak.width, 
								MachineCoords.sGeneratorVuPeak.height, 
								&memDC, 
								&machineskinmask,
								MachineCoords.sGeneratorVuPeak.x, 
								MachineCoords.sGeneratorVuPeak.y);
				}
			}

			if (vol > 0)
			{
				TransparentBlt(devc,
							x+MachineCoords.dGeneratorVu.x, 
							y+MachineCoords.dGeneratorVu.y, 
							vol, 
							MachineCoords.sGeneratorVu0.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorVu0.x, 
							MachineCoords.sGeneratorVu0.y);
			}
		}
		else
		{
			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
				if (MachineCoords.sGeneratorVu0.width)
				{
					vol /= MachineCoords.sGeneratorVu0.width;// restrict to leds
					vol *= MachineCoords.sGeneratorVu0.width;
				}
			}
			else
			{
				vol = 0;
			}

			devc->BitBlt(x+vol+MachineCoords.dGeneratorVu.x, 
							y+MachineCoords.dGeneratorVu.y, 
							MachineCoords.dGeneratorVu.width-vol, 
							MachineCoords.sGeneratorVu0.height, 
							&memDC, 
							MachineCoords.sGenerator.x+MachineCoords.dGeneratorVu.x, 
							MachineCoords.sGenerator.y+MachineCoords.dGeneratorVu.y, 
							SRCCOPY); //background

			if (max > 0)
			{
				if (MachineCoords.sGeneratorVuPeak.width)
				{
					max /= MachineCoords.sGeneratorVuPeak.width;// restrict to leds
					max *= MachineCoords.sGeneratorVuPeak.width;
					devc->BitBlt(x+max+MachineCoords.dGeneratorVu.x, 
								y+MachineCoords.dGeneratorVu.y, 
								MachineCoords.sGeneratorVuPeak.width, 
								MachineCoords.sGeneratorVuPeak.height, 
								&memDC, 
								MachineCoords.sGeneratorVuPeak.x, 
								MachineCoords.sGeneratorVuPeak.y, 
								SRCCOPY); //peak
				}
			}

			if (vol > 0)
			{
				devc->BitBlt(x+MachineCoords.dGeneratorVu.x, 
							y+MachineCoords.dGeneratorVu.y, 
							vol, 
							MachineCoords.sGeneratorVu0.height, 
							&memDC, 
							MachineCoords.sGeneratorVu0.x, 
							MachineCoords.sGeneratorVu0.y, 
							SRCCOPY); // leds
			}
		}

		break;
	case MACHMODE_FX:
	case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
		// scale our volumes
		vol *= MachineCoords.dEffectVu.width;
		vol /= 96;

		max *= MachineCoords.dEffectVu.width;
		max /= 96;

		if (MachineCoords.bHasTransparency)
		{
			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
				if (MachineCoords.sEffectVu0.width)
				{
					vol /= MachineCoords.sEffectVu0.width;// restrict to leds
					vol *= MachineCoords.sEffectVu0.width;
				}
			}
			else
			{
				vol = 0;
			}

			RECT r;
			r.left = x+vol+MachineCoords.dEffectVu.x;
			r.top = y+MachineCoords.dEffectVu.y;
			r.right = r.left + MachineCoords.dEffectVu.width-vol;
			r.bottom = r.top + MachineCoords.sEffectVu0.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);

			TransparentBlt(devc,
						r.left, 
						r.top, 
						MachineCoords.dEffectVu.width-vol, 
						MachineCoords.sEffectVu0.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sEffect.x+MachineCoords.dEffectVu.x, 
						MachineCoords.sEffect.y+MachineCoords.dEffectVu.y);

			if (max > 0)
			{
				if (MachineCoords.sEffectVuPeak.width)
				{
					max /= MachineCoords.sEffectVuPeak.width;// restrict to leds
					max *= MachineCoords.sEffectVuPeak.width;
					TransparentBlt(devc,
								x+max+MachineCoords.dEffectVu.x, 
								y+MachineCoords.dEffectVu.y, 
								MachineCoords.sEffectVuPeak.width, 
								MachineCoords.sEffectVuPeak.height, 
								&memDC, 
								&machineskinmask,
								MachineCoords.sEffectVuPeak.x, 
								MachineCoords.sEffectVuPeak.y);
				}
			}

			if (vol > 0)
			{
				TransparentBlt(devc,
							x+MachineCoords.dEffectVu.x, 
							y+MachineCoords.dEffectVu.y, 
							vol, 
							MachineCoords.sEffectVu0.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectVu0.x, 
							MachineCoords.sEffectVu0.y);
			}
		}
		else
		{
			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
				if (MachineCoords.sEffectVu0.width)
				{
					vol /= MachineCoords.sEffectVu0.width;// restrict to leds
					vol *= MachineCoords.sEffectVu0.width;
				}
			}
			else
			{
				vol = 0;
			}

			devc->BitBlt(x+vol+MachineCoords.dEffectVu.x, 
							y+MachineCoords.dEffectVu.y, 
							MachineCoords.dEffectVu.width-vol, 
							MachineCoords.sEffectVu0.height, 
							&memDC, 
							MachineCoords.sEffect.x+MachineCoords.dEffectVu.x, 
							MachineCoords.sEffect.y+MachineCoords.dEffectVu.y, 
							SRCCOPY); //background

			if (max > 0)
			{
				if (MachineCoords.sEffectVuPeak.width)
				{
					max /= MachineCoords.sEffectVuPeak.width;// restrict to leds
					max *= MachineCoords.sEffectVuPeak.width;
					devc->BitBlt(x+max+MachineCoords.dEffectVu.x, 
								y+MachineCoords.dEffectVu.y, 
								MachineCoords.sEffectVuPeak.width, 
								MachineCoords.sEffectVuPeak.height, 
								&memDC, 
								MachineCoords.sEffectVuPeak.x, 
								MachineCoords.sEffectVuPeak.y, 
								SRCCOPY); //peak
				}
			}

			if (vol > 0)
			{
				devc->BitBlt(x+MachineCoords.dEffectVu.x, 
							y+MachineCoords.dEffectVu.y, 
							vol, 
							MachineCoords.sEffectVu0.height, 
							&memDC, 
							MachineCoords.sEffectVu0.x, 
							MachineCoords.sEffectVu0.y, 
							SRCCOPY); // leds
			}
		}

		break;

	}

	memDC.SelectObject(oldbmp);
	memDC.DeleteDC();
}

void CChildView::ClearMachineSpace(Machine* mac,int macnum, CDC *devc)
{
	if(!mac)
	    return;

	if (MachineCoords.bHasTransparency)
	{
		RECT r;
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			r.left = mac->_x;
			r.top = mac->_y;
			r.right = r.left + MachineCoords.sGenerator.width;
			r.bottom = r.top + MachineCoords.sGenerator.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			break;
		case MACHMODE_FX:
		case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
			r.left = mac->_x;
			r.top = mac->_y;
			r.right = r.left + MachineCoords.sEffect.width;
			r.bottom = r.top + MachineCoords.sEffect.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			break;
		}
	}
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
	CBitmap* oldbmp = memDC.SelectObject(&machineskin);

	// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
	if (MachineCoords.bHasTransparency)
	{
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			/*
			RECT r;
			r.left = x;
			r.top = y;
			r.right = r.left + MachineCoords.sGenerator.width;
			r.bottom = r.top + MachineCoords.sGenerator.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			*/

			TransparentBlt(devc,
						x, 
						y, 
						MachineCoords.sGenerator.width, 
						MachineCoords.sGenerator.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sGenerator.x, 
						MachineCoords.sGenerator.y);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dGeneratorPan.width;
				panning /= 128;
				TransparentBlt(devc,
							x+panning+MachineCoords.dGeneratorPan.x, 
							y+MachineCoords.dGeneratorPan.y, 
							MachineCoords.sGeneratorPan.width, 
							MachineCoords.sGeneratorPan.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorPan.x, 
							MachineCoords.sGeneratorPan.y);
			}
			if (mac->_mute)
			{
				TransparentBlt(devc,
							x+MachineCoords.dGeneratorMute.x, 
							y+MachineCoords.dGeneratorMute.y, 
							MachineCoords.sGeneratorMute.width, 
							MachineCoords.sGeneratorMute.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorMute.x, 
							MachineCoords.sGeneratorMute.y);
			}
			else if (_pSong->machineSoloed == macnum)
			{
				TransparentBlt(devc,
							x+MachineCoords.dGeneratorSolo.x, 
							y+MachineCoords.dGeneratorSolo.y, 
							MachineCoords.sGeneratorSolo.width, 
							MachineCoords.sGeneratorSolo.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sGeneratorSolo.x, 
							MachineCoords.sGeneratorSolo.y);
			}
			// Draw text
			{
				CFont* oldFont= devc->SelectObject(&Global::pConfig->generatorFont);
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_generator_fontcolour);
				devc->TextOut(x+MachineCoords.dGeneratorName.x, y+MachineCoords.dGeneratorName.y, mac->_editName);
				devc->SetBkMode(OPAQUE);
				devc->SelectObject(oldFont);
			}
			break;
		case MACHMODE_FX:
		case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
			/*
			RECT r;
			r.left = x;
			r.top = y;
			r.right = r.left + MachineCoords.sEffect.width;
			r.bottom = r.top + MachineCoords.sEffect.height;
			devc->FillSolidRect(&r,Global::pConfig->mv_colour);
			*/

			TransparentBlt(devc,
						x, 
						y,
						MachineCoords.sEffect.width, 
						MachineCoords.sEffect.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sEffect.x, 
						MachineCoords.sEffect.y);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dEffectPan.width;
				panning /= 128;
				TransparentBlt(devc,
							x+panning+MachineCoords.dEffectPan.x, 
							y+MachineCoords.dEffectPan.y, 
							MachineCoords.sEffectPan.width, 
							MachineCoords.sEffectPan.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectPan.x, 
							MachineCoords.sEffectPan.y);
			}
			if (mac->_mute)
			{
				TransparentBlt(devc,
							x+MachineCoords.dEffectMute.x, 
							y+MachineCoords.dEffectMute.y, 
							MachineCoords.sEffectMute.width, 
							MachineCoords.sEffectMute.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectMute.x, 
							MachineCoords.sEffectMute.y);
			}
			if (mac->_bypass)
			{
				TransparentBlt(devc,
							x+MachineCoords.dEffectBypass.x, 
							y+MachineCoords.dEffectBypass.y, 
							MachineCoords.sEffectBypass.width, 
							MachineCoords.sEffectBypass.height, 
							&memDC, 
							&machineskinmask,
							MachineCoords.sEffectBypass.x, 
							MachineCoords.sEffectBypass.y);
			}
			// Draw text
			{
				CFont* oldFont= devc->SelectObject(&Global::pConfig->effectFont);
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_effect_fontcolour);
				devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, mac->_editName);
				devc->SetBkMode(OPAQUE);
				devc->SelectObject(oldFont);
			}
			break;

		case MACHMODE_MASTER:
			TransparentBlt(devc,
						x, 
						y,
						MachineCoords.sMaster.width, 
						MachineCoords.sMaster.height, 
						&memDC, 
						&machineskinmask,
						MachineCoords.sMaster.x, 
						MachineCoords.sMaster.y);
			break;
		}
	}
	else
	{
		switch (mac->_mode)
		{
		case MACHMODE_GENERATOR:
			devc->BitBlt(x, 
						y, 
						MachineCoords.sGenerator.width, 
						MachineCoords.sGenerator.height, 
						&memDC, 
						MachineCoords.sGenerator.x, 
						MachineCoords.sGenerator.y, 
						SRCCOPY);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dGeneratorPan.width;
				panning /= 128;
				devc->BitBlt(x+panning+MachineCoords.dGeneratorPan.x, 
							y+MachineCoords.dGeneratorPan.y, 
							MachineCoords.sGeneratorPan.width, 
							MachineCoords.sGeneratorPan.height, 
							&memDC, 
							MachineCoords.sGeneratorPan.x, 
							MachineCoords.sGeneratorPan.y, 
							SRCCOPY);
			}
			if (mac->_mute)
			{
				devc->BitBlt(x+MachineCoords.dGeneratorMute.x, 
							y+MachineCoords.dGeneratorMute.y, 
							MachineCoords.sGeneratorMute.width, 
							MachineCoords.sGeneratorMute.height, 
							&memDC, 
							MachineCoords.sGeneratorMute.x, 
							MachineCoords.sGeneratorMute.y, 
							SRCCOPY);
			}
			else if (_pSong->machineSoloed == macnum)
			{
				devc->BitBlt(x+MachineCoords.dGeneratorSolo.x, 
							y+MachineCoords.dGeneratorSolo.y, 
							MachineCoords.sGeneratorSolo.width, 
							MachineCoords.sGeneratorSolo.height, 
							&memDC, 
							MachineCoords.sGeneratorSolo.x, 
							MachineCoords.sGeneratorSolo.y, 
							SRCCOPY);
			}
			// Draw text
			{
				CFont* oldFont= devc->SelectObject(&Global::pConfig->generatorFont);
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_generator_fontcolour);
				devc->TextOut(x+MachineCoords.dGeneratorName.x, y+MachineCoords.dGeneratorName.y, mac->_editName);
				devc->SetBkMode(OPAQUE);
				devc->SelectObject(oldFont);
			}
			break;
		case MACHMODE_FX:
		case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
			devc->BitBlt(x, 
						y,
						MachineCoords.sEffect.width, 
						MachineCoords.sEffect.height, 
						&memDC, 
						MachineCoords.sEffect.x, 
						MachineCoords.sEffect.y, 
						SRCCOPY);
			// Draw pan
			{
				int panning = mac->_panning*MachineCoords.dEffectPan.width;
				panning /= 128;
				devc->BitBlt(x+panning+MachineCoords.dEffectPan.x, 
							y+MachineCoords.dEffectPan.y, 
							MachineCoords.sEffectPan.width, 
							MachineCoords.sEffectPan.height, 
							&memDC, 
							MachineCoords.sEffectPan.x, 
							MachineCoords.sEffectPan.y, 
							SRCCOPY);
			}
			if (mac->_mute)
			{
				devc->BitBlt(x+MachineCoords.dEffectMute.x, 
							y+MachineCoords.dEffectMute.y, 
							MachineCoords.sEffectMute.width, 
							MachineCoords.sEffectMute.height, 
							&memDC, 
							MachineCoords.sEffectMute.x, 
							MachineCoords.sEffectMute.y, 
							SRCCOPY);
			}
			if (mac->_bypass)
			{
				devc->BitBlt(x+MachineCoords.dEffectBypass.x, 
							y+MachineCoords.dEffectBypass.y, 
							MachineCoords.sEffectBypass.width, 
							MachineCoords.sEffectBypass.height, 
							&memDC, 
							MachineCoords.sEffectBypass.x, 
							MachineCoords.sEffectBypass.y, 
							SRCCOPY);
			}
			// Draw text
			{
				CFont* oldFont= devc->SelectObject(&Global::pConfig->effectFont);
				devc->SetBkMode(TRANSPARENT);
				devc->SetTextColor(Global::pConfig->mv_effect_fontcolour);
				devc->TextOut(x+MachineCoords.dEffectName.x, y+MachineCoords.dEffectName.y, mac->_editName);
				devc->SetBkMode(OPAQUE);
				devc->SelectObject(oldFont);
			}
			break;

		case MACHMODE_MASTER:
			devc->BitBlt(x, 
						y,
						MachineCoords.sMaster.width, 
						MachineCoords.sMaster.height, 
						&memDC, 
						MachineCoords.sMaster.x, 
						MachineCoords.sMaster.y, 
						SRCCOPY);
			break;
		}
	}
	memDC.SelectObject(oldbmp);
	memDC.DeleteDC();
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
		Machine* pMac = Global::_pSong->_pMachine[c];
		if (pMac)
		{
			int x1 = pMac->_x;
			int y1 = pMac->_y;
			int x2,y2;
			switch (pMac->_mode)
			{
			case MACHMODE_GENERATOR:
				x2 = pMac->_x+MachineCoords.sGenerator.width;
				y2 = pMac->_y+MachineCoords.sGenerator.height;
				break;
			case MACHMODE_FX:
			case MACHMODE_PLUGIN: // Plugins which are generators are MACHMODE_GENERATOR
				x2 = pMac->_x+MachineCoords.sEffect.width;
				y2 = pMac->_y+MachineCoords.sEffect.height;
				break;

			case MACHMODE_MASTER:
				x2 = pMac->_x+MachineCoords.sMaster.width;
				y2 = pMac->_y+MachineCoords.sMaster.height;
				break;
			}
			
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
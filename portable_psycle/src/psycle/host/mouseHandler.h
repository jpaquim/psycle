void CChildView::OnRButtonDown( UINT nFlags, CPoint point )
{	
	if (viewMode == VMMachine)
	{
		// Check for right pressed connection
		int propMac = GetMachine(point);
		
		if (propMac != -1) 
		{
			// Shows machine properties dialog
			DoMacPropDialog(propMac);
		}
		else
		{
			for (int c=0; c<MAX_MACHINES; c++)
			{
				Machine *tmac = Global::_pSong->_pMachine[c];
				if (tmac)
				{
					for (int w = 0; w<MAX_CONNECTIONS; w++)
					{
						if (tmac->_connection[w])
						{
							int xt = tmac->_connectionPoint[w].x;
							int yt = tmac->_connectionPoint[w].y;
							
							if ((point.x > xt) && (point.x < xt+triangle_size_tall) && (point.y > yt) && (point.y < yt+triangle_size_tall))
							{
								for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
								{
									if (WireDialog[i])
									{
										if ((WireDialog[i]->_pSrcMachine == tmac) &&
											(WireDialog[i]->_pDstMachine == Global::_pSong->_pMachine[tmac->_outputMachines[w]]))
										{
											return;
										}
									}
								}
								for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
								{
									if (!WireDialog[i])
									{
										WireDialog[i] = new CWireDlg(this);
										WireDialog[i]->this_index = i;
										WireDialog[i]->wireIndex = w;
										WireDialog[i]->isrcMac = c;
										WireDialog[i]->_pSrcMachine = tmac;
										WireDialog[i]->_pDstMachine = Global::_pSong->_pMachine[tmac->_outputMachines[w]];
										WireDialog[i]->Create();
										pParentMain->CenterWindowOnPoint(WireDialog[i], point);
										WireDialog[i]->ShowWindow(SW_SHOW);
										return;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	/*
	else if (viewMode == VMPattern)
	{
		editcur.track = tOff + (point.x-XOFFSET)/ROWWIDTH;
		if ( editcur.track >= _pSong->SONGTRACKS ) editcur.track = _pSong->SONGTRACKS-1;
		else if ( editcur.track < 0 ) editcur.track = 0;

		int plines = _pSong->patternLines[_ps()];
		editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
		if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
		else if ( editcur.line < 0 ) editcur.line = 0;

		editcur.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);
	}
*/
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if (viewMode == VMPattern)
	{
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_POPUPMENU));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
		
		menu.DestroyMenu();
//		Repaint(DMCursor);
	}
}


void CChildView::OnLButtonDown( UINT nFlags, CPoint point )
{
	
	SetCapture();

	switch(viewMode)
	{
		
	case VMMachine: // User is in machine view mode
		if (Global::_pSong->_machineLock)
		{
			return;
		}
		smac = -1;
		smacmode = 0;
		
		wiresource = -1;
		wiredest = -1;

		if ( nFlags & MK_CONTROL)
		{
			smac=GetMachine(point);
			if ( smac != -1 )
			{
				switch (Global::_pSong->_pMachine[smac]->_mode)
				{
				case MACHMODE_GENERATOR:
				case MACHMODE_FX:
				mcd_x = point.x - Global::_pSong->_pMachine[smac]->_x;
				mcd_y = point.y - Global::_pSong->_pMachine[smac]->_y;
					_pSong->seqBus = _pSong->FindBusFromIndex(smac);
					pParentMain->UpdateComboGen();
					Repaint();	
					break;
				}
			} 
			else
			{						
				wiresource = GetMachine(point);
				wiremove = -1;
				if (wiresource == -1)
				{

					for (int c=0; c<MAX_MACHINES; c++)
					{
						Machine *tmac = Global::_pSong->_pMachine[c];
						if (tmac)
						{
							for (int w = 0; w<MAX_CONNECTIONS; w++)
							{
								if (tmac->_connection[w])
								{
									int xt = tmac->_connectionPoint[w].x;
									int yt = tmac->_connectionPoint[w].y;
									
									if ((point.x > xt) && (point.x < xt+triangle_size_tall) && (point.y > yt) && (point.y < yt+triangle_size_tall))
									{
										// we found it																		
										wiredest = tmac->_outputMachines[w];
										wiremove = Global::_pSong->_pMachine[wiredest]->FindInputWire(c);
										wiresource = -1;
										break;
									}
								}
							}
						}
			}
				}
				if (wiredest != -1)
				{
					switch (Global::_pSong->_pMachine[wiredest]->_mode)
					{
					case MACHMODE_GENERATOR:
						wireDX = Global::_pSong->_pMachine[wiredest]->_x+(MachineCoords.sGenerator.width/2);
						wireDY = Global::_pSong->_pMachine[wiredest]->_y+(MachineCoords.sGenerator.height/2);
						break;
					case MACHMODE_FX:
						wireDX = Global::_pSong->_pMachine[wiredest]->_x+(MachineCoords.sEffect.width/2);
						wireDY = Global::_pSong->_pMachine[wiredest]->_y+(MachineCoords.sEffect.height/2);
						break;

					case MACHMODE_MASTER:
						wireDX = Global::_pSong->_pMachine[wiredest]->_x+(MachineCoords.sMaster.width/2);
						wireDY = Global::_pSong->_pMachine[wiredest]->_y+(MachineCoords.sMaster.height/2);
						break;
					}		

					OnMouseMove(nFlags,point);
				}
			}

			//end of added by J. Redfern


		}
		else if (nFlags & MK_SHIFT)
		{
			wiresource = GetMachine(point);
			wiremove = -1;
			if (wiresource == -1)
			{
				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = Global::_pSong->_pMachine[c];
					if (tmac)
					{
						for (int w = 0; w<MAX_CONNECTIONS; w++)
						{
							if (tmac->_connection[w])
							{
								int xt = tmac->_connectionPoint[w].x;
								int yt = tmac->_connectionPoint[w].y;
								
								if ((point.x > xt) && (point.x < xt+triangle_size_tall) && (point.y > yt) && (point.y < yt+triangle_size_tall))
								{
									// we found it
									wiremove = w;
									wiresource = c;
									break;
								}
							}
						}
					}
				}
			}
			if (wiresource != -1)
			{
				switch (Global::_pSong->_pMachine[wiresource]->_mode)
				{
				case MACHMODE_GENERATOR:
					wireSX = Global::_pSong->_pMachine[wiresource]->_x+(MachineCoords.sGenerator.width/2);
					wireSY = Global::_pSong->_pMachine[wiresource]->_y+(MachineCoords.sGenerator.height/2);
					break;
				case MACHMODE_FX:
					wireSX = Global::_pSong->_pMachine[wiresource]->_x+(MachineCoords.sEffect.width/2);
					wireSY = Global::_pSong->_pMachine[wiresource]->_y+(MachineCoords.sEffect.height/2);
					break;

				case MACHMODE_MASTER:
					wireSX = Global::_pSong->_pMachine[wiresource]->_x+(MachineCoords.sMaster.width/2);
					wireSY = Global::_pSong->_pMachine[wiresource]->_y+(MachineCoords.sMaster.height/2);
					break;
				}
				OnMouseMove(nFlags,point);
			}
		}// Shift
		else if (nFlags & MK_LBUTTON)
		{
			smac=GetMachine(point);

			if ( smac != -1 )
			{
				switch (Global::_pSong->_pMachine[smac]->_mode)
				{
					case MACHMODE_GENERATOR:
						mcd_x = point.x - Global::_pSong->_pMachine[smac]->_x;
						mcd_y = point.y - Global::_pSong->_pMachine[smac]->_y;
						_pSong->seqBus = _pSong->FindBusFromIndex(smac);
						pParentMain->UpdateComboGen();
						Repaint();	
						break;
				}

				mcd_x = point.x - Global::_pSong->_pMachine[smac]->_x;
				mcd_y = point.y - Global::_pSong->_pMachine[smac]->_y;

				int panning;

				switch (Global::_pSong->_pMachine[smac]->_mode)
				{
				case MACHMODE_GENERATOR:
					panning = Global::_pSong->_pMachine[smac]->_panning*MachineCoords.dGeneratorPan.width;
					panning /= 128;
					if ((mcd_x >= panning+MachineCoords.dGeneratorPan.x) && 
						(mcd_x < panning+MachineCoords.dGeneratorPan.x+MachineCoords.sGeneratorPan.width) && 
						(mcd_y >= MachineCoords.dGeneratorPan.y) && 
						(mcd_y < MachineCoords.dGeneratorPan.y+MachineCoords.sGeneratorPan.height)) //changing panning
					{
						smacmode = 1;
						OnMouseMove(nFlags,point);
					}
					else if ((mcd_x >= MachineCoords.dGeneratorMute.x) && 
							(mcd_x < MachineCoords.dGeneratorMute.x+MachineCoords.sGeneratorMute.width) &&
							(mcd_y >= MachineCoords.dGeneratorMute.y) && 
							(mcd_y < MachineCoords.dGeneratorMute.y+MachineCoords.sGeneratorMute.height)) //Mute 
					{
						Global::_pSong->_pMachine[smac]->_mute = !Global::_pSong->_pMachine[smac]->_mute;
						if (Global::_pSong->_pMachine[smac]->_mute)
						{
							Global::_pSong->_pMachine[smac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[smac]->_volumeDisplay=0;
							if (Global::_pSong->machineSoloed == smac )
							{
								Global::_pSong->machineSoloed = -1;
							}
						}
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_x >= MachineCoords.dGeneratorSolo.x) && 
							(mcd_x < MachineCoords.dGeneratorSolo.x+MachineCoords.sGeneratorSolo.width) &&
							(mcd_y >= MachineCoords.dGeneratorSolo.y) && 
							(mcd_y < MachineCoords.dGeneratorSolo.y+MachineCoords.sGeneratorSolo.height)) //Solo 
					{
						if (Global::_pSong->machineSoloed == smac )
						{
							Global::_pSong->machineSoloed = -1;
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->_pMachine[i] )
								{
									if (( Global::_pSong->_pMachine[i]->_mode == MACHMODE_GENERATOR ))
									{
										Global::_pSong->_pMachine[i]->_mute = false;
									}
								}
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->_pMachine[i] )
								{
									if (( Global::_pSong->_pMachine[i]->_mode == MACHMODE_GENERATOR ) && (i != smac))
									{
										Global::_pSong->_pMachine[i]->_mute = true;
										Global::_pSong->_pMachine[i]->_volumeCounter=0.0f;
										Global::_pSong->_pMachine[i]->_volumeDisplay=0;
									}
								}
							}
							Global::_pSong->_pMachine[smac]->_mute = false;
							Global::_pSong->machineSoloed = smac;
						}
						updatePar = smac;
						Repaint(DMAllMacsRefresh);
					}
					break;
				case MACHMODE_FX:
					panning = Global::_pSong->_pMachine[smac]->_panning*MachineCoords.dEffectPan.width;
					panning /= 128;
					if ((mcd_x >= panning+MachineCoords.dEffectPan.x) && 
						(mcd_x < panning+MachineCoords.dEffectPan.x+MachineCoords.sEffectPan.width) && 
						(mcd_y >= MachineCoords.dEffectPan.y) && 
						(mcd_y < MachineCoords.dEffectPan.y+MachineCoords.sEffectPan.height)) //changing panning
					{
						smacmode = 1;
						OnMouseMove(nFlags,point);
					}
					else if ((mcd_x >= MachineCoords.dEffectMute.x) && 
							(mcd_x < MachineCoords.dEffectMute.x+MachineCoords.sEffectMute.width) &&
							(mcd_y >= MachineCoords.dEffectMute.y) && 
							(mcd_y < MachineCoords.dEffectMute.y+MachineCoords.sEffectMute.height)) //Mute 
					{
						Global::_pSong->_pMachine[smac]->_mute = !Global::_pSong->_pMachine[smac]->_mute;
						if (Global::_pSong->_pMachine[smac]->_mute)
						{
							Global::_pSong->_pMachine[smac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[smac]->_volumeDisplay=0;
						}
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_x >= MachineCoords.dEffectBypass.x) && 
							(mcd_x < MachineCoords.dEffectBypass.x+MachineCoords.sEffectBypass.width) &&
							(mcd_y >= MachineCoords.dEffectBypass.y) && 
							(mcd_y < MachineCoords.dEffectBypass.y+MachineCoords.sEffectBypass.height)) //Solo 
					{
						Global::_pSong->_pMachine[smac]->_bypass = !Global::_pSong->_pMachine[smac]->_bypass;
						if (Global::_pSong->_pMachine[smac]->_bypass)
						{
							Global::_pSong->_pMachine[smac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[smac]->_volumeDisplay=0;
						}
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					break;

				case MACHMODE_MASTER:
					break;
				}
			}
		}// No Shift

		break;
		
	case VMPattern:
		
		int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
		if ( ttm >= _pSong->SONGTRACKS ) ttm = _pSong->SONGTRACKS-1;
		else if ( ttm < 0 ) ttm = 0;
		
		if (point.y >= 0 && point.y < YOFFSET ) // Mouse is in Track Header.
		{	
			int pointpos= ((point.x-XOFFSET)%ROWWIDTH) - HEADER_INDENT;

			if ((pointpos >= PatHeaderCoords.dRecordOn.x) && 
				(pointpos < PatHeaderCoords.dRecordOn.x+PatHeaderCoords.sRecordOn.width) &&
				(point.y >= PatHeaderCoords.dRecordOn.y+1) &&
				(point.y < PatHeaderCoords.dRecordOn.y+1+PatHeaderCoords.sRecordOn.height))
			{
				_pSong->_trackArmed[ttm] = !_pSong->_trackArmed[ttm];
				_pSong->_trackArmedCount = 0;
				for ( int i=0;i<MAX_TRACKS;i++ )
				{
					if (_pSong->_trackArmed[i])
					{
						_pSong->_trackArmedCount++;
					}
				}
			}
			else if ((pointpos >= PatHeaderCoords.dMuteOn.x) && 
				(pointpos < PatHeaderCoords.dMuteOn.x+PatHeaderCoords.sMuteOn.width) &&
				(point.y >= PatHeaderCoords.dMuteOn.y+1) &&
				(point.y < PatHeaderCoords.dMuteOn.y+1+PatHeaderCoords.sMuteOn.height))
			{
				_pSong->_trackMuted[ttm] = !_pSong->_trackMuted[ttm];
			}
			else if ((pointpos >= PatHeaderCoords.dSoloOn.x) && 
				(pointpos < PatHeaderCoords.dSoloOn.x+PatHeaderCoords.sSoloOn.width) &&
				(point.y >= PatHeaderCoords.dSoloOn.y+1) &&
				(point.y < PatHeaderCoords.dSoloOn.y+1+PatHeaderCoords.sSoloOn.height))
			{
				if (Global::_pSong->_trackSoloed != ttm )
				{
					for ( int i=0;i<MAX_TRACKS;i++ )
					{
						_pSong->_trackMuted[i] = true;
					}
					_pSong->_trackMuted[ttm] = false;
					_pSong->_trackSoloed = ttm;
				}
				else
				{
					for ( int i=0;i<MAX_TRACKS;i++ )
					{
						_pSong->_trackMuted[i] = false;
					}
					_pSong->_trackSoloed = -1;
				}
			}
			oldm.track = -1;
			Repaint(DMTrackHeader);
			break;
		}
		else if ( point.y >= YOFFSET )
		{
			oldm.track=ttm;

			int plines = _pSong->patternLines[_ps()];
			oldm.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
			if ( oldm.line >= plines ) { oldm.line = plines - 1; }
			else if ( oldm.line < 0 ) oldm.line = 0;

			oldm.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);

			blockStart = TRUE;
			if (nFlags & MK_SHIFT)
			{
				editcur = oldm;
				Repaint(DMCursor);
			}
		}

	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture();
	
	switch (viewMode)
	{
	case VMMachine:
		
		if (wiresource != -1)
		{
			wiredest = GetMachine(point);
			if ((wiredest !=-1) && (wiredest != wiresource))
			{
				AddMacViewUndo();

				// are we moving a wire?
				if (wiremove >= 0)
				{
					// buffer the volume
					int dm,w;		//dm is the old destination machine
					float volume = 1.0f;

					if (Global::_pSong->_pMachine[wiresource])
					{
						dm = Global::_pSong->_pMachine[wiresource]->_outputMachines[wiremove];

						if (Global::_pSong->_pMachine[dm])
						{
							w = Global::_pSong->_pMachine[dm]->FindInputWire(wiresource);
							Global::_pSong->_pMachine[dm]->GetWireVolume(w,volume);
							if (Global::_pSong->InsertConnection(wiresource, wiredest,volume))
							{
								// delete the old wire
								Global::_pSong->_pMachine[wiresource]->_connection[wiremove] = FALSE;
								Global::_pSong->_pMachine[wiresource]->_numOutputs--;

								Global::_pSong->_pMachine[dm]->_inputCon[w] = FALSE;
								Global::_pSong->_pMachine[dm]->_numInputs--;
							}
							else
							{
								MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
							}
						}
					}
				}
				// or making a new one?
				else if (!Global::_pSong->InsertConnection(wiresource, wiredest))
				{
					MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
				}

			}
			wiresource = -1;
			wiredest = -1;
			Repaint();
		}

		//Added by J. Redfern
		else if (wiredest != -1)
		{
			wiresource = GetMachine(point);
			if ((wiresource !=-1) && (wiresource != wiredest))
			{
				AddMacViewUndo();

				// are we moving a wire?
				if (wiremove >= 0)
				{
					// buffer the volume
					int sm,w;		//sm is the old source machine
					float volume = 1.0f;

					if (Global::_pSong->_pMachine[wiredest])
					{					

						sm = Global::_pSong->_pMachine[wiredest]->_inputMachines[wiremove];
						
						if (Global::_pSong->_pMachine[sm])
						{
							w = Global::_pSong->_pMachine[wiredest]->FindInputWire(sm);
							Global::_pSong->_pMachine[wiredest]->GetWireVolume(w,volume);
							if (Global::_pSong->InsertConnection(wiresource, wiredest,volume))
							{
								// delete the old wire

								w = Global::_pSong->_pMachine[sm]->FindOutputWire(wiredest);

								Global::_pSong->_pMachine[sm]->_connection[w] = FALSE;
								Global::_pSong->_pMachine[sm]->_numOutputs--;

								Global::_pSong->_pMachine[wiredest]->_inputCon[wiremove] = FALSE;
								Global::_pSong->_pMachine[wiredest]->_numInputs--;
							}
							else
							{
								MessageBox("Machine connection failed!","Error!", MB_ICONERROR);
							}
						}
					}
				}
			}
			wiresource = -1;
			wiredest = -1;
			Repaint();

		} //end of added by J. Redfern
		else if ( smacmode == 0 && smac != -1 )
		{
			AddMacViewUndo();

			switch(_pSong->_pMachine[smac]->_mode)
			{
				case MACHMODE_GENERATOR:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sGenerator.width > CW) 
					{ 
						_pSong->_pMachine[smac]->_x = CW-MachineCoords.sGenerator.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sGenerator.height > CH) 
					{ 
						_pSong->_pMachine[smac]->_y = CH-MachineCoords.sGenerator.height; 
						Repaint(); 
					}
					break;
				case MACHMODE_FX:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sEffect.width > CW) 
					{ 
						_pSong->_pMachine[smac]->_x = CW-MachineCoords.sEffect.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sEffect.height > CH) 
					{ 
						_pSong->_pMachine[smac]->_y = CH-MachineCoords.sEffect.height; 
						Repaint(); 
					}
					break;

				case MACHMODE_MASTER:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sMaster.width > CW) 
					{ 
						_pSong->_pMachine[smac]->_x = CW-MachineCoords.sMaster.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->_pMachine[smac]->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sMaster.height > CH) 
					{ 
						_pSong->_pMachine[smac]->_y = CH-MachineCoords.sMaster.height; 
						Repaint(); 
					}
					break;
			}

		}
		smac = -1;
		smacmode = 0;
		
		wiresource = -1;
		wiredest = -1;
		break;

	case VMPattern:
		if ( (blockStart) &&
			( point.y > YOFFSET && point.y < YOFFSET+(maxl*ROWHEIGHT)) &&
			(point.x > XOFFSET && point.x < XOFFSET+(maxt*ROWWIDTH)))
		{
			editcur.track = tOff + char((point.x-XOFFSET)/ROWWIDTH);
//			if ( editcur.track >= _pSong->SONGTRACKS ) editcur.track = _pSong->SONGTRACKS-1;
//			else if ( editcur.track < 0 ) editcur.track = 0;

//			int plines = _pSong->patternLines[_ps()];
			editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
//			if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
//			else if ( editcur.line < 0 ) editcur.line = 0;

			editcur.col = _xtoCol((point.x-XOFFSET)%ROWWIDTH);
			Repaint(DMCursor);
			pParentMain->StatusBarIdle();
			if (!(nFlags & MK_SHIFT))
			{
				blockSelected=false;
				blockSel.end.line=0;
				blockSel.end.track=0;
				ChordModeOffs = 0;
				bScrollDetatch=false;
				Repaint(DMSelection);
			}
		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

void CChildView::OnMouseMove( UINT nFlags, CPoint point )
{
	switch (viewMode)
	{
	case VMMachine:
		if (smac > -1 && (nFlags & MK_LBUTTON))
		{
			if (_pSong->_pMachine[smac])
			{
				if (smacmode == 0)
				{
					_pSong->_pMachine[smac]->_x = point.x-mcd_x;
					_pSong->_pMachine[smac]->_y = point.y-mcd_y;

					char buf[128];
					sprintf(buf, "%s (%d,%d)", Global::_pSong->_pMachine[smac]->_editName, Global::_pSong->_pMachine[smac]->_x, Global::_pSong->_pMachine[smac]->_y);
					pParentMain->StatusBarText(buf);
					Repaint();
				}
				else if ((smacmode == 1) && (Global::_pSong->_pMachine[smac]->_mode != MACHMODE_MASTER))
				{
					int newpan = 64;
					switch(Global::_pSong->_pMachine[smac]->_mode)
					{
					case MACHMODE_GENERATOR:
						newpan = (point.x - Global::_pSong->_pMachine[smac]->_x - MachineCoords.dGeneratorPan.x - (MachineCoords.sGeneratorPan.width/2))*128;
						if (MachineCoords.dGeneratorPan.width)
						{
							newpan /= MachineCoords.dGeneratorPan.width;
						}
						break;
					case MACHMODE_FX:
						newpan = (point.x - Global::_pSong->_pMachine[smac]->_x - MachineCoords.dEffectPan.x - (MachineCoords.sEffectPan.width/2))*128;
						if (MachineCoords.dEffectPan.width)
						{
							newpan /= MachineCoords.dEffectPan.width;
						}
						break;
					}

					if (Global::_pSong->_pMachine[smac]->_panning != newpan)
					{
						AddMacViewUndo();

						Global::_pSong->_pMachine[smac]->SetPan(newpan);
						newpan= Global::_pSong->_pMachine[smac]->_panning;
						
						char buf[128];
						if (newpan != 64)
						{
							sprintf(buf, "%s Pan: %.0f%% Left / %.0f%% Right", Global::_pSong->_pMachine[smac]->_editName, 100.0f - ((float)newpan*0.78125f), (float)newpan*0.78125f);
						}
						else
						{
							sprintf(buf, "%s Pan: Center", Global::_pSong->_pMachine[smac]->_editName);
						}
						pParentMain->StatusBarText(buf);
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
				}
			}
		}
		
		if ((nFlags == (MK_SHIFT | MK_LBUTTON)) && (wiresource != -1))
		{
			wireDX = point.x;
			wireDY = point.y;
			Repaint();
		}
				
		if ((nFlags == (MK_CONTROL | MK_LBUTTON)) && (wiredest != -1))
		{
			wireSX = point.x;
			wireSY = point.y;
			Repaint();
		}
		
		break;

	case VMPattern:

		if ((nFlags & MK_LBUTTON) && oldm.track != -1)
		{
			ntOff = tOff;
			nlOff = lOff;
			int paintmode = 0;

			int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
			if ( point.x < XOFFSET ) ttm--; // 1/2 = 0 , -1/2 = 0 too!
			int ccm;
			if ( ttm < tOff ) // Exceeded from left
			{
				ccm=0;
				if ( ttm < 0 ) { ttm = 0; } // Out of Range
				// and Scroll
				ntOff = ttm;
				if (ntOff != tOff) paintmode=DMHScroll;
			}
			else if ( ttm - tOff >= VISTRACKS ) // Exceeded from right
			{
				ccm=8;
				if ( ttm >= _pSong->SONGTRACKS ) // Out of Range
				{	
					ttm = _pSong->SONGTRACKS-1;
					if ( tOff != ttm-VISTRACKS ) 
					{ 
						ntOff = ttm-VISTRACKS+1; 
						paintmode=DMHScroll; 
					}
				}
				else	//scroll
				{	
					ntOff = ttm-VISTRACKS+1;
					if ( ntOff != tOff ) 
						paintmode=DMHScroll;
				}
			}
			else // Not exceeded
			{
				ccm=_xtoCol((point.x-XOFFSET)%ROWWIDTH);
			}

			int plines = _pSong->patternLines[_ps()];
			int llm = lOff + (point.y-YOFFSET)/ROWHEIGHT;
			if ( point.y < YOFFSET ) llm--; // 1/2 = 0 , -1/2 = 0 too!

			if ( llm < lOff ) // Exceeded from top
			{
				if ( llm < 0 ) // Out of range
				{	
					llm = 0;
					if ( lOff != 0 ) 
					{ 
						nlOff = 0; 
						paintmode=DMVScroll; 
					}
				}
				else	//scroll
				{	
					nlOff = llm;
					if ( nlOff != lOff ) 
						paintmode=DMVScroll;
				}
			}
			else if ( llm - lOff >= VISLINES ) // Exceeded from bottom
			{
				if ( llm >= plines ) //Out of Range
				{	
					llm = plines-1;
					if ( lOff != llm-VISLINES) 
					{ 
						nlOff = llm-VISLINES+1; 
						paintmode=DMVScroll; 
					}
				}
				else	//scroll
				{	
					nlOff = llm-VISLINES+1;
					if ( nlOff != lOff ) 
						paintmode=DMVScroll;
				}
			}
			
			else if ( llm >= plines ) { llm = plines-1; } //Out of Range

			if ((ttm != oldm.track ) || (llm != oldm.line) || (ccm != oldm.col))
			{
				if (blockStart) 
				{
					blockStart = FALSE;
					blockSelected=false;
					blockSel.end.line=0;
					blockSel.end.track=0;
					StartBlock(oldm.track,oldm.line,oldm.col);
				}
				ChangeBlock(ttm,llm,ccm);
				oldm.track=ttm;
				oldm.line=llm;
				oldm.col=ccm;
				paintmode=DMSelection;
			}

			bScrollDetatch=true;
			detatchpoint.track = ttm;
			detatchpoint.line = llm;
			detatchpoint.col = ccm;
			if (nFlags & MK_SHIFT)
			{
				editcur = detatchpoint;
				if (!paintmode)
				{
					paintmode=DMCursor;
				}
			}

			if (paintmode)
			{
				Repaint(paintmode);
			}
		}
		else if (nFlags == MK_MBUTTON)
		{
			// scrolling
			if (abs(point.y - MBStart.y) > ROWHEIGHT)
			{
				int nlines = _pSong->patternLines[_ps()];
				int delta = (point.y - MBStart.y)/ROWHEIGHT;
				int nPos = lOff - delta;
				if (nPos > lOff )
				{
					if (nPos < 0)
						nPos = 0;
					else if (nPos > nlines-VISLINES)
						nlOff = nlines-VISLINES;
					else
						nlOff=nPos;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				else if (nPos < lOff )
				{
					if (nPos < 0)
						nlOff = 0;
					else if (nPos > nlines-VISLINES)
						nlOff = nlines-VISLINES;
					else
						nlOff=nPos;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				MBStart.y += delta*ROWHEIGHT;
			}
			// switching tracks
			if (abs(point.x - MBStart.x) > (ROWWIDTH))
			{
				int delta = (point.x - MBStart.x)/(ROWWIDTH);
				int nPos = tOff - delta;
				if (nPos > tOff)
				{
					if (nPos < 0)
						ntOff= 0;
					else if (nPos>_pSong->SONGTRACKS-VISTRACKS)
						ntOff=_pSong->SONGTRACKS-VISTRACKS;
					else
						ntOff=nPos;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMHScroll);
				}
				else if (nPos < tOff)
				{
					if (nPos < 0)
						ntOff= 0;
					else if (nPos>_pSong->SONGTRACKS-VISTRACKS)
						ntOff=_pSong->SONGTRACKS-VISTRACKS;
					else
						ntOff=nPos;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMHScroll);
				}
				MBStart.x += delta*ROWWIDTH;
			}
		}
		break;
	}//<-- End LBUTTONPRESING/VIEWMODE switch statement
}

//////////////////////////////////////////////////////////////////////
// Double Click Handler

void CChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	int tmac=-1;
	
	switch (viewMode)
	{
		case VMMachine: // User is in machine view mode
		
			tmac = GetMachine(point);

			if(tmac>-1)
			{
				switch (Global::_pSong->_pMachine[tmac]->_mode)
				{
				case MACHMODE_GENERATOR:
					if ((mcd_x >= MachineCoords.dGeneratorPan.x) && 
						(mcd_x < MachineCoords.dGeneratorPan.x+MachineCoords.dGeneratorPan.width) && 
						(mcd_y >= MachineCoords.dGeneratorPan.y) && 
						(mcd_y < MachineCoords.dGeneratorPan.y+MachineCoords.sGeneratorPan.height)) //changing panning
					{
						smac=tmac;
						smacmode = 1;
						OnMouseMove(nFlags,point);
						return;
					}
					else if ((mcd_x >= MachineCoords.dGeneratorMute.x) && 
							(mcd_x < MachineCoords.dGeneratorMute.x+MachineCoords.sGeneratorMute.width) &&
							(mcd_y >= MachineCoords.dGeneratorMute.y) && 
							(mcd_y < MachineCoords.dGeneratorMute.y+MachineCoords.sGeneratorMute.height)) //Mute 
					{
						Global::_pSong->_pMachine[tmac]->_mute = !Global::_pSong->_pMachine[tmac]->_mute;
						if (Global::_pSong->_pMachine[tmac]->_mute)
						{
							Global::_pSong->_pMachine[tmac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[tmac]->_volumeDisplay=0;
							if (Global::_pSong->machineSoloed == tmac )
							{
								Global::_pSong->machineSoloed = -1;
							}
						}
						updatePar = tmac;
						Repaint(DMMacRefresh);
						return;
					}
					else if ((mcd_x >= MachineCoords.dGeneratorSolo.x) && 
							(mcd_x < MachineCoords.dGeneratorSolo.x+MachineCoords.sGeneratorSolo.width) &&
							(mcd_y >= MachineCoords.dGeneratorSolo.y) && 
							(mcd_y < MachineCoords.dGeneratorSolo.y+MachineCoords.sGeneratorSolo.height)) //Solo 
					{
						if (Global::_pSong->machineSoloed == tmac )
						{
							Global::_pSong->machineSoloed = -1;
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->_pMachine[i] )
								{
									if ( Global::_pSong->_pMachine[i]->_mode == MACHMODE_GENERATOR )
									{
										Global::_pSong->_pMachine[i]->_mute = false;
									}
								}
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->_pMachine[i] ) 
								{
									if (( Global::_pSong->_pMachine[i]->_mode == MACHMODE_GENERATOR ) && (i != tmac))
									{
										Global::_pSong->_pMachine[i]->_mute = true;
										Global::_pSong->_pMachine[i]->_volumeCounter=0.0f;
										Global::_pSong->_pMachine[i]->_volumeDisplay=0;
									}
								}
							}
							Global::_pSong->_pMachine[tmac]->_mute = false;
							Global::_pSong->machineSoloed = tmac;
						}
						updatePar = tmac;
						Repaint(DMAllMacsRefresh);
						return;
					}
					break;
				case MACHMODE_FX:
					if ((mcd_x >= MachineCoords.dEffectPan.x) && 
						(mcd_x < MachineCoords.dEffectPan.x+MachineCoords.dEffectPan.width) && 
						(mcd_y >= MachineCoords.dEffectPan.y) && 
						(mcd_y < MachineCoords.dEffectPan.y+MachineCoords.sEffectPan.height)) //changing panning
					{
						smac=tmac;
						smacmode = 1;
						OnMouseMove(nFlags,point);
						return;
					}
					else if ((mcd_x >= MachineCoords.dEffectMute.x) && 
							(mcd_x < MachineCoords.dEffectMute.x+MachineCoords.sEffectMute.width) &&
							(mcd_y >= MachineCoords.dEffectMute.y) && 
							(mcd_y < MachineCoords.dEffectMute.y+MachineCoords.sEffectMute.height)) //Mute 
					{
						Global::_pSong->_pMachine[tmac]->_mute = !Global::_pSong->_pMachine[tmac]->_mute;
						if (Global::_pSong->_pMachine[tmac]->_mute)
						{
							Global::_pSong->_pMachine[tmac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[tmac]->_volumeDisplay=0;
						}
						updatePar = tmac;
						Repaint(DMMacRefresh);
						return;
					}
					else if ((mcd_x >= MachineCoords.dEffectBypass.x) && 
							(mcd_x < MachineCoords.dEffectBypass.x+MachineCoords.sEffectBypass.width) &&
							(mcd_y >= MachineCoords.dEffectBypass.y) && 
							(mcd_y < MachineCoords.dEffectBypass.y+MachineCoords.sEffectBypass.height)) //Solo 
					{
						Global::_pSong->_pMachine[tmac]->_bypass = !Global::_pSong->_pMachine[tmac]->_bypass;
						if (Global::_pSong->_pMachine[tmac]->_bypass)
						{
							Global::_pSong->_pMachine[tmac]->_volumeCounter=0.0f;
							Global::_pSong->_pMachine[tmac]->_volumeDisplay=0;
						}
						updatePar = tmac;
						Repaint(DMMacRefresh);
						return;
					}
					break;

				case MACHMODE_MASTER:
					break;
				}
				pParentMain->ShowMachineGui(tmac, point);
//					Repaint();
			}
			else
			{
				// Check for pressed connection

				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = Global::_pSong->_pMachine[c];
					if (tmac)
					{
						for (int w = 0; w<MAX_CONNECTIONS; w++)
						{
							if (tmac->_connection[w])
							{
								int xt = tmac->_connectionPoint[w].x;
								int yt = tmac->_connectionPoint[w].y;
								
								if ((point.x > xt) && (point.x < xt+triangle_size_tall) && (point.y > yt) && (point.y < yt+triangle_size_tall))
								{
									for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
									{
										if (WireDialog[i])
										{
											if ((WireDialog[i]->_pSrcMachine == tmac) &&
												(WireDialog[i]->_pDstMachine == Global::_pSong->_pMachine[tmac->_outputMachines[w]]))
											{
												return;
											}
										}
									}
									for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
									{
										if (!WireDialog[i])
										{
											WireDialog[i] = new CWireDlg(this);
											WireDialog[i]->this_index = i;
											WireDialog[i]->wireIndex = w;
											WireDialog[i]->isrcMac = c;
											WireDialog[i]->_pSrcMachine = tmac;
											WireDialog[i]->_pDstMachine = Global::_pSong->_pMachine[tmac->_outputMachines[w]];
											WireDialog[i]->Create();
											pParentMain->CenterWindowOnPoint(WireDialog[i], point);
											WireDialog[i]->ShowWindow(SW_SHOW);
											return;
										}
									}
								}
							}
						}
					}
				}
				// if no connection then Show new machine dialog
				NewMachine(point.x,point.y);
//				Repaint();
			}
		
			break;

			////////////////////////////////////////////////////////////////

		case VMPattern: // User is in pattern view mode
			// select track
			if (( point.y >= YOFFSET ) && (point.x >= XOFFSET))
			{
				const int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
				const int nl = _pSong->patternLines[_pSong->playOrder[editPosition]];

				StartBlock(ttm,0,0);
				EndBlock(ttm,nl-1,8);
				blockStart = FALSE;
			}

			break;


	} // <-- End switch(viewMode)
}



BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if ( viewMode == VMPattern )
	{
		int nlines = _pSong->patternLines[_ps()];
		int nPos = lOff - (zDelta/30);
		if (nPos > lOff )
		{
			if (nPos < 0)
				nPos = 0;
			else if (nPos > nlines-VISLINES)
				nlOff = nlines-VISLINES;
			else
				nlOff=nPos;
			bScrollDetatch=true;
			detatchpoint.track = ntOff+1;
			detatchpoint.line = nlOff+1;
			Repaint(DMVScroll);
		}
		else if (nPos < lOff )
		{
			if (nPos < 0)
				nlOff = 0;
			else if (nPos > nlines-VISLINES)
				nlOff = nlines-VISLINES;
			else
				nlOff=nPos;
			bScrollDetatch=true;
			detatchpoint.track = ntOff+1;
			detatchpoint.line = nlOff+1;
			Repaint(DMVScroll);
		}
	}
	return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnMButtonDown( UINT nFlags, CPoint point )
{
	MBStart.x = point.x;
	MBStart.y = point.y;
	CWnd ::OnMButtonDown(nFlags, point);
}

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINEDOWN:
				if ( lOff<_pSong->patternLines[_ps()]-VISLINES)
				{
					nlOff=lOff+1;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				break;
			case SB_LINEUP:
				if ( lOff>0 )
				{
					nlOff=lOff-1;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				break;
			case SB_PAGEDOWN:
				if ( lOff<_pSong->patternLines[_ps()]-VISLINES)
				{
					const int nl = _pSong->patternLines[_ps()];
					nlOff=lOff+16;
					if (nlOff >= nl)
					{
						nlOff = nl-1;
					}
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				break;
			case SB_PAGEUP:
				if ( lOff>0)
				{
					nlOff=lOff-16;
					if (nlOff < 0)
					{
						nlOff = 0;
					}
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				if (nlOff!=(int)nPos)
				{
					const int nl = _pSong->patternLines[_ps()];
					nlOff=(int)nPos;
					if (nlOff >= nl)
					{
						nlOff = nl-1;
					}
					else if (nlOff < 0)
					{
						nlOff = 0;
					}
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMVScroll);
				}
				break;
			default: 
				break;
		}
	}
	CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINERIGHT:
			case SB_PAGERIGHT:
				if ( tOff<_pSong->SONGTRACKS-VISTRACKS)
				{
					ntOff=tOff+1;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMHScroll);
				}
				break;
			case SB_LINELEFT:
			case SB_PAGELEFT:
				if ( tOff>0 )
				{
					ntOff=tOff-1;
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMHScroll);
				}
				else PrevTrack(1,false);
				break;
			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				if (ntOff!=(int)nPos)
				{
					const int nt = _pSong->SONGTRACKS;
					ntOff=(int)nPos;
					if (ntOff >= nt)
					{
						ntOff = nt-1;
					}
					else if (ntOff < 0)
					{
						ntOff = 0;
					}
					bScrollDetatch=true;
					detatchpoint.track = ntOff+1;
					detatchpoint.line = nlOff+1;
					Repaint(DMHScroll);
				}
				break;
			default: 
				break;
		}
	}

	CWnd ::OnHScroll(nSBCode, nPos, pScrollBar);
}


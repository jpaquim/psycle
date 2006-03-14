

void CPsycleWTLView::OnRButtonDown( UINT nFlags, CPoint point )
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
				Machine *tmac = Global::_pSong->pMachine(c);
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
											(WireDialog[i]->_pDstMachine == Global::_pSong->pMachine(tmac->_outputMachines[w])))
										{
											return;
										}
									}
								}
								for (i = 0; i < MAX_WIRE_DIALOGS; i++)
								{
									if (!WireDialog[i])
									{
										WireDialog[i] = new CWireDlg(this);
										WireDialog[i]->this_index = i;
										WireDialog[i]->wireIndex = w;
										WireDialog[i]->isrcMac = c;
										WireDialog[i]->_pSrcMachine = tmac;
										WireDialog[i]->_pDstMachine = Global::_pSong->pMachine(tmac->_outputMachines[w]);
										WireDialog[i]->Create(*this);
										pMainFrame->CenterWindowOnPoint((HWND)(WireDialog[i]), point);
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
		if ( editcur.track >= _pSong->SongTracks() ) editcur.track = _pSong->SongTracks()-1;
		else if ( editcur.track < 0 ) editcur.track = 0;

		int plines = _pSong->PatternLines(_ps());
		editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
		if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
		else if ( editcur.line < 0 ) editcur.line = 0;

		editcur.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);
	}
*/
}

void CPsycleWTLView::OnContextMenu(HWND hWnd, CPoint& point) 
{
	if (viewMode == VMPattern)
	{
		WTL::CMenu _menu;
		WTL::CMenuItemInfo info;
		_menu.LoadMenu(IDR_POPUPMENU);
		WTL::CMenu _popup(_menu.GetSubMenu(0));
		
		ATLASSERT(_popup != NULL);

		// ƒƒjƒ…[‚Ì‰Šú‰»
		// UNDO
		if(pUndoList) 
		{
			switch (pUndoList->type)
			{
			case UNDO_SEQUENCE:
				_menu.EnableMenuItem(ID_EDIT_UNDO,MF_ENABLED);
	//			_menu.SetUISetText(ID_EDIT_UNDO,RES_STR(IDS_UNDO));
				break;
			default:
				if(viewMode == VMPattern)// && bEditMode)
				{
					_menu.EnableMenuItem(ID_EDIT_UNDO,MF_ENABLED);
				}
				else
				{
					_menu.EnableMenuItem(ID_EDIT_UNDO,MF_GRAYED);
				}
				break;
			}
		}
		else
		{
			_menu.EnableMenuItem(ID_EDIT_UNDO,MF_GRAYED);
		}
		// REDO
		if(pRedoList) 
		{
			switch (pRedoList->type)
			{
			case UNDO_SEQUENCE:
				_menu.EnableMenuItem(ID_EDIT_REDO,MF_ENABLED);
				break;
			default:
				if(viewMode == VMPattern)// && bEditMode)
				{
					_menu.EnableMenuItem(ID_EDIT_REDO,MF_ENABLED);
				}
				else
				{
					_menu.EnableMenuItem(ID_EDIT_REDO,MF_GRAYED);
				}
				break;
			}
		}
		else
		{
			_menu.EnableMenuItem(ID_EDIT_REDO,MF_GRAYED);
		}

		_menu.EnableMenuItem(ID_EDIT_CUT,(viewMode == VMPattern)?MF_ENABLED:MF_GRAYED);
		_menu.EnableMenuItem(ID_EDIT_COPY,(viewMode == VMPattern)?MF_ENABLED:MF_GRAYED);
		_menu.EnableMenuItem(ID_EDIT_DELETE,(viewMode == VMPattern)?MF_ENABLED:MF_GRAYED);
		_menu.EnableMenuItem(ID_EDIT_MIXPASTE,(patBufferCopy&&(viewMode == VMPattern))?MF_ENABLED:MF_GRAYED);
		_menu.EnableMenuItem(ID_EDIT_PASTE,(patBufferCopy&&(viewMode == VMPattern))?MF_ENABLED:MF_GRAYED);

		
		UINT bCutCopy =  blockSelected && (viewMode == VMPattern)?MF_ENABLED:MF_GRAYED;

		_menu.EnableMenuItem(ID_POP_CUT,bCutCopy);
		_menu.EnableMenuItem(ID_POP_COPY,bCutCopy);
		_menu.EnableMenuItem(ID_POP_PASTE,(isBlockCopied && (viewMode == VMPattern))?MF_ENABLED:MF_GRAYED);
		_menu.EnableMenuItem(ID_POP_DELETE, bCutCopy);
		_menu.EnableMenuItem(ID_POP_INTERPOLATE, bCutCopy);
		_menu.EnableMenuItem(ID_POP_CHANGEGENERATOR, bCutCopy);
		_menu.EnableMenuItem(ID_POP_CHANGEINSTRUMENT, bCutCopy);
		_menu.EnableMenuItem(ID_POP_TRANSPOSE1, bCutCopy);
		_menu.EnableMenuItem(ID_POP_TRANSPOSE12, bCutCopy);
		_menu.EnableMenuItem(ID_POP_TRANSPOSE_1, bCutCopy);
		_menu.EnableMenuItem(ID_POP_TRANSPOSE_12, bCutCopy);
		_menu.EnableMenuItem(ID_POP_BLOCK_SWINGFILL, bCutCopy);
		_menu.EnableMenuItem(ID_POP_MIXPASTE,(isBlockCopied && (viewMode == VMPattern))?MF_ENABLED:MF_GRAYED);
		
		_popup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
		_menu.DestroyMenu();

//		Repaint(DMCursor);
	}
}


void CPsycleWTLView::OnLButtonDown( UINT nFlags, CPoint point )
{
	
	SetCapture();

	switch(viewMode)
	{
		
	case VMMachine: // User is in machine view mode
		if (Global::_pSong->IsMachineLock())
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
				mcd_x = point.x - Global::_pSong->pMachine(smac)->_x;
				mcd_y = point.y - Global::_pSong->pMachine(smac)->_y;
			}

			_pSong->SeqBus(_pSong->FindBusFromIndex(smac));
			pMainFrame->UpdateComboGen();
		}
		else if (nFlags & MK_SHIFT)
		{
			wiresource = GetMachine(point);
			wiremove = -1;
			if (wiresource == -1)
			{
				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = Global::_pSong->pMachine(c);
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
				switch (Global::_pSong->pMachine(wiresource)->_mode)
				{
				case MACHMODE_GENERATOR:
					wireSX = Global::_pSong->pMachine(wiresource)->_x+(MachineCoords.sGenerator.width/2);
					wireSY = Global::_pSong->pMachine(wiresource)->_y+(MachineCoords.sGenerator.height/2);
					break;
				case MACHMODE_FX:
					wireSX = Global::_pSong->pMachine(wiresource)->_x+(MachineCoords.sEffect.width/2);
					wireSY = Global::_pSong->pMachine(wiresource)->_y+(MachineCoords.sEffect.height/2);
					break;

				case MACHMODE_MASTER:
					wireSX = Global::_pSong->pMachine(wiresource)->_x+(MachineCoords.sMaster.width/2);
					wireSY = Global::_pSong->pMachine(wiresource)->_y+(MachineCoords.sMaster.height/2);
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
				mcd_x = point.x - Global::_pSong->pMachine(smac)->_x;
				mcd_y = point.y - Global::_pSong->pMachine(smac)->_y;

				int panning;

				switch (Global::_pSong->pMachine(smac)->_mode)
				{
				case MACHMODE_GENERATOR:
					panning = Global::_pSong->pMachine(smac)->_panning*MachineCoords.dGeneratorPan.width;
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
						Global::_pSong->pMachine(smac)->_mute = !Global::_pSong->pMachine(smac)->_mute;
						if (Global::_pSong->pMachine(smac)->_mute)
						{
							Global::_pSong->pMachine(smac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(smac)->_volumeDisplay=0;
							if (Global::_pSong->MachineSoloed() == smac )
							{
								Global::_pSong->MachineSoloed(-1);
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
						if (Global::_pSong->MachineSoloed() == smac )
						{
							Global::_pSong->MachineSoloed(-1);
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->pMachine(i) )
								{
									if (( Global::_pSong->pMachine(i)->_mode == MACHMODE_GENERATOR ))
									{
										Global::_pSong->pMachine(i)->_mute = false;
									}
								}
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->pMachine(i) )
								{
									if (( Global::_pSong->pMachine(i)->_mode == MACHMODE_GENERATOR ) && (i != smac))
									{
										Global::_pSong->pMachine(i)->_mute = true;
										Global::_pSong->pMachine(i)->_volumeCounter=0.0f;
										Global::_pSong->pMachine(i)->_volumeDisplay=0;
									}
								}
							}
							Global::_pSong->pMachine(smac)->_mute = false;
							Global::_pSong->MachineSoloed(smac);
						}
						updatePar = smac;
						Repaint(DMAllMacsRefresh);
					}
					break;
				case MACHMODE_FX:
					panning = Global::_pSong->pMachine(smac)->_panning*MachineCoords.dEffectPan.width;
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
						Global::_pSong->pMachine(smac)->_mute = !Global::_pSong->pMachine(smac)->_mute;
						if (Global::_pSong->pMachine(smac)->_mute)
						{
							Global::_pSong->pMachine(smac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(smac)->_volumeDisplay=0;
						}
						updatePar = smac;
						Repaint(DMMacRefresh);
					}
					else if ((mcd_x >= MachineCoords.dEffectBypass.x) && 
							(mcd_x < MachineCoords.dEffectBypass.x+MachineCoords.sEffectBypass.width) &&
							(mcd_y >= MachineCoords.dEffectBypass.y) && 
							(mcd_y < MachineCoords.dEffectBypass.y+MachineCoords.sEffectBypass.height)) //Solo 
					{
						Global::_pSong->pMachine(smac)->_bypass = !Global::_pSong->pMachine(smac)->_bypass;
						if (Global::_pSong->pMachine(smac)->_bypass)
						{
							Global::_pSong->pMachine(smac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(smac)->_volumeDisplay=0;
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
		if ( ttm >= _pSong->SongTracks() ) ttm = _pSong->SongTracks()-1;
		else if ( ttm < 0 ) ttm = 0;
		
		if (point.y >= 0 && point.y < YOFFSET ) // Mouse is in Track Header.
		{	
			int pointpos= ((point.x-XOFFSET)%ROWWIDTH) - HEADER_INDENT;

			if ((pointpos >= PatHeaderCoords.dRecordOn.x) && 
				(pointpos < PatHeaderCoords.dRecordOn.x+PatHeaderCoords.sRecordOn.width) &&
				(point.y >= PatHeaderCoords.dRecordOn.y+1) &&
				(point.y < PatHeaderCoords.dRecordOn.y+1+PatHeaderCoords.sRecordOn.height))
			{
				_pSong->IsTrackArmed(ttm,_pSong->IsTrackArmed(ttm));
				_pSong->TrackArmedCount(0);
				for ( int i=0;i<MAX_TRACKS;i++ )
				{
					if (_pSong->IsTrackArmed(i))
					{
						_pSong->TrackArmedCount(_pSong->TrackArmedCount() + 1);
					}
				}
			}
			else if ((pointpos >= PatHeaderCoords.dMuteOn.x) && 
				(pointpos < PatHeaderCoords.dMuteOn.x+PatHeaderCoords.sMuteOn.width) &&
				(point.y >= PatHeaderCoords.dMuteOn.y+1) &&
				(point.y < PatHeaderCoords.dMuteOn.y+1+PatHeaderCoords.sMuteOn.height))
			{
				_pSong->IsTrackMuted(ttm,!_pSong->IsTrackMuted(ttm));
			}
			else if ((pointpos >= PatHeaderCoords.dSoloOn.x) && 
				(pointpos < PatHeaderCoords.dSoloOn.x+PatHeaderCoords.sSoloOn.width) &&
				(point.y >= PatHeaderCoords.dSoloOn.y+1) &&
				(point.y < PatHeaderCoords.dSoloOn.y+1+PatHeaderCoords.sSoloOn.height))
			{
				if (Global::_pSong->TrackSoloed() != ttm )
				{
					for ( int i = 0;i < MAX_TRACKS;i++ )
					{
						_pSong->IsTrackMuted(i,true);
					}
					_pSong->IsTrackMuted(ttm,false);
					_pSong->TrackSoloed(ttm);
				}
				else
				{
					for ( int i=0;i<MAX_TRACKS;i++ )
					{
						_pSong->IsTrackMuted(i,false);
					}
					_pSong->TrackSoloed(-1);
				}
			}
			oldm.track = -1;
			Repaint(DMTrackHeader);
			break;
		}
		else if ( point.y >= YOFFSET )
		{
			oldm.track=ttm;

			int plines = _pSong->PatternLines(_ps());
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

void CPsycleWTLView::OnLButtonUp( UINT nFlags, CPoint point )
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
					int dm,w;
					float volume = 1.0f;

					if (Global::_pSong->pMachine(wiresource))
					{
						dm = Global::_pSong->pMachine(wiresource)->_outputMachines[wiremove];

						if (Global::_pSong->pMachine(dm))
						{
							w = Global::_pSong->pMachine(dm)->FindInputWire(wiresource);
							Global::_pSong->pMachine(dm)->GetWireVolume(w,volume);
							if (Global::_pSong->InsertConnection(wiresource, wiredest,volume))
							{
								// delete the old wire
								Global::_pSong->pMachine(wiresource)->_connection[wiremove] = FALSE;
								Global::_pSong->pMachine(wiresource)->_numOutputs--;

								Global::_pSong->pMachine(dm)->_inputCon[w] = FALSE;
								Global::_pSong->pMachine(dm)->_numInputs--;
							}
							else
							{
								MessageBox(SF::CResourceString(IDS_ERR_MSG0061),SF::CResourceString(IDS_ERR_MSG0043), MB_ICONERROR);
							}
						}
					}
				}
				// or making a new one?
				else if (!Global::_pSong->InsertConnection(wiresource, wiredest))
				{
					MessageBox(SF::CResourceString(IDS_ERR_MSG0061),SF::CResourceString(IDS_ERR_MSG0043), MB_ICONERROR);
				}

			}
			wiresource = -1;
			Repaint();
		}
		else if ( smacmode == 0 && smac != -1 )
		{
			AddMacViewUndo();

			switch(_pSong->pMachine(smac)->_mode)
			{
				case MACHMODE_GENERATOR:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->pMachine(smac)->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sGenerator.width > CW) 
					{ 
						_pSong->pMachine(smac)->_x = CW-MachineCoords.sGenerator.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->pMachine(smac)->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sGenerator.height > CH) 
					{ 
						_pSong->pMachine(smac)->_y = CH-MachineCoords.sGenerator.height; 
						Repaint(); 
					}
					break;
				case MACHMODE_FX:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->pMachine(smac)->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sEffect.width > CW) 
					{ 
						_pSong->pMachine(smac)->_x = CW-MachineCoords.sEffect.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->pMachine(smac)->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sEffect.height > CH) 
					{ 
						_pSong->pMachine(smac)->_y = CH-MachineCoords.sEffect.height; 
						Repaint(); 
					}
					break;

				case MACHMODE_MASTER:
					if (point.x-mcd_x < 0 ) 
					{ 
						_pSong->pMachine(smac)->_x = 0; 
						Repaint(); 
					}
					else if	(point.x-mcd_x+MachineCoords.sMaster.width > CW) 
					{ 
						_pSong->pMachine(smac)->_x = CW-MachineCoords.sMaster.width; 
						Repaint(); 
					}
					if (point.y-mcd_y < 0 ) 
					{ 
						_pSong->pMachine(smac)->_y = 0; 
						Repaint(); 
					}
					else if (point.y-mcd_y+MachineCoords.sMaster.height > CH) 
					{ 
						_pSong->pMachine(smac)->_y = CH-MachineCoords.sMaster.height; 
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
//			if ( editcur.track >= _pSong->SongTracks() ) editcur.track = _pSong->SongTracks()-1;
//			else if ( editcur.track < 0 ) editcur.track = 0;

//			int plines = _pSong->PatternLines(_ps());
			editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
//			if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
//			else if ( editcur.line < 0 ) editcur.line = 0;

			editcur.col = _xtoCol((point.x-XOFFSET)%ROWWIDTH);
			Repaint(DMCursor);
			pMainFrame->StatusBarIdle();
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

void CPsycleWTLView::OnMouseMove( UINT nFlags, CPoint point )
{
	switch (viewMode)
	{
	case VMMachine:
		if (smac > -1 && (nFlags & MK_LBUTTON))
		{
			if (_pSong->pMachine(smac))
			{
				if (smacmode == 0)
				{
					_pSong->pMachine(smac)->_x = point.x-mcd_x;
					_pSong->pMachine(smac)->_y = point.y-mcd_y;
					pMainFrame->StatusBarText(
						(SF::tformat(_T("%s (%d,%d)")) 
							% Global::_pSong->pMachine(smac)->_editName 
							% Global::_pSong->pMachine(smac)->_x 
							% Global::_pSong->pMachine(smac)->_y
						).str().c_str()
					);
					Repaint();
				}
				else if ((smacmode == 1) && (Global::_pSong->pMachine(smac)->_mode != MACHMODE_MASTER))
				{
					int newpan = 64;
					switch(Global::_pSong->pMachine(smac)->_mode)
					{
					case MACHMODE_GENERATOR:
						newpan = (point.x - Global::_pSong->pMachine(smac)->_x - MachineCoords.dGeneratorPan.x - (MachineCoords.sGeneratorPan.width/2))*128;
						if (MachineCoords.dGeneratorPan.width)
						{
							newpan /= MachineCoords.dGeneratorPan.width;
						}
						break;
					case MACHMODE_FX:
						newpan = (point.x - Global::_pSong->pMachine(smac)->_x - MachineCoords.dEffectPan.x - (MachineCoords.sEffectPan.width/2))*128;
						if (MachineCoords.dEffectPan.width)
						{
							newpan /= MachineCoords.dEffectPan.width;
						}
						break;
					}

					if (Global::_pSong->pMachine(smac)->_panning != newpan)
					{
						AddMacViewUndo();

						Global::_pSong->pMachine(smac)->SetPan(newpan);
						newpan= Global::_pSong->pMachine(smac)->_panning;
						
						TCHAR buf[256];
						if (newpan != 64)
						{
							_stprintf(buf, m_FmtPan, Global::_pSong->pMachine(smac)->_editName, 100.0f - ((float)newpan*0.78125f), (float)newpan*0.78125f);
						}
						else
						{
							_stprintf(buf, m_FmtPan1, Global::_pSong->pMachine(smac)->_editName);
						}
						pMainFrame->StatusBarText(
							buf
						);
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
				if ( ttm >= _pSong->SongTracks() ) // Out of Range
				{	
					ttm = _pSong->SongTracks()-1;
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

			int plines = _pSong->PatternLines(_ps());
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
				int nlines = _pSong->PatternLines(_ps());
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
					else if (nPos>_pSong->SongTracks()-VISTRACKS)
						ntOff=_pSong->SongTracks()-VISTRACKS;
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
					else if (nPos>_pSong->SongTracks()-VISTRACKS)
						ntOff=_pSong->SongTracks()-VISTRACKS;
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

void CPsycleWTLView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	int tmac=-1;
	
	switch (viewMode)
	{
		case VMMachine: // User is in machine view mode
		
			tmac = GetMachine(point);

			if(tmac>-1)
			{
				switch (Global::_pSong->pMachine(tmac)->_mode)
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
						Global::_pSong->pMachine(tmac)->_mute = !Global::_pSong->pMachine(tmac)->_mute;
						if (Global::_pSong->pMachine(tmac)->_mute)
						{
							Global::_pSong->pMachine(tmac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(tmac)->_volumeDisplay=0;
							if (Global::_pSong->MachineSoloed() == tmac )
							{
								Global::_pSong->MachineSoloed(-1);
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
						if (Global::_pSong->MachineSoloed() == tmac )
						{
							Global::_pSong->MachineSoloed(-1);
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->pMachine(i) )
								{
									if ( Global::_pSong->pMachine(i)->_mode == MACHMODE_GENERATOR )
									{
										Global::_pSong->pMachine(i)->_mute = false;
									}
								}
							}
						}
						else 
						{
							for ( int i=0;i<MAX_MACHINES;i++ )
							{
								if ( Global::_pSong->pMachine(i) ) 
								{
									if (( Global::_pSong->pMachine(i)->_mode == MACHMODE_GENERATOR ) && (i != tmac))
									{
										Global::_pSong->pMachine(i)->_mute = true;
										Global::_pSong->pMachine(i)->_volumeCounter=0.0f;
										Global::_pSong->pMachine(i)->_volumeDisplay=0;
									}
								}
							}
							Global::_pSong->pMachine(tmac)->_mute = false;
							Global::_pSong->MachineSoloed(tmac);
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
						Global::_pSong->pMachine(tmac)->_mute = !Global::_pSong->pMachine(tmac)->_mute;
						if (Global::_pSong->pMachine(tmac)->_mute)
						{
							Global::_pSong->pMachine(tmac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(tmac)->_volumeDisplay=0;
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
						Global::_pSong->pMachine(tmac)->_bypass = !Global::_pSong->pMachine(tmac)->_bypass;
						if (Global::_pSong->pMachine(tmac)->_bypass)
						{
							Global::_pSong->pMachine(tmac)->_volumeCounter=0.0f;
							Global::_pSong->pMachine(tmac)->_volumeDisplay=0;
						}
						updatePar = tmac;
						Repaint(DMMacRefresh);
						return;
					}
					break;

				case MACHMODE_MASTER:
					break;
				}
				pMainFrame->ShowMachineGui(tmac, point);
//					Repaint();
			}
			else
			{
				// Check for pressed connection

				for (int c=0; c<MAX_MACHINES; c++)
				{
					Machine *tmac = Global::_pSong->pMachine(c);
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
												(WireDialog[i]->_pDstMachine == Global::_pSong->pMachine(tmac->_outputMachines[w])))
											{
												return;
											}
										}
									}
									for (i = 0; i < MAX_WIRE_DIALOGS; i++)
									{
										if (!WireDialog[i])
										{
											WireDialog[i] = new CWireDlg(this);
											WireDialog[i]->this_index = i;
											WireDialog[i]->wireIndex = w;
											WireDialog[i]->isrcMac = c;
											WireDialog[i]->_pSrcMachine = tmac;
											WireDialog[i]->_pDstMachine = 
												Global::_pSong->pMachine(tmac->_outputMachines[w]);
											WireDialog[i]->Create((HWND)(*this));
											pMainFrame->CenterWindowOnPoint((HWND)(*(WireDialog[i])), point);
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
				const int nl = _pSong->PatternLines(_pSong->PlayOrder(editPosition));

				StartBlock(ttm,0,0);
				EndBlock(ttm,nl-1,8);
				blockStart = FALSE;
			}

			break;


	} // <-- End switch(viewMode)
}



BOOL CPsycleWTLView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if ( viewMode == VMPattern )
	{
		int nlines = _pSong->PatternLines(_ps());
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
	SetMsgHandled(FALSE);
	return 0;
}

void CPsycleWTLView::OnMButtonDown( UINT nFlags, CPoint& point )
{
	MBStart.x = point.x;
	MBStart.y = point.y;
	SetMsgHandled(FALSE);
}

void CPsycleWTLView::OnVScroll(UINT nSBCode, UINT nPos, HWND hScrollBar) 
{
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINEDOWN:
				if ( lOff<_pSong->PatternLines(_ps())-VISLINES)
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
				if ( lOff<_pSong->PatternLines(_ps())-VISLINES)
				{
					const int nl = _pSong->PatternLines(_ps());
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
					const int nl = _pSong->PatternLines(_ps());
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
	SetMsgHandled(FALSE);
}


void CPsycleWTLView::OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar) 
{
	WTL::CScrollBar CScrollBar(hScrollBar);
	if ( viewMode == VMPattern )
	{
		switch(nSBCode)
		{
			case SB_LINERIGHT:
			case SB_PAGERIGHT:
				if ( tOff<_pSong->SongTracks()-VISTRACKS)
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
					const int nt = _pSong->SongTracks();
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
	SetMsgHandled(FALSE);
}


///\file
///\brief pointer handler for psycle::host::CChildView, private header
#include "internal_machines.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		void CChildView::OnRButtonDown( UINT nFlags, CPoint point )
		{	
			//Right mouse button behaviour (OnRButtonDown() and OnRButtonUp()) extended by sampler.
			SetCapture();			
  			if(viewMode == view_modes::machine) // User is in machine view mode
			{
				if (_pSong->_machineLock)
					return;

				PsycleCanvas::Event ev;
				ev.type = PsycleCanvas::Event::BUTTON_PRESS;
				ev.x = point.x;
				ev.y = point.y;
				ev.button = 3;
				ev.shift = nFlags;
				machine_view_.OnEvent(&ev);
				return;
			}
			CWnd::OnRButtonDown(nFlags,point);
		}
		
		void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
		{
			ReleaseCapture();
			if (viewMode == view_modes::machine)
			{
				PsycleCanvas::Event ev;
				ev.type = PsycleCanvas::Event::BUTTON_RELEASE;
				ev.x = point.x;
				ev.y = point.y;
				ev.button = 3;
				ev.shift = nFlags;
				machine_view_.OnEvent(&ev);
				return;
			}
			Repaint(); // For what is this needed ?
			CWnd::OnRButtonUp(nFlags,point);
		}

		void CChildView::OnContextMenu(CWnd* pWnd, CPoint point) 
		{			
			if (viewMode == view_modes::pattern)
			{
#ifdef use_patternview
				pattern_view_.OnContextMenu(pWnd, point);
#else

				CMenu menu;
				VERIFY(menu.LoadMenu(IDR_POPUPMENU));
				CMenu* pPopup = menu.GetSubMenu(0);
				ASSERT(pPopup != NULL);
				pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
				
				menu.DestroyMenu();
		//		Repaint(draw_modes::cursor);
#endif
			}
			CWnd::OnContextMenu(pWnd,point);
		}


		void CChildView::OnLButtonDown( UINT nFlags, CPoint point )
		{
			SetCapture();

			if(viewMode == view_modes::machine) {
				if (_pSong->_machineLock)
					return;
				PsycleCanvas::Event ev;
				ev.type = PsycleCanvas::Event::BUTTON_PRESS;
				ev.x = point.x;
				ev.y = point.y;
				ev.button = 1;
				ev.shift = nFlags;
				machine_view_.OnEvent(&ev);
				return;
			} else
			if (viewMode==view_modes::pattern) {			
#ifdef use_patternview
				pattern_view_.OnLButtonDown(nFlags, point);
#else
				int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
				if ( ttm >= _pSong->SONGTRACKS ) ttm = _pSong->SONGTRACKS-1;
				else if ( ttm < 0 ) ttm = 0;
				
				if (point.y >= 0 && point.y < YOFFSET ) // Mouse is in Track Header.
				{	
					int pointpos= ((point.x-XOFFSET)%ROWWIDTH) - HEADER_INDENT;

					if (InRect(pointpos,point.y,PatHeaderCoords.dRecordOn,PatHeaderCoords.sRecordOn))
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
					else if (InRect(pointpos,point.y,PatHeaderCoords.dMuteOn,PatHeaderCoords.sMuteOn))
					{
						_pSong->_trackMuted[ttm] = !_pSong->_trackMuted[ttm];
					}
					else if (InRect(pointpos,point.y,PatHeaderCoords.dSoloOn,PatHeaderCoords.sSoloOn))
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
					Repaint(draw_modes::track_header);
				}
				else if ( point.y >= YOFFSET )
				{
					oldm.track=ttm;

					int plines = _pSong->patternLines[_ps()];
					oldm.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
					if ( oldm.line >= plines ) { oldm.line = plines - 1; }
					else if ( oldm.line < 0 ) oldm.line = 0;

					oldm.col=_xtoCol((point.x-XOFFSET)%ROWWIDTH);

					if (blockSelected
						&& oldm.track >=blockSel.start.track && oldm.track <= blockSel.end.track
						&& oldm.line >=blockSel.start.line && oldm.line <= blockSel.end.line && Global::pConfig->_windowsBlocks)
					{
						blockswitch=true;
						blockLastOrigin = blockSel;
						editcur = oldm;
					}
					else blockStart = true;
					if (nFlags & MK_SHIFT)
					{
						editcur = oldm;
						Repaint(draw_modes::cursor);
					}
				}
#endif
			}//<-- End LBUTTONPRESING/VIEWMODE if statement
			CWnd::OnLButtonDown(nFlags,point);
		}

		void CChildView::OnLButtonUp( UINT nFlags, CPoint point )
		{
			ReleaseCapture();
			
			if (viewMode == view_modes::machine )
			{
				PsycleCanvas::Event ev;
				ev.type = PsycleCanvas::Event::BUTTON_RELEASE;
				ev.x = point.x;
				ev.y = point.y;
				ev.button = 1;
				ev.shift = nFlags;
				machine_view_.OnEvent(&ev);
				return;
			}
			else if (viewMode == view_modes::pattern)
			{
#ifdef use_patternview
				pattern_view_.OnLButtonUp(nFlags, point);
#else
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
					Repaint(draw_modes::cursor);
					pParentMain->StatusBarIdle();
					if (!(nFlags & MK_SHIFT) && Global::pConfig->_windowsBlocks)
					{
						blockSelected=false;
						blockSel.end.line=0;
						blockSel.end.track=0;
						ChordModeOffs = 0;
						bScrollDetatch=false;
						Repaint(draw_modes::selection);
					}
				}
				else if (blockswitch)
				{
					if (blockSel.start.track != blockLastOrigin.start.track ||
						blockSel.start.line != blockLastOrigin.start.line)
					{
						CSelection dest = blockSel;
						blockSel = blockLastOrigin;
						if ( nFlags & MK_CONTROL ) 
						{
							CopyBlock(false);
							PasteBlock(dest.start.track,dest.start.line,false);
						}
						else SwitchBlock(dest.start.track,dest.start.line);
						blockSel = dest;
					}
					else blockSelected=false; 
					blockswitch=false;
					Repaint(draw_modes::selection);
				}
#endif
			}//<-- End LBUTTONPRESING/VIEWMODE switch statement
			CWnd::OnLButtonUp(nFlags,point);
		}


		void CChildView::OnMouseMove( UINT nFlags, CPoint point )
		{
			if (viewMode == view_modes::machine)
			{
				PsycleCanvas::Event ev;
				ev.type = PsycleCanvas::Event::MOTION_NOTIFY;
				ev.x = point.x;
				ev.y = point.y;
				if ( nFlags & MK_LBUTTON )
					ev.button = 1;
				else if ( nFlags & MK_RBUTTON )
					ev.button = 3;
				else
					ev.button = 0;
				ev.shift = nFlags;
				machine_view_.OnEvent(&ev);
				return;
			}

			else if (viewMode == view_modes::pattern)
			{
#ifdef use_patternview
				pattern_view_.OnMouseMove(nFlags, point);
#else
				if ((nFlags & MK_LBUTTON) && oldm.track != -1)
				{
					ntOff = tOff;
					nlOff = lOff;
					draw_modes::draw_mode paintmode = draw_modes::all;

					int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
					if ( point.x < XOFFSET ) ttm--; // 1/2 = 0 , -1/2 = 0 too!
					int ccm;
					if ( ttm < tOff ) // Exceeded from left
					{
						ccm=0;
						if ( ttm < 0 ) { ttm = 0; } // Out of Range
						// and Scroll
						ntOff = ttm;
						if (ntOff != tOff) paintmode=draw_modes::horizontal_scroll;
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
								paintmode=draw_modes::horizontal_scroll; 
							}
						}
						else	//scroll
						{	
							ntOff = ttm-VISTRACKS+1;
							if ( ntOff != tOff ) 
								paintmode=draw_modes::horizontal_scroll;
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
								paintmode=draw_modes::vertical_scroll; 
							}
						}
						else	//scroll
						{	
							nlOff = llm;
							if ( nlOff != lOff ) 
								paintmode=draw_modes::vertical_scroll;
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
								paintmode=draw_modes::vertical_scroll; 
							}
						}
						else	//scroll
						{	
							nlOff = llm-VISLINES+1;
							if ( nlOff != lOff ) 
								paintmode=draw_modes::vertical_scroll;
						}
					}
					
					else if ( llm >= plines ) { llm = plines-1; } //Out of Range

					if ((ttm != oldm.track ) || (llm != oldm.line) || (ccm != oldm.col))
					{
						if (blockStart) 
						{
							blockStart = false;
							blockSelected=false;
							blockSel.end.line=0;
							blockSel.end.track=0;
							StartBlock(oldm.track,oldm.line,oldm.col);
						}
						else if ( blockswitch ) 
						{
							blockSelectBarState = 1;

							int tstart = (blockLastOrigin.start.track+(ttm-editcur.track) >= 0)?(ttm-editcur.track):-blockLastOrigin.start.track;
							int lstart = (blockLastOrigin.start.line+(llm-editcur.line) >= 0)?(llm-editcur.line):-blockLastOrigin.start.line;
							if (blockLastOrigin.end.track+(ttm-editcur.track) >= _pSong->SONGTRACKS) tstart = _pSong->SONGTRACKS-blockLastOrigin.end.track-1;
							if (blockLastOrigin.end.line+(llm-editcur.line) >= plines) lstart = plines - blockLastOrigin.end.line-1;

							blockSel.start.track=blockLastOrigin.start.track+(tstart);
							blockSel.start.line=blockLastOrigin.start.line+(lstart);
							iniSelec = blockSel.start;
							int tend = blockLastOrigin.end.track+(tstart);
							int lend = blockLastOrigin.end.line+(lstart);
							ChangeBlock(tend,lend,ccm);
						}
						else ChangeBlock(ttm,llm,ccm);
						oldm.track=ttm;
						oldm.line=llm;
						oldm.col=ccm;
						paintmode=draw_modes::selection;
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
							paintmode=draw_modes::cursor;
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
							Repaint(draw_modes::vertical_scroll);
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
							Repaint(draw_modes::vertical_scroll);
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
							Repaint(draw_modes::horizontal_scroll);
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
							Repaint(draw_modes::horizontal_scroll);
						}
						MBStart.x += delta*ROWWIDTH;
					}
				}
#endif
			}//<-- End LBUTTONPRESING/VIEWMODE switch statement
			CWnd::OnMouseMove(nFlags,point);
		}



		void CChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
		{
			int tmac=-1;
			
			switch (viewMode)
			{
				case view_modes::machine: // User is in machine view mode
					PsycleCanvas::Event ev;
					ev.type = PsycleCanvas::Event::BUTTON_2PRESS;
					ev.x = point.x;
					ev.y = point.y;
					ev.button = 0;
					ev.shift = nFlags;
					machine_view_.OnEvent(&ev);				
				break;				
				case view_modes::pattern: // User is in pattern view mode
#ifdef use_patternview
#else
					if (( point.y >= YOFFSET ) && (point.x >= XOFFSET))
					{
						const int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
						const int nl = _pSong->patternLines[_pSong->playOrder[editPosition]];

						StartBlock(ttm,0,0);
						EndBlock(ttm,nl-1,8);
						blockStart = false;
					}
#endif

					break;
			} // <-- End switch(viewMode)
			CWnd::OnLButtonDblClk(nFlags,point);
		}



		BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
		{
			if ( viewMode == view_modes::pattern )
			{
#ifdef use_patternview
				pattern_view_.OnMouseWheel(nFlags, zDelta, pt);
#else
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
					Repaint(draw_modes::vertical_scroll);
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
					Repaint(draw_modes::vertical_scroll);
				}
#endif
			}
			return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
		}

		void CChildView::OnMButtonDown(UINT nFlags, CPoint point)
		{
#ifdef use_patternview
			pattern_view_.OnMButtonDown(nFlags, point);
#else
			MBStart.x = point.x;
			MBStart.y = point.y;
			CWnd ::OnMButtonDown(nFlags, point);
#endif
		}

		void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
		{
			if ( viewMode == view_modes::pattern )
			{
#ifdef use_patternview
				pattern_view_.OnVScroll(nSBCode, nPos, pScrollBar);
#else
				switch(nSBCode)
				{
					case SB_LINEDOWN:
						if ( lOff<_pSong->patternLines[_ps()]-VISLINES)
						{
							nlOff=lOff+1;
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::vertical_scroll);
						}
						break;
					case SB_LINEUP:
						if ( lOff>0 )
						{
							nlOff=lOff-1;
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::vertical_scroll);
						}
						break;
					case SB_PAGEDOWN:
						if ( lOff<_pSong->patternLines[_ps()]-VISLINES)
						{
							const int nl = _pSong->patternLines[_ps()]-VISLINES;
							nlOff=lOff+16;
							if (nlOff > nl)
							{
								nlOff = nl;
							}
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::vertical_scroll);
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
							Repaint(draw_modes::vertical_scroll);
						}
						break;
					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:
						if (nlOff!=(int)nPos)
						{
							const int nl = _pSong->patternLines[_ps()]-VISLINES;
							nlOff=(int)nPos;
							if (nlOff > nl)
							{
								nlOff = nl;
							}
							else if (nlOff < 0)
							{
								nlOff = 0;
							}
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::vertical_scroll);
						}
						break;
					default: 
						break;
				}
#endif
			}
			CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
		}


		void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
		{
			if ( viewMode == view_modes::pattern )
			{
#ifdef use_patternview
				pattern_view_.OnHScroll(nSBCode, nPos, pScrollBar);
#else
				switch(nSBCode)
				{
					case SB_LINERIGHT:
					case SB_PAGERIGHT:
						if ( tOff<_pSong->SONGTRACKS-VISTRACKS)
						{
							ntOff=tOff+1;
//	Disabled, since people find it as a bug, not as a feature.
//  Reenabled, because else, when the cursor jumps to next line, it gets redrawn 
//   and the scrollbar position reseted.
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::horizontal_scroll);
						}
						break;
					case SB_LINELEFT:
					case SB_PAGELEFT:
						if ( tOff>0 )
						{
							ntOff=tOff-1;
//	Disabled, since people find it as a bug, not as a feature.
//  Reenabled, because else, when the cursor jumps to next line, it gets redrawn 
//   and the scrollbar position reseted.
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::horizontal_scroll);
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
//	Disabled, since people find it as a bug, not as a feature.
//  Reenabled, because else, when the cursor jumps to next line, it gets redrawn 
//   and the scrollbar position reseted.
							bScrollDetatch=true;
							detatchpoint.track = ntOff+1;
							detatchpoint.line = nlOff+1;
							Repaint(draw_modes::horizontal_scroll);
						}
						break;
					default: 
						break;

				}
#endif
			}

			CWnd ::OnHScroll(nSBCode, nPos, pScrollBar);
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

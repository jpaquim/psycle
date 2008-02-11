/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief pattern view graphic operations for psycle::host::CChildView, private header
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		#define DRAW_DATA		1
		#define DRAW_HSCROLL	2
		#define DRAW_VSCROLL	4
		#define DRAW_TRHEADER	8
		#define DRAW_FULL_DATA	16

		void CChildView::PreparePatternRefresh(int drawMode)
		{
		#ifdef _DEBUG_PATVIEW
			TRACE("PreparePatternRefresh\n");
		#endif

			CRect rect;	
			updateMode=drawMode;					// this is ununsed for patterns
			const int snt = _pSong->SONGTRACKS;
			const int plines = _pSong->patternLines[_pSong->playOrder[editPosition]];
			if ( editcur.track >= snt ) // This should only happen when changing the song tracks.
			{							// Else, there is a problem.
				TRACE("editcur.track out of range in PreparePatternRefresh");
				editcur.track = snt-1;
			}
			if ( editcur.line >= plines ) // This should only happen when changing the pattern lines
			{							  // or changing to a pattern with less lines.
				TRACE("editcur.line out of range in PreparePatternRefresh");
				editcur.line = plines-1;
			}
			//////////////////////////////////////////////////////////////////////
			// Set the offsets and positions of data on the screen.

			// Track Offset
			if ( snt <= VISTRACKS)	
			{ 
				maxt = snt; 
				rntOff = 0; 
			}
			else
			{
				if (bScrollDetatch)
				{
					if ( drawMode == draw_modes::horizontal_scroll )
					{
						rntOff = ntOff;
						if ( rntOff >= snt-VISTRACKS ) 
							maxt = VISTRACKS;
						else 
							maxt = VISTRACKS+1;
					}
					else
					{
						if ( tOff+VISTRACKS > snt )
						{
							rntOff = snt-VISTRACKS;
							maxt=VISTRACKS;
						}
						else if (detatchpoint.track < tOff ) 
						{ 
							rntOff = detatchpoint.track; 
							maxt = VISTRACKS+1; 
						}
						else
						{
							if (detatchpoint.track >= tOff+VISTRACKS ) 
								rntOff =detatchpoint.track-VISTRACKS+1;
							else 
								rntOff = tOff;
						
							if ( rntOff >= snt-VISTRACKS ) 
								maxt = VISTRACKS;
							else 
								maxt = VISTRACKS+1;
						}
					}
				}
				else if (Global::pConfig->_centerCursor)
				{
					if ( drawMode == draw_modes::horizontal_scroll ) 
						rntOff = ntOff;
					else 
						rntOff = editcur.track - (VISTRACKS/2);

					if (rntOff >= snt-VISTRACKS)	
					{	
						rntOff = snt-VISTRACKS;	
						maxt = VISTRACKS;	
					}
					else 
					{
						if ( rntOff < 0 ) 
						{ 
							rntOff = 0; 
						}
						maxt = VISTRACKS+1;
					}
				}
				else
				{
					if ( drawMode == draw_modes::horizontal_scroll )
					{
						rntOff = ntOff;
						if ( rntOff >= snt-VISTRACKS ) 
							maxt = VISTRACKS;
						else 
							maxt = VISTRACKS+1;
					}
					else
					{
						if ( tOff+VISTRACKS > snt )
						{
							rntOff = snt-VISTRACKS;
							maxt=VISTRACKS;
						}
						else if ( editcur.track < tOff ) 
						{ 
							rntOff = editcur.track; 
							maxt = VISTRACKS+1; 
						}
						else
						{
							if ( editcur.track >= tOff+VISTRACKS ) 
								rntOff =editcur.track-VISTRACKS+1;
							else 
								rntOff = tOff;
						
							if ( rntOff >= snt-VISTRACKS ) 
								maxt = VISTRACKS;
							else 
								maxt = VISTRACKS+1;
						}
					}
				}
			}
			// Line Offset
			if ( plines <=  VISLINES)	
			{ 
				maxl = plines; 
				rnlOff = 0; 
			}
			else 
			{
				if (bScrollDetatch)
				{
					if ( drawMode == draw_modes::vertical_scroll )
					{
						rnlOff = nlOff;
						if ( rnlOff >= plines-VISLINES ) 
							maxl = VISLINES;
						else 
							maxl = VISLINES+1;
					}
					else 
					{
						if ( lOff+VISLINES > plines )
						{
							rnlOff = plines-VISLINES;
							maxl=VISLINES;
						}
						else if ( detatchpoint.line < lOff+1 ) 
						{ 
							rnlOff = detatchpoint.line-1; 
							if (rnlOff < 0)
							{
								rnlOff = 0;
							}
							maxl = VISLINES+1; 
						}
						else 
						{
							if ( detatchpoint.line >= lOff+VISLINES ) 
								rnlOff =detatchpoint.line-VISLINES+1;
							else 
								rnlOff = lOff;

							if ( rnlOff >= plines-VISLINES ) 
								maxl = VISLINES;
							else 
								maxl = VISLINES+1;
						}
					}
				}
				else if (Global::pConfig->_centerCursor)
				{
					if ( drawMode == draw_modes::vertical_scroll ) 
						rnlOff = nlOff;
					else 
						rnlOff = editcur.line - (VISLINES/2);

					if (rnlOff >= plines-VISLINES) 
					{ 
						rnlOff = plines-VISLINES; 
						maxl = VISLINES; 
					}
					else 
					{
						if ( rnlOff < 0 ) 
						{ 
							rnlOff = 0; 
						}
						maxl = VISLINES+1;
					}
				}
				else
				{
					if ( drawMode == draw_modes::vertical_scroll )
					{
						rnlOff = nlOff;
						if ( rnlOff >= plines-VISLINES ) 
							maxl = VISLINES;
						else 
							maxl = VISLINES+1;
					}
					else 
					{
						if ( lOff+VISLINES > plines )
						{
							rnlOff = plines-VISLINES;
							maxl=VISLINES;
						}
						else if ( editcur.line < lOff+1 ) 
						{ 
							rnlOff = editcur.line-1; 
							if (rnlOff < 0)
							{
								rnlOff = 0;
							}
							maxl = VISLINES+1; 
						}
						else 
						{
							if ( editcur.line >= lOff+VISLINES ) 
								rnlOff =editcur.line-VISLINES+1;
							else 
								rnlOff = lOff;

							if ( rnlOff >= plines-VISLINES ) 
								maxl = VISLINES;
							else 
								maxl = VISLINES+1;
						}
					}
				}
			}
			////////////////////////////////////////////////////////////////////
			// Determines if background Scroll is needed or not.

			if (drawMode != draw_modes::all && drawMode != draw_modes::pattern)
			{
				if ( rnlOff != lOff )
				{
					rect.top=YOFFSET;	
					rect.left=0;
					rect.bottom=CH;		
					rect.right=CW;
					updatePar |= DRAW_VSCROLL;
					InvalidateRect(rect,false);
				}
				if ( rntOff != tOff )
				{
					rect.top=0;		
					rect.left=XOFFSET;
					rect.bottom=CH;
					rect.right=CW;
					updatePar |= DRAW_HSCROLL;
					InvalidateRect(rect,false);
				}
			}
			
			switch (drawMode)
			{
			case draw_modes::all: 
				// header
				rect.top=0; 
				rect.left=0;
				rect.bottom=CH;	
				rect.right=CW;
				updatePar |= DRAW_TRHEADER | DRAW_FULL_DATA;
				InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					ShowScrollBar(SB_HORZ,TRUE);
					SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					ShowScrollBar(SB_VERT,TRUE);
					SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					ShowScrollBar(SB_VERT,FALSE); 
				}
				break;
			case draw_modes::pattern: 
				// all data
				rect.top=YOFFSET;		
				rect.left=0;
				rect.bottom=CH;
				rect.right=CW;
				updatePar |= DRAW_FULL_DATA;
				InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					ShowScrollBar(SB_HORZ,TRUE);
					SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					ShowScrollBar(SB_VERT,TRUE);
					SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					ShowScrollBar(SB_VERT,FALSE); 
				}
				break;
			case draw_modes::playback: 
				{
					int pos = Global::pPlayer->_lineCounter;
					if (( pos-rnlOff >= 0 ) &&  ( pos-rnlOff <maxl ) &&
						(_pSong->playOrder[editPosition] == _pSong->playOrder[Global::pPlayer->_playPosition]))
					{
						if (pos != playpos)
						{
							newplaypos = pos;

							rect.top= YOFFSET+ ((pos-rnlOff)*ROWHEIGHT);
							rect.bottom=rect.top+ROWHEIGHT;	// left never changes and is set at ChildView init.
							rect.left = 0;
							rect.right=CW;
							NewPatternDraw(0, _pSong->SONGTRACKS, pos, pos);
							updatePar |= DRAW_DATA;
							InvalidateRect(rect,false);
							if ((playpos >= 0) && (playpos != newplaypos))
							{
								rect.top = YOFFSET+ ((playpos-rnlOff)*ROWHEIGHT);
								rect.bottom = rect.top+ROWHEIGHT;
								rect.left = 0;
								rect.right = CW;
								NewPatternDraw(0, _pSong->SONGTRACKS, playpos, playpos);
								updatePar |= DRAW_DATA;
								playpos =-1;
								InvalidateRect(rect,false);
							}
						}
					}
					else 
					{
						newplaypos=-1;
						if (playpos >= 0) 
						{
							rect.top = YOFFSET+ ((playpos-rnlOff)*ROWHEIGHT);
							rect.bottom = rect.top+ROWHEIGHT;
							rect.left = 0;
							rect.right = CW;
							NewPatternDraw(0, _pSong->SONGTRACKS, playpos, playpos);
							updatePar |= DRAW_DATA;
							playpos = -1;
							InvalidateRect(rect,false);
						}
					}
				}
				break;
			case draw_modes::playback_change: 
				if (_pSong->playOrder[editPosition] == _pSong->playOrder[Global::pPlayer->_playPosition])
				{
					newplaypos= Global::pPlayer->_lineCounter;
				}
				else 
				{
					newplaypos=-1;
				}
				playpos=-1;
				rect.top=YOFFSET;		
				rect.left=0;
				rect.bottom=CH;
				rect.right=CW;
				updatePar |= DRAW_FULL_DATA;
				InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					ShowScrollBar(SB_HORZ,TRUE);
					SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					ShowScrollBar(SB_VERT,TRUE);
					SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					ShowScrollBar(SB_VERT,FALSE); 
				}
				break;
			case draw_modes::selection: 
				// could optimize to only draw the changes
				if (blockSelected)
				{
					if ((blockSel.end.track<rntOff) || (blockSel.end.line<rnlOff) ||
						(blockSel.start.track>=rntOff+VISTRACKS) ||
						(blockSel.start.line>=rnlOff+VISLINES))
					{
						newselpos.bottom = 0; // This marks as "don't show selection" (because out of range)
					}
					else 
					{
		//				if (blockSel.start.line <= blockSel.end.line)
		//				{
							newselpos.top=blockSel.start.line;
							newselpos.bottom=blockSel.end.line+1;
		//				}
		//				else
		//				{
		//					newselpos.top=blockSel.end.line;
		//					newselpos.bottom=blockSel.start.line+1;
		//				}

		//				if (blockSel.start.track <= blockSel.end.track)
		//				{
							newselpos.left=blockSel.start.track;
							newselpos.right=blockSel.end.track+1;
		//				}
		//				else
		//				{
		//					newselpos.right=blockSel.start.track;
		//					newselpos.left=blockSel.end.track+1;
		//				}

						if (selpos.bottom == 0)
						{
							//if(blockSel.start.track<rntOff) 
							//	rect.left=XOFFSET;
							//else 
								rect.left=XOFFSET+(blockSel.start.track-rntOff)*ROWWIDTH;
							
							//if(blockSel.start.line<=rnlOff) 
							//	rect.top=YOFFSET;
							//else 
								rect.top=YOFFSET+(blockSel.start.line-rnlOff)*ROWHEIGHT;
							
							//if(blockSel.end.track>=rntOff+VISTRACKS) 
							//	rect.right=CW;
							//else 
								rect.right=XOFFSET+(blockSel.end.track-rntOff+1)*ROWWIDTH;

							//if(blockSel.end.line>=rnlOff+VISLINES ) 
							//	rect.bottom=CH;
							//else 
								rect.bottom=YOFFSET+(blockSel.end.line-rnlOff+1)*ROWHEIGHT;
							
							NewPatternDraw(blockSel.start.track, blockSel.end.track, blockSel.start.line, blockSel.end.line);
							updatePar |= DRAW_DATA;
							InvalidateRect(rect,false);
						}
						else if (newselpos != selpos)
						{
							if (newselpos.left < selpos.left)
							{
								rect.left = newselpos.left;
								if (newselpos.right > selpos.right)
								{
									rect.right = newselpos.right;
								}
								else if (newselpos.right < selpos.right)
								{
									rect.right = selpos.right;
								}
								else 
								{
									rect.right = selpos.left;
								}

								if (newselpos.top <= selpos.top)
								{
									rect.top = newselpos.top;
								}
								else 
								{
									rect.top = selpos.top;
								}

								if (newselpos.bottom >= selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else 
								{
									rect.bottom = selpos.bottom;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}
							else if (newselpos.left > selpos.left)
							{
								rect.left = selpos.left;
								if (newselpos.right > selpos.right)
								{
									rect.right = newselpos.right;
								}
								else if (newselpos.right < selpos.right)
								{
									rect.right = selpos.right;
								}
								else 
								{
									rect.right = newselpos.left;
								}

								if (newselpos.top <= selpos.top)
								{
									rect.top = newselpos.top;
								}
								else 
								{
									rect.top = selpos.top;
								}

								if (newselpos.bottom >= selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else 
								{
									rect.bottom = selpos.bottom;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}

							if (newselpos.right < selpos.right)
							{
								rect.left = newselpos.right;
								rect.right = selpos.right;

								if (newselpos.top <= selpos.top)
								{
									rect.top = newselpos.top;
								}
								else 
								{
									rect.top = selpos.top;
								}

								if (newselpos.bottom >= selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else 
								{
									rect.bottom = selpos.bottom;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}
							else if (newselpos.right > selpos.right)
							{
								rect.left = selpos.right;
								rect.right = newselpos.right;

								if (newselpos.top <= selpos.top)
								{
									rect.top = newselpos.top;
								}
								else 
								{
									rect.top = selpos.top;
								}

								if (newselpos.bottom >= selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else 
								{
									rect.bottom = selpos.bottom;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}

							if (newselpos.top < selpos.top)
							{
								rect.top = newselpos.top;
								if (newselpos.bottom > selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else if (newselpos.bottom < selpos.bottom)
								{
									rect.bottom = selpos.bottom;
								}
								else 
								{
									rect.bottom = selpos.top;
								}

								if (newselpos.left <= selpos.left)
								{
									rect.left = newselpos.left;
								}
								else 
								{
									rect.left = selpos.left;
								}

								if (newselpos.right >= selpos.right)
								{
									rect.right = newselpos.right;
								}
								else 
								{
									rect.right = selpos.right;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}
							else if (newselpos.top > selpos.top)
							{
								rect.top = selpos.top;
								if (newselpos.bottom > selpos.bottom)
								{
									rect.bottom = newselpos.bottom;
								}
								else if (newselpos.bottom < selpos.bottom)
								{
									rect.bottom = selpos.bottom;
								}
								else 
								{
									rect.bottom = newselpos.top;
								}

								if (newselpos.left <= selpos.left)
								{
									rect.left = newselpos.left;
								}
								else 
								{
									rect.left = selpos.left;
								}

								if (newselpos.right >= selpos.right)
								{
									rect.right = newselpos.right;
								}
								else 
								{
									rect.right = selpos.right;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}

							if (newselpos.bottom < selpos.bottom)
							{
								rect.top = newselpos.bottom;
								rect.bottom = selpos.bottom;

								if (newselpos.left <= selpos.left)
								{
									rect.left = newselpos.left;
								}
								else 
								{
									rect.left = selpos.left;
								}

								if (newselpos.right >= selpos.right)
								{
									rect.right = newselpos.right;
								}
								else 
								{
									rect.right = selpos.right;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}
							else if (newselpos.bottom > selpos.bottom)
							{
								rect.top = selpos.bottom;
								rect.bottom = newselpos.bottom;

								if (newselpos.left <= selpos.left)
								{
									rect.left = newselpos.left;
								}
								else 
								{
									rect.left = selpos.left;
								}

								if (newselpos.right >= selpos.right)
								{
									rect.right = newselpos.right;
								}
								else 
								{
									rect.right = selpos.right;
								}
							
								NewPatternDraw(rect.left, rect.right, rect.top, rect.bottom);
								updatePar |= DRAW_DATA;
								rect.left = XOFFSET+(rect.left-rntOff)*ROWWIDTH;
								rect.right = XOFFSET+(rect.right-rntOff)*ROWWIDTH;
								rect.top=YOFFSET+(rect.top-rnlOff)*ROWHEIGHT;
								rect.bottom=YOFFSET+(rect.bottom-rnlOff)*ROWHEIGHT;
								InvalidateRect(rect,false);
							}

						}
					}
				}
				else if ( selpos.bottom != 0)
				{
					//if(selpos.left<rntOff) 
					//	rect.left=XOFFSET;
					//else 
						rect.left=XOFFSET+(selpos.left-rntOff)*ROWWIDTH;
					
					//if(selpos.top<=rnlOff) 
					//	rect.top=YOFFSET;
					//else 
						rect.top=YOFFSET+(selpos.top-rnlOff)*ROWHEIGHT;
					
					//if(selpos.right>=rntOff+VISTRACKS) 
					//	rect.right=CW;
					//else 
						rect.right=XOFFSET+(selpos.right-rntOff)*ROWWIDTH;

					//if(selpos.bottom>=rnlOff+VISLINES ) 
					//	rect.bottom=CH;
					//else 
						rect.bottom=YOFFSET+(selpos.bottom-rnlOff)*ROWHEIGHT;
					
					NewPatternDraw(selpos.left, selpos.right, selpos.top, selpos.bottom);
					updatePar |= DRAW_DATA;
					newselpos.bottom=0;
					InvalidateRect(rect,false);
				}
				break;
			case draw_modes::data: 
				{
					SPatternDraw* pPD = &pPatternDraw[numPatternDraw-1];
					
					rect.left=XOFFSET+  ((pPD->drawTrackStart-rntOff)*ROWWIDTH);
					rect.right=XOFFSET+ ((pPD->drawTrackEnd-(rntOff-1))*ROWWIDTH);
					rect.top=YOFFSET+	((pPD->drawLineStart-rnlOff)*ROWHEIGHT);
					rect.bottom=YOFFSET+((pPD->drawLineEnd-(rnlOff-1))*ROWHEIGHT);
					updatePar |= DRAW_DATA;
					InvalidateRect(rect,false);
				}
				break;
			case draw_modes::track_header: 
				// header
				rect.top=0; 
				rect.left=XOFFSET;
				rect.bottom=YOFFSET-1;	
				rect.right=XOFFSET+maxt*ROWWIDTH;
				updatePar |= DRAW_TRHEADER;
				InvalidateRect(rect,false);
				break;
		//	case draw_modes::cursor: 
		//		break;
			case draw_modes::none: 
				break;
			}

			if ((editcur.col != editlast.col) || (editcur.track != editlast.track) || (editcur.line != editlast.line))
			{
				rect.left = XOFFSET+(editcur.track-rntOff)*ROWWIDTH;
				rect.right = rect.left+ROWWIDTH;
				rect.top = YOFFSET+(editcur.line-rnlOff)*ROWHEIGHT;
				rect.bottom = rect.top+ROWWIDTH;
				NewPatternDraw(editcur.track, editcur.track, editcur.line, editcur.line);
				updatePar |= DRAW_DATA;
				InvalidateRect(rect,false);
				if (editcur.line != editlast.line)
				{
					if (XOFFSET!=1)
					{
						rect.left = 0;
						rect.right = XOFFSET;
						InvalidateRect(rect,false);
					}
					rect.left = XOFFSET+(editlast.track-rntOff)*ROWWIDTH;
					rect.right = rect.left+ROWWIDTH;
					rect.top = YOFFSET+(editlast.line-rnlOff)*ROWHEIGHT;
					rect.bottom = rect.top+ROWWIDTH;
					NewPatternDraw(editlast.track, editlast.track, editlast.line, editlast.line);
					InvalidateRect(rect,false);
					if (XOFFSET!=1)
					{
						rect.left = 0;
						rect.right = XOFFSET;
						InvalidateRect(rect,false);
					}
				}
				else if (editcur.track != editlast.track)
				{
					rect.left = XOFFSET+(editlast.track-rntOff)*ROWWIDTH;
					rect.right = rect.left+ROWWIDTH;
					rect.top = YOFFSET+(editlast.line-rnlOff)*ROWHEIGHT;
					rect.bottom = rect.top+ROWWIDTH;
					NewPatternDraw(editlast.track, editlast.track, editlast.line, editlast.line);
					InvalidateRect(rect,false);
				}
			}

			// turn off play line if not playing
			if (playpos >= 0 && !Global::pPlayer->_playing) 
			{
				newplaypos=-1;
				rect.top = YOFFSET+ (playpos-rnlOff)*ROWHEIGHT;
				rect.bottom = rect.top+ROWHEIGHT;
				rect.left = 0;
				rect.right = XOFFSET+(maxt)*ROWWIDTH;
				NewPatternDraw(0, _pSong->SONGTRACKS, playpos, playpos);
				playpos =-1;
				updatePar |= DRAW_DATA;
				InvalidateRect(rect,false);
			}

			////////////////////////////////////////////////////////////////////
			// Checks for specific code to update.

			SetScrollPos(SB_HORZ,rntOff);
			SetScrollPos(SB_VERT,rnlOff);
			//\todo: This line has been commented out because it is what is causing the breaks in sound when
			//  using follow song and the pattern changes. Commenting it out should only mean that the refresh
			//	is delayed for the next idle time, and not forced now, inside the timer, (which has a lock).
//			UpdateWindow();
		}




		#define DF_NONE			0
		#define	DF_SELECTION	1
		#define DF_PLAYBAR		2
		#define DF_CURSOR		4
		#define DF_DRAWN		15

		void CChildView::DrawPatEditor(CDC *devc)
		{
			///////////////////////////////////////////////////////////
			// Prepare pattern for update 
			CRect rect;
			CFont* oldFont;

			int scrollT= tOff-rntOff;
			int scrollL= lOff-rnlOff;

			tOff = ntOff = rntOff; 
			lOff = nlOff = rnlOff;

			oldFont= devc->SelectObject(&Global::pConfig->seqFont);

			// 1 if there is a redraw header, we do that 
			/////////////////////////////////////////////////////////////
			// Update Mute/Solo Indicators
			if ((updatePar & DRAW_TRHEADER) || (abs(scrollT) > VISTRACKS) || (scrollT && scrollL))
			{
				rect.top = 0;
				rect.bottom = YOFFSET;
				if (XOFFSET!=1)
				{
					rect.left = 0;
					rect.right = 1;
					devc->FillSolidRect(&rect,pvc_separator[0]);
					rect.left++;
					rect.right = XOFFSET-1;
					devc->FillSolidRect(&rect,pvc_background[0]);
					devc->SetBkColor(Global::pConfig->pvc_background);	// This affects TXT background
					devc->SetTextColor(Global::pConfig->pvc_font);
					TXT(devc,"Line",1,1,XOFFSET-2,YOFFSET-2);
				}
				CDC memDC;
				CBitmap *oldbmp;
				memDC.CreateCompatibleDC(devc);
				oldbmp = memDC.SelectObject(&patternheader);
				int xOffset = XOFFSET-1;

				if (PatHeaderCoords.bHasTransparency)
				{
					for(int i=tOff;i<tOff+maxt;i++)
					{
						rect.left = xOffset;
						rect.right = xOffset+1;
						devc->FillSolidRect(&rect,pvc_separator[i+1]);
						rect.left++;
						rect.right+= ROWWIDTH-1;
						devc->FillSolidRect(&rect,pvc_background[i+1]);

						const int trackx0 = i/10;
						const int track0x = i%10;

						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						TransparentBlt(devc,
							xOffset+1+HEADER_INDENT,
							1,
							PatHeaderCoords.sBackground.width, 
							PatHeaderCoords.sBackground.height,
							&memDC, 
							&patternheadermask,
							PatHeaderCoords.sBackground.x,
							PatHeaderCoords.sBackground.y);
						TransparentBlt(devc,
							xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
							1+PatHeaderCoords.dDigitX0.y, 
							PatHeaderCoords.sNumber0.width,	 
							PatHeaderCoords.sNumber0.height, 
							&memDC, 
							&patternheadermask,
							PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
							PatHeaderCoords.sNumber0.y);
						TransparentBlt(devc,
							xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
							1+PatHeaderCoords.dDigit0X.y, 
							PatHeaderCoords.sNumber0.width,	 
							PatHeaderCoords.sNumber0.height, 
							&memDC, 
							&patternheadermask,
							PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
							PatHeaderCoords.sNumber0.y);
						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						if (Global::_pSong->_trackMuted[i])
							TransparentBlt(devc,
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
								1+PatHeaderCoords.dMuteOn.y, 
								PatHeaderCoords.sMuteOn.width, 
								PatHeaderCoords.sMuteOn.height, 
								&memDC, 
								&patternheadermask,
								PatHeaderCoords.sMuteOn.x, 
								PatHeaderCoords.sMuteOn.y);

						if (Global::_pSong->_trackArmed[i])
							TransparentBlt(devc,
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
								1+PatHeaderCoords.dRecordOn.y, 
								PatHeaderCoords.sRecordOn.width, 
								PatHeaderCoords.sRecordOn.height, 
								&memDC, 
								&patternheadermask,
								PatHeaderCoords.sRecordOn.x, 
								PatHeaderCoords.sRecordOn.y);

						if (Global::_pSong->_trackSoloed == i )
							TransparentBlt(devc,
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
								1+PatHeaderCoords.dSoloOn.y, 
								PatHeaderCoords.sSoloOn.width, 
								PatHeaderCoords.sSoloOn.height, 
								&memDC, 
								&patternheadermask,
								PatHeaderCoords.sSoloOn.x, 
								PatHeaderCoords.sSoloOn.y);
						xOffset += ROWWIDTH;
					}
				}
				else
				{
					for(int i=tOff;i<tOff+maxt;i++)
					{
						rect.left = xOffset;
						rect.right = xOffset+1;
						devc->FillSolidRect(&rect,pvc_separator[i+1]);
						rect.left++;
						rect.right+= ROWWIDTH-1;
						devc->FillSolidRect(&rect,pvc_background[i+1]);

						const int trackx0 = i/10;
						const int track0x = i%10;

						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						devc->BitBlt(
							xOffset+1+HEADER_INDENT,
							1,
							PatHeaderCoords.sBackground.width, 
							PatHeaderCoords.sBackground.height,
							&memDC, 
							PatHeaderCoords.sBackground.x,
							PatHeaderCoords.sBackground.y, 
							SRCCOPY);
						devc->BitBlt(
							xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
							1+PatHeaderCoords.dDigitX0.y, 
							PatHeaderCoords.sNumber0.width,	 
							PatHeaderCoords.sNumber0.height, 
							&memDC, 
							PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
							PatHeaderCoords.sNumber0.y, 
							SRCCOPY);
						devc->BitBlt(
							xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
							1+PatHeaderCoords.dDigit0X.y, 
							PatHeaderCoords.sNumber0.width,	 
							PatHeaderCoords.sNumber0.height, 
							&memDC, 
							PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
							PatHeaderCoords.sNumber0.y, 
							SRCCOPY);

						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						if (Global::_pSong->_trackMuted[i])
							devc->BitBlt(
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
								1+PatHeaderCoords.dMuteOn.y, 
								PatHeaderCoords.sMuteOn.width, 
								PatHeaderCoords.sMuteOn.height, 
								&memDC, 
								PatHeaderCoords.sMuteOn.x, 
								PatHeaderCoords.sMuteOn.y, 
								SRCCOPY);

						if (Global::_pSong->_trackArmed[i])
							devc->BitBlt(
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
								1+PatHeaderCoords.dRecordOn.y, 
								PatHeaderCoords.sRecordOn.width, 
								PatHeaderCoords.sRecordOn.height, 
								&memDC, 
								PatHeaderCoords.sRecordOn.x, 
								PatHeaderCoords.sRecordOn.y, 
								SRCCOPY);

						if (Global::_pSong->_trackSoloed == i )
							devc->BitBlt(
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
								1+PatHeaderCoords.dSoloOn.y, 
								PatHeaderCoords.sSoloOn.width, 
								PatHeaderCoords.sSoloOn.height, 
								&memDC, 
								PatHeaderCoords.sSoloOn.x, 
								PatHeaderCoords.sSoloOn.y, 
								SRCCOPY);
						xOffset += ROWWIDTH;
					}
				}
				memDC.SelectObject(oldbmp);
				memDC.DeleteDC();
			}

			// 2 if there is a redraw all, we do that then exit
			if ((updatePar & DRAW_FULL_DATA) || (abs(scrollT) > VISTRACKS) || (abs(scrollL) > VISLINES) || (scrollT && scrollL))
			{
		#ifdef _DEBUG_PATVIEW
				TRACE("DRAW_FULL_DATA\n");
		#endif
				// draw everything
				rect.top = YOFFSET;
				rect.bottom = CH;

				if (XOFFSET!=1)
				{
					rect.left = 0;
					rect.right = 1;
					devc->FillSolidRect(&rect,pvc_separator[0]);
					rect.left++;
					rect.right = XOFFSET-1;
					devc->FillSolidRect(&rect,pvc_background[0]);
				}
				int xOffset = XOFFSET-1;

				for (int i=tOff;i<tOff+maxt;i++)
				{
					rect.left = xOffset;
					rect.right = xOffset+1;
					devc->FillSolidRect(&rect,pvc_separator[i+1]);
					rect.left++;
					rect.right+= ROWWIDTH-1;
					devc->FillSolidRect(&rect,pvc_background[i+1]);

					xOffset += ROWWIDTH;
				}
				DrawPatternData(devc,0,VISTRACKS+1,0,VISLINES+1);
				// wipe todo list
				numPatternDraw = 0;
				// Fill Bottom Space with Background colour if needed
				if (maxl < VISLINES+1)
				{
		#ifdef _DEBUG_PATVIEW
					TRACE("DRAW_BOTTOM\n");
		#endif
					if (XOFFSET!=1)
					{
						rect.left=0; 
						rect.right=XOFFSET; 
						rect.top=YOFFSET+(maxl*ROWHEIGHT); 
						rect.bottom=CH;
						devc->FillSolidRect(&rect,pvc_separator[0]);
					}

					int xOffset = XOFFSET;

					rect.top=YOFFSET+(maxl*ROWHEIGHT); 
					rect.bottom=CH;
					for(int i=tOff;i<tOff+maxt;i++)
					{
						rect.left=xOffset; 
						rect.right=xOffset+ROWWIDTH; 
						devc->FillSolidRect(&rect,pvc_separator[i+1]);
						xOffset += ROWWIDTH;
					}
				}
				// Fill Right Space with Background colour if needed
				if (maxt < VISTRACKS+1)
				{
		#ifdef _DEBUG_PATVIEW
					TRACE("DRAW_RIGHT\n");
		#endif
					rect.top=0; 
					rect.bottom=CH;  
					rect.right=CW;
					rect.left=XOFFSET+(maxt*ROWWIDTH)-1;
					devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
				}
			}
			else
			{
				if (scrollT && scrollL)
				{
					/*
					// there is a flaw in this that makes background rect not draw quite right
					// so for now x+y does a redraw all - i don't think anyone will complain
					// as x+y scrolling only happens when you have that stupid center cursor on
					// or when selecting
					// scroll x AND y
					CRgn rgn;
					if (XOFFSET!=1)
					{
						const RECT linR = {0, YOFFSET, XOFFSET, CH};
						devc->ScrollDC(0,scrollL*ROWHEIGHT,&linR,&linR,&rgn,&rect);
					}
					const RECT patR = {XOFFSET,YOFFSET , CW, CH};
					devc->ScrollDC(scrollT*ROWWIDTH,scrollL*ROWHEIGHT,&patR,&patR,&rgn,&rect);
					if (updatePar & DRAW_TRHEADER)
					{
						if (scrollT > 0)
						{	
							rect.top = YOFFSET;
							rect.bottom = CH;
							int xOffset = XOFFSET-1;
							for (int i = 0; i < scrollT; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							xOffset = XOFFSET-1+((VISTRACKS-scrollT)*ROWWIDTH);
							for (i = VISTRACKS-scrollT; i < VISTRACKS+1; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
							DrawPatternData(devc, VISTRACKS-scrollT-1, VISTRACKS+1, 0,VISLINES+1);
							if (scrollL > 0)
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL++\n");
		#endif
								DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1, 0,scrollL);
							}
							else 
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL+-\n");
		#endif
								DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1,VISLINES+scrollL,VISLINES+1);
							}
						}
						else 
						{	
							rect.top = YOFFSET;
							rect.bottom = CH;
							int xOffset = XOFFSET-1;
							for (int i = 0; i < 1-scrollT; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
							for (i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
							DrawPatternData(devc,0, 1-scrollT, 0, VISLINES+1);
							if (scrollL > 0)
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL-+\n");
		#endif
								DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT, 0,scrollL);
							}
							else
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL--\n");
		#endif
								DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT,VISLINES+scrollL,VISLINES+1);
							}
						}
					}
					else
					{
						// scroll header too
						const RECT trkR = {XOFFSET, 0, CW, YOFFSET-1};
						devc->ScrollDC(scrollT*ROWWIDTH,0,&trkR,&trkR,&rgn,&rect);
						if (scrollT > 0)
						{	
							rect.top = 0;
							rect.bottom = CH;
							int xOffset = XOFFSET-1;
							for (int i = 0; i < scrollT; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							rect.top = YOFFSET;
							xOffset = XOFFSET-1+((VISTRACKS-scrollT)*ROWWIDTH);
							for (i = VISTRACKS-scrollT; i < VISTRACKS+1; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
							DrawPatternData(devc, VISTRACKS-scrollT-1, VISTRACKS+1, 0,VISLINES+1);
							if (scrollL > 0)
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL++H\n");
		#endif
								DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1, 0,scrollL);
							}
							else 
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL+-H\n");
		#endif
								DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1,VISLINES+scrollL,VISLINES+1);
							}

							CDC memDC;
							CBitmap *oldbmp;
							memDC.CreateCompatibleDC(devc);
							oldbmp = memDC.SelectObject(&patternheader);
							xOffset = XOFFSET-1;

							if (PatHeaderCoords.bHasTransparency)
							{
								for(int i=tOff;i<tOff+scrollT;i++)
								{
									const int trackx0 = i/10;
									const int track0x = i%10;

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT,
										1,
										PatHeaderCoords.sBackground.width, 
										PatHeaderCoords.sBackground.height,
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sBackground.x,
										PatHeaderCoords.sBackground.y);
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
										1+PatHeaderCoords.dDigitX0.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y);
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
										1+PatHeaderCoords.dDigit0X.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y);
									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									if (Global::_pSong->_trackMuted[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y);

									if (Global::_pSong->_trackArmed[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y);

									if (Global::_pSong->_trackSoloed == i )
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
											1+PatHeaderCoords.dSoloOn.y, 
											PatHeaderCoords.sSoloOn.width, 
											PatHeaderCoords.sSoloOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sSoloOn.x, 
											PatHeaderCoords.sSoloOn.y);
									xOffset += ROWWIDTH;
								}
							}
							else
							{
								for(int i=tOff;i<tOff+scrollT;i++)
								{
									const int trackx0 = i/10;
									const int track0x = i%10;

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									devc->BitBlt(
										xOffset+1+HEADER_INDENT,
										1,
										PatHeaderCoords.sBackground.width, 
										PatHeaderCoords.sBackground.height,
										&memDC, 
										PatHeaderCoords.sBackground.x,
										PatHeaderCoords.sBackground.y, 
										SRCCOPY);
									devc->BitBlt(
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
										1+PatHeaderCoords.dDigitX0.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y, 
										SRCCOPY);
									devc->BitBlt(
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
										1+PatHeaderCoords.dDigit0X.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y, 
										SRCCOPY);

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									if (Global::_pSong->_trackMuted[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y, 
											SRCCOPY);

									if (Global::_pSong->_trackArmed[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y, 
											SRCCOPY);

									if (Global::_pSong->_trackSoloed == i )
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
											1+PatHeaderCoords.dSoloOn.y, 
											PatHeaderCoords.sSoloOn.width, 
											PatHeaderCoords.sSoloOn.height, 
											&memDC, 
											PatHeaderCoords.sSoloOn.x, 
											PatHeaderCoords.sSoloOn.y, 
											SRCCOPY);
									xOffset += ROWWIDTH;
								}
							}
							memDC.SelectObject(oldbmp);
							memDC.DeleteDC();
						}
						else
						{	
							rect.top = YOFFSET;
							rect.bottom = CH;
							int xOffset = XOFFSET-1;
							for (int i = 0; i < 1-scrollT; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							rect.top = 0;
							xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
							for (i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
							{
								rect.left = xOffset;
								rect.right = xOffset+1;
								devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
								rect.left++;
								rect.right+= ROWWIDTH-1;
								devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

								xOffset += ROWWIDTH;
							}
							DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
							DrawPatternData(devc,0, 1-scrollT, 0, VISLINES+1);
							if (scrollL > 0)
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL-+H\n");
		#endif
								DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT, 0,scrollL);
							}
							else
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HVSCROLL--H\n");
		#endif
								DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT,VISLINES+scrollL,VISLINES+1);
							}

							CDC memDC;
							CBitmap *oldbmp;
							memDC.CreateCompatibleDC(devc);
							oldbmp = memDC.SelectObject(&patternheader);
							xOffset = XOFFSET-1+((maxt+scrollT-1)*ROWWIDTH);

							if (PatHeaderCoords.bHasTransparency)
							{
								for(i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
								{
									const int trackx0 = i/10;
									const int track0x = i%10;

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT,
										1,
										PatHeaderCoords.sBackground.width, 
										PatHeaderCoords.sBackground.height,
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sBackground.x,
										PatHeaderCoords.sBackground.y);
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
										1+PatHeaderCoords.dDigitX0.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y);
									TransparentBlt(devc,
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
										1+PatHeaderCoords.dDigit0X.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										&patternheadermask,
										PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y);
									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									if (Global::_pSong->_trackMuted[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y);

									if (Global::_pSong->_trackArmed[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y);

									if (Global::_pSong->_trackSoloed == i )
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
											1+PatHeaderCoords.dSoloOn.y, 
											PatHeaderCoords.sSoloOn.width, 
											PatHeaderCoords.sSoloOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sSoloOn.x, 
											PatHeaderCoords.sSoloOn.y);
									xOffset += ROWWIDTH;
								}
							}
							else
							{
								for(i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
								{
									const int trackx0 = i/10;
									const int track0x = i%10;

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									devc->BitBlt(
										xOffset+1+HEADER_INDENT,
										1,
										PatHeaderCoords.sBackground.width, 
										PatHeaderCoords.sBackground.height,
										&memDC, 
										PatHeaderCoords.sBackground.x,
										PatHeaderCoords.sBackground.y, 
										SRCCOPY);
									devc->BitBlt(
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
										1+PatHeaderCoords.dDigitX0.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y, 
										SRCCOPY);
									devc->BitBlt(
										xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
										1+PatHeaderCoords.dDigit0X.y, 
										PatHeaderCoords.sNumber0.width,	 
										PatHeaderCoords.sNumber0.height, 
										&memDC, 
										PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
										PatHeaderCoords.sNumber0.y, 
										SRCCOPY);

									// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
									if (Global::_pSong->_trackMuted[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y, 
											SRCCOPY);

									if (Global::_pSong->_trackArmed[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y, 
											SRCCOPY);

									if (Global::_pSong->_trackSoloed == i )
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
											1+PatHeaderCoords.dSoloOn.y, 
											PatHeaderCoords.sSoloOn.width, 
											PatHeaderCoords.sSoloOn.height, 
											&memDC, 
											PatHeaderCoords.sSoloOn.x, 
											PatHeaderCoords.sSoloOn.y, 
											SRCCOPY);
									xOffset += ROWWIDTH;
								}
							}
							memDC.SelectObject(oldbmp);
							memDC.DeleteDC();
						}
					}
					// Fill Bottom Space with Background colour if needed
					if (maxl < VISLINES+1)
					{
		#ifdef _DEBUG_PATVIEW
						TRACE("DRAW_BOTTOM\n");
		#endif
						if (XOFFSET!=1)
						{
							CRect rect;
							rect.left=0; 
							rect.right=XOFFSET; 
							rect.top=YOFFSET+(maxl*ROWHEIGHT); 
							rect.bottom=CH;
							devc->FillSolidRect(&rect,pvc_separator[0]);
						}

						int xOffset = XOFFSET;

						CRect rect;
						rect.top=YOFFSET+(maxl*ROWHEIGHT); 
						rect.bottom=CH;
						for(int i=tOff;i<tOff+maxt;i++)
						{
							rect.left=xOffset; 
							rect.right=xOffset+ROWWIDTH; 
							devc->FillSolidRect(&rect,pvc_separator[i+1]);
							xOffset += ROWWIDTH;
						}
					}
					// Fill Right Space with Background colour if needed
					if (maxt < VISTRACKS+1)
					{
		#ifdef _DEBUG_PATVIEW
						TRACE("DRAW_RIGHT\n");
		#endif
						CRect rect;
						rect.top=0; 
						rect.bottom=CH;  
						rect.right=CW;
						rect.left=XOFFSET+(maxt*ROWWIDTH)-1;
						devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
					}
					*/
				}
				else // not scrollT + scrollL
				{
					// h scroll - remember to check the header when scrolling H so no double blits
					//			  add to draw list uncovered area
					if (scrollT)
					{
						CRgn rgn;
						if (updatePar & DRAW_TRHEADER)
						{
							const RECT patR = {XOFFSET,YOFFSET , CW, CH};
							devc->ScrollDC(scrollT*ROWWIDTH,0,&patR,&patR,&rgn,&rect);
							if ( scrollT > 0 )
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HSCROLL+\n");
		#endif
								rect.top = YOFFSET;
								rect.bottom = CH;

								int xOffset = XOFFSET-1;
								for (int i = 0; i < scrollT; i++)
								{
								rect.left = xOffset;
								rect.right = xOffset+1;
									devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
									rect.left++;
									rect.right+= ROWWIDTH-1;
									devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

									xOffset += ROWWIDTH;
								}
								DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
							}
							else 
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HSCROLL-\n");
		#endif
								rect.top = YOFFSET;
								rect.bottom = CH;
								int xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
								for (int i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
								{
									rect.left = xOffset;
									rect.right = xOffset+1;
									devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
									rect.left++;
									rect.right+= ROWWIDTH-1;
									devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

									xOffset += ROWWIDTH;
								}
								DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
							}
						}
						else
						{
							// scroll header too
							const RECT trkR = {XOFFSET, 0, CW, CH};
							devc->ScrollDC(scrollT*ROWWIDTH,0,&trkR,&trkR,&rgn,&rect);
							if (scrollT > 0)
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HSCROLL+\n");
		#endif
								rect.top = 0;
								rect.bottom = CH;
								int xOffset = XOFFSET-1;
								for (int i = 0; i < scrollT; i++)
								{
									rect.left = xOffset;
									rect.right = xOffset+1;
									devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
									rect.left++;
									rect.right+= ROWWIDTH-1;
									devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

									xOffset += ROWWIDTH;
								}
								DrawPatternData(devc,0, scrollT, 0, VISLINES+1);

								CDC memDC;
								CBitmap *oldbmp;
								memDC.CreateCompatibleDC(devc);
								oldbmp = memDC.SelectObject(&patternheader);
								xOffset = XOFFSET-1;

								if (PatHeaderCoords.bHasTransparency)
								{
									for(int i=tOff;i<tOff+scrollT;i++)
									{
										const int trackx0 = i/10;
										const int track0x = i%10;

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT,
											1,
											PatHeaderCoords.sBackground.width, 
											PatHeaderCoords.sBackground.height,
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sBackground.x,
											PatHeaderCoords.sBackground.y);
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
											1+PatHeaderCoords.dDigitX0.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y);
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
											1+PatHeaderCoords.dDigit0X.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y);
										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										if (Global::_pSong->_trackMuted[i])
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y);

										if (Global::_pSong->_trackArmed[i])
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y);

										if (Global::_pSong->_trackSoloed == i )
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
												1+PatHeaderCoords.dSoloOn.y, 
												PatHeaderCoords.sSoloOn.width, 
												PatHeaderCoords.sSoloOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sSoloOn.x, 
												PatHeaderCoords.sSoloOn.y);
										xOffset += ROWWIDTH;
									}
								}
								else
								{
									for(int i=tOff;i<tOff+scrollT;i++)
									{
										const int trackx0 = i/10;
										const int track0x = i%10;

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										devc->BitBlt(
											xOffset+1+HEADER_INDENT,
											1,
											PatHeaderCoords.sBackground.width, 
											PatHeaderCoords.sBackground.height,
											&memDC, 
											PatHeaderCoords.sBackground.x,
											PatHeaderCoords.sBackground.y, 
											SRCCOPY);
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
											1+PatHeaderCoords.dDigitX0.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y, 
											SRCCOPY);
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
											1+PatHeaderCoords.dDigit0X.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y, 
											SRCCOPY);

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										if (Global::_pSong->_trackMuted[i])
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y, 
												SRCCOPY);

										if (Global::_pSong->_trackArmed[i])
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y, 
												SRCCOPY);

										if (Global::_pSong->_trackSoloed == i )
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
												1+PatHeaderCoords.dSoloOn.y, 
												PatHeaderCoords.sSoloOn.width, 
												PatHeaderCoords.sSoloOn.height, 
												&memDC, 
												PatHeaderCoords.sSoloOn.x, 
												PatHeaderCoords.sSoloOn.y, 
												SRCCOPY);
										xOffset += ROWWIDTH;
									}
								}
								memDC.SelectObject(oldbmp);
								memDC.DeleteDC();
							}
							else 
							{	
		#ifdef _DEBUG_PATVIEW
								TRACE("DRAW_HSCROLL-\n");
		#endif
								rect.top = 0;
								rect.bottom = CH;
								int xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
								for (int i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
								{
									rect.left = xOffset;
									rect.right = xOffset+1;
									devc->FillSolidRect(&rect,pvc_separator[i+tOff+1]);
									rect.left++;
									rect.right+= ROWWIDTH-1;
									devc->FillSolidRect(&rect,pvc_background[i+tOff+1]);

									xOffset += ROWWIDTH;
								}
								DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);

								CDC memDC;
								CBitmap *oldbmp;
								memDC.CreateCompatibleDC(devc);
								oldbmp = memDC.SelectObject(&patternheader);
								xOffset = XOFFSET-1+((maxt+scrollT-1)*ROWWIDTH);

								if (PatHeaderCoords.bHasTransparency)
								{
									for(int i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
									{
										const int trackx0 = i/10;
										const int track0x = i%10;

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT,
											1,
											PatHeaderCoords.sBackground.width, 
											PatHeaderCoords.sBackground.height,
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sBackground.x,
											PatHeaderCoords.sBackground.y);
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
											1+PatHeaderCoords.dDigitX0.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y);
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
											1+PatHeaderCoords.dDigit0X.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y);
										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										if (Global::_pSong->_trackMuted[i])
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y);

										if (Global::_pSong->_trackArmed[i])
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y);

										if (Global::_pSong->_trackSoloed == i )
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
												1+PatHeaderCoords.dSoloOn.y, 
												PatHeaderCoords.sSoloOn.width, 
												PatHeaderCoords.sSoloOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sSoloOn.x, 
												PatHeaderCoords.sSoloOn.y);
										xOffset += ROWWIDTH;
									}
								}
								else
								{
									for(int i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
									{
										const int trackx0 = i/10;
										const int track0x = i%10;

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										devc->BitBlt(
											xOffset+1+HEADER_INDENT,
											1,
											PatHeaderCoords.sBackground.width, 
											PatHeaderCoords.sBackground.height,
											&memDC, 
											PatHeaderCoords.sBackground.x,
											PatHeaderCoords.sBackground.y, 
											SRCCOPY);
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigitX0.x, 
											1+PatHeaderCoords.dDigitX0.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											PatHeaderCoords.sNumber0.x+(trackx0*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y, 
											SRCCOPY);
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dDigit0X.x, 
											1+PatHeaderCoords.dDigit0X.y, 
											PatHeaderCoords.sNumber0.width,	 
											PatHeaderCoords.sNumber0.height, 
											&memDC, 
											PatHeaderCoords.sNumber0.x+(track0x*PatHeaderCoords.sNumber0.width), 
											PatHeaderCoords.sNumber0.y, 
											SRCCOPY);

										// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
										if (Global::_pSong->_trackMuted[i])
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y, 
												SRCCOPY);

										if (Global::_pSong->_trackArmed[i])
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y, 
												SRCCOPY);

										if (Global::_pSong->_trackSoloed == i )
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dSoloOn.x, 
												1+PatHeaderCoords.dSoloOn.y, 
												PatHeaderCoords.sSoloOn.width, 
												PatHeaderCoords.sSoloOn.height, 
												&memDC, 
												PatHeaderCoords.sSoloOn.x, 
												PatHeaderCoords.sSoloOn.y, 
												SRCCOPY);
										xOffset += ROWWIDTH;
									}
								}
								memDC.SelectObject(oldbmp);
								memDC.DeleteDC();
							}
						}
						// Fill Bottom Space with Background colour if needed
						if (maxl < VISLINES+1)
						{
							int xOffset = XOFFSET;
							CRect rect;
							rect.top=YOFFSET+(maxl*ROWHEIGHT); 
							rect.bottom=CH;
							for(int i=tOff;i<tOff+maxt;i++)
							{
								rect.left=xOffset; 
								rect.right=xOffset+ROWWIDTH; 
								devc->FillSolidRect(&rect,pvc_separator[i+1]);
								xOffset += ROWWIDTH;
							}
						}
						// Fill Right Space with Background colour if needed
						if (maxt < VISTRACKS+1)
						{
		#ifdef _DEBUG_PATVIEW
							TRACE("DRAW_RIGHT\n");
		#endif
							CRect rect;
							rect.top=0; 
							rect.bottom=CH;  
							rect.right=CW;
							rect.left=XOFFSET+(maxt*ROWWIDTH)-1;
							devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
						}
					}

					// v scroll - 
					//			  add to draw list uncovered area
					else if (scrollL)
					{
						const RECT linR = {0, YOFFSET, CW, CH};

						CRgn rgn;
						devc->ScrollDC(0,scrollL*ROWHEIGHT,&linR,&linR,&rgn,&rect);
						// add visible part to 
						if (scrollL > 0)
						{	
		#ifdef _DEBUG_PATVIEW
							TRACE("DRAW_VSCROLL+\n");
		#endif
							//if(editcur.line!=0)
							DrawPatternData(devc, 0, VISTRACKS+1, 0,scrollL);
						}
						else 
						{	
		#ifdef _DEBUG_PATVIEW
							TRACE("DRAW_VSCROLL-\n");
		#endif
							DrawPatternData(devc, 0, VISTRACKS+1,VISLINES+scrollL,VISLINES+1);
						}
						// Fill Bottom Space with Background colour if needed
						if (maxl < VISLINES+1)
						{
		#ifdef _DEBUG_PATVIEW
							TRACE("DRAW_BOTTOM\n");
		#endif
							if (XOFFSET!=1)
							{
								CRect rect;
								rect.left=0; 
								rect.right=XOFFSET; 
								rect.top=YOFFSET+(maxl*ROWHEIGHT); 
								rect.bottom=CH;
								devc->FillSolidRect(&rect,pvc_separator[0]);
							}

							int xOffset = XOFFSET;

							CRect rect;
							rect.top=YOFFSET+(maxl*ROWHEIGHT); 
							rect.bottom=CH;
							for(int i=tOff;i<tOff+maxt;i++)
							{
								rect.left=xOffset; 
								rect.right=xOffset+ROWWIDTH; 
								devc->FillSolidRect(&rect,pvc_separator[i+1]);
								xOffset += ROWWIDTH;
							}
						}
					}
				}

				// then we draw any data that needs to be drawn
				// each time we draw data check for playbar or cursor, not fast but...
				// better idea is to have an array of flags, so never draw twice
				////////////////////////////////////////////////////////////
				////////////////////////////////////////////////////////////
				// Draw Pattern data.
				if (updatePar & DRAW_DATA)
				{
		#ifdef _DEBUG_PATVIEW
					TRACE("DRAW_DATA\n");
		#endif
					////////////////////////////////////////////////
					// Draw Data Changed (draw_modes::dataChange)
					for (int i = 0; i < numPatternDraw; i++)
					{

						int ts = pPatternDraw[i].drawTrackStart-tOff;
						if ( ts < 0 ) 
							ts = 0;
						int te = pPatternDraw[i].drawTrackEnd -(tOff-1);
						if ( te > maxt ) 
							te = maxt;

						int ls = pPatternDraw[i].drawLineStart-lOff;
						if ( ls < 0 ) 
							ls = 0;
						int le = pPatternDraw[i].drawLineEnd-(lOff-1);
						if ( le > maxl ) 
							le = maxl;

						DrawPatternData(devc,ts,te,ls,le);
					}
					numPatternDraw = 0;
				}
			}

			playpos=newplaypos;
			selpos=newselpos;
			editlast=editcur;

			devc->SelectObject(oldFont);

			updateMode = draw_modes::none;
			updatePar = 0;
		}


		// ADVISE! [lOff+lstart..lOff+lend] and [tOff+tstart..tOff+tend] HAVE TO be valid!
		void CChildView::DrawPatternData(CDC *devc,int tstart,int tend, int lstart, int lend)
		{

		#ifdef _DEBUG_PATVIEW
			TRACE("DrawPatternData\n");
		#endif

		//	if (lstart > VISLINES)
			if (lstart > maxl)
			{
				return;
			}
			else if (lstart < 0)
			{
				lstart = 0;
			}


			if (lend < 0)
			{
				return;
			}
			else if (lend > maxl)
		//	else if (lend > VISLINES+1)
			{
		//		lend = VISLINES+1;
				lend = maxl;
			}

		//	if (tstart > VISTRACKS)
			if (tstart > maxt)
			{
				return;
			}
			else if (tstart < 0)
			{
				tstart = 0;
			}

			if (tend < 0)
			{
				return;
			}
		//	else if (tend > VISTRACKS+1)
			else if (tend > maxt)
			{
		//		tend = VISTRACKS+1;
				tend = maxt;
			}

			int yOffset=lstart*ROWHEIGHT+YOFFSET;
			int linecount=lOff+ lstart;
			char tBuf[16];

			COLORREF* pBkg;
			for (int i=lstart;i<lend;i++) // Lines
			{
				// break this up into several more general loops for speed
				if((linecount%_pSong->LinesPerBeat()) == 0)
				{
					if ((linecount%(_pSong->LinesPerBeat()*Global::pConfig->pv_timesig)) == 0) 
						pBkg = pvc_row4beat;
					else 
						pBkg = pvc_rowbeat;
				}
				else
				{
					pBkg = pvc_row;
				}

				if ((XOFFSET!=1))// && (tstart == 0))
				{
					if ((linecount == editcur.line) && (Global::pConfig->_linenumbersCursor))
					{
						devc->SetBkColor(pvc_cursor[0]);
						devc->SetTextColor(pvc_fontCur[0]);
					}
					else if (linecount == newplaypos)
					{
						devc->SetBkColor(pvc_playbar[0]);
						devc->SetTextColor(pvc_fontPlay[0]);
					}
					else 
					{
						devc->SetBkColor(pBkg[0]);
						devc->SetTextColor(pvc_font[0]);
					}
					if (Global::pConfig->_linenumbersHex)
					{
						sprintf(tBuf," %.2X",linecount);
						TXTFLAT(devc,tBuf,1,yOffset,XOFFSET-2,ROWHEIGHT-1);	// Print Line Number.
					}
					else
					{
						sprintf(tBuf,"%3i",linecount);
						TXTFLAT(devc,tBuf,1,yOffset,XOFFSET-2,ROWHEIGHT-1);	// Print Line Number.
					}
				}

				unsigned char *patOffset = _ppattern() +
											(linecount*MULTIPLY) + 	(tstart+tOff)*EVENT_SIZE;

				int xOffset= XOFFSET+(tstart*ROWWIDTH);

				int trackcount = tstart+tOff;
				for (int t=tstart;t<tend;t++)
				{
					if (linecount == newplaypos)
					{
						devc->SetBkColor(pvc_playbar[trackcount]);
						devc->SetTextColor(pvc_fontPlay[trackcount]);
					}
					else if ((linecount >= newselpos.top) && 
						(linecount < newselpos.bottom) &&
						(trackcount >= newselpos.left) &&
						(trackcount < newselpos.right))
					{

						if (pBkg == pvc_rowbeat)
						{
							devc->SetBkColor(pvc_selectionbeat[trackcount]);
						}
						else if (pBkg == pvc_row4beat)
						{
							devc->SetBkColor(pvc_selection4beat[trackcount]);
						}
						else
						{
							devc->SetBkColor(pvc_selection[trackcount]);
						}
						devc->SetTextColor(pvc_fontSel[trackcount]);
					}
					else
					{
						devc->SetBkColor(pBkg[trackcount]);
						devc->SetTextColor(pvc_font[trackcount]);
					}
					OutNote(devc,xOffset+COLX[0],yOffset,*patOffset);
					if (*(++patOffset) == 255 )
					{
						OutData(devc,xOffset+COLX[1],yOffset,0,true);
					}
					else
					{
						OutData(devc,xOffset+COLX[1],yOffset,*patOffset,false);
					}

					if (*(++patOffset) == 255 )
					{
						OutData(devc,xOffset+COLX[3],yOffset,0,true);
					}
					else 
					{
						OutData(devc,xOffset+COLX[3],yOffset,*patOffset,false);
					}

					if (*(++patOffset) == 0 && *(patOffset+1) == 0 && 
						(*(patOffset-3) <= notecommands::release || *(patOffset-3) == 255 ))
					{
						OutData(devc,xOffset+COLX[5],yOffset,0,true);
						patOffset++;
						OutData(devc,xOffset+COLX[7],yOffset,0,true);
					}
					else
					{
						OutData(devc,xOffset+COLX[5],yOffset,*patOffset,false);
						patOffset++;
						OutData(devc,xOffset+COLX[7],yOffset,*patOffset,false);
					}
					// could optimize this check some, make separate loops
					if ((linecount == editcur.line) && (trackcount == editcur.track))
					{
						devc->SetBkColor(pvc_cursor[trackcount]);
						devc->SetTextColor(pvc_fontCur[trackcount]);
						switch (editcur.col)
						{
						case 0:
							OutNote(devc,xOffset+COLX[0],yOffset,*(patOffset-4));
							break;
						case 1:
							if (*(patOffset-3) == 255 )
							{
								OutData4(devc,xOffset+COLX[1],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[1],yOffset,(*(patOffset-3))>>4,false);
							}
							break;
						case 2:
							if (*(patOffset-3) == 255 )
							{
								OutData4(devc,xOffset+COLX[2],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[2],yOffset,*(patOffset-3),false);
							}
							break;
						case 3:
							if (*(patOffset-2) == 255 )
							{
								OutData4(devc,xOffset+COLX[3],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[3],yOffset,(*(patOffset-2))>>4,false);
							}
							break;
						case 4:
							if (*(patOffset-2) == 255 )
							{
								OutData4(devc,xOffset+COLX[4],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[4],yOffset,*(patOffset-2),false);
							}
							break;
						case 5:
							if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
								(*(patOffset-4) <= notecommands::release || *(patOffset-4) == 255 ))
							{
								OutData4(devc,xOffset+COLX[5],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[5],yOffset,(*(patOffset-1))>>4,false);
							}
							break;
						case 6:
							if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
								(*(patOffset-4) <= notecommands::release || *(patOffset-4) == 255 ))
							{
								OutData4(devc,xOffset+COLX[6],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[6],yOffset,(*(patOffset-1)),false);
							}
							break;
						case 7:
							if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
								(*(patOffset-4) <= notecommands::release || *(patOffset-4) == 255 ))
							{
								OutData4(devc,xOffset+COLX[7],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[7],yOffset,(*(patOffset))>>4,false);
							}
							break;
						case 8:
							if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
								(*(patOffset-4) <= notecommands::release || *(patOffset-4) == 255 ))
							{
								OutData4(devc,xOffset+COLX[8],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[8],yOffset,(*(patOffset)),false);
							}
							break;
						}
					}
					trackcount++;
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
	#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
	#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
					patOffset+=2;
	#else
					patOffset++;
	#endif
#endif
					xOffset+=ROWWIDTH;
				}
				linecount++;
				yOffset+=ROWHEIGHT;
			}
		}

		void CChildView::NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd)
		{
			if (viewMode == view_modes::pattern)
			{
				if (!(updatePar & DRAW_FULL_DATA))
				{
					// inserts pattern data to be drawn into the list
					if (numPatternDraw < MAX_DRAW_MESSAGES)
					{
						for (int i=0; i < numPatternDraw; i++)
						{
							if ((pPatternDraw[i].drawTrackStart <= drawTrackStart) &&
								(pPatternDraw[i].drawTrackEnd >= drawTrackEnd) &&
								(pPatternDraw[i].drawLineStart <= drawLineStart) &&
								(pPatternDraw[i].drawLineEnd >= drawLineEnd))
							{
								return;
							}
						}
						pPatternDraw[numPatternDraw].drawTrackStart = drawTrackStart;
						pPatternDraw[numPatternDraw].drawTrackEnd = drawTrackEnd;
						pPatternDraw[numPatternDraw].drawLineStart = drawLineStart;
						pPatternDraw[numPatternDraw].drawLineEnd = drawLineEnd;
						numPatternDraw++;
					}
					else if (numPatternDraw == MAX_DRAW_MESSAGES)
					{
						// this should never have to happen with a 32 message buffer, but just incase....
						numPatternDraw++;
						PreparePatternRefresh(draw_modes::all);
					}
				}
			}
			else
			{
				numPatternDraw=0;
			}
		}

		void CChildView::RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2)
		{
			// makes an array of colours between source1 and source2
			float p0 = float((source1>>16)&0xff);
			float p1 = float((source1>>8)&0xff);
			float p2 = float(source1&0xff);

			float d0 = float((source2>>16)&0xff);
			float d1 = float((source2>>8)&0xff);
			float d2 = float(source2&0xff);

			int len = _pSong->SONGTRACKS+1;

			float a0=(d0-p0)/(len);
			float a1=(d1-p1)/(len);
			float a2=(d2-p2)/(len);

			for (int i = 0; i < len; i++)
			{
				pDest[i] = (helpers::math::rounded(p0*0x10000)&0xff0000)
							| (helpers::math::rounded(p1*0x100)&0xff00)
							| (helpers::math::rounded(p2)&0xff);
				p0+=a0;
				p1+=a1;
				p2+=a2;

				if (p0 < 0)
				{
					p0 = 0;
				}
				else if (p0 > 255)
				{
					p0 = 255;
				}

				if (p1 < 0)
				{
					p1 = 0;
				}
				else if (p1 > 255)
				{
					p1 = 255;
				}

				if (p2 < 0)
				{
					p2 = 2;
				}
				else if (p2 > 255)
				{
					p2 = 255;
				}
			}
		}

		COLORREF CChildView::ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add)
		{
			int a0 = ((add>>16)&0x0ff)+((adjust>>16)&0x0ff)-((base>>16)&0x0ff);
			int a1 = ((add>>8 )&0x0ff)+((adjust>>8 )&0x0ff)-((base>>8 )&0x0ff);
			int a2 = ((add    )&0x0ff)+((adjust    )&0x0ff)-((base    )&0x0ff);

			if (a0 < 0)
			{
				a0 = 0;
			}
			else if (a0 > 255)
			{
				a0 = 255;
			}

			if (a1 < 0)
			{
				a1 = 0;
			}
			else if (a1 > 255)
			{
				a1 = 255;
			}

			if (a2 < 0)
			{
				a2 = 0;
			}
			else if (a2 > 255)
			{
				a2 = 255;
			}

			COLORREF pa = (a0<<16) | (a1<<8) | (a2);
			return pa;
		}

		void CChildView::RecalculateColourGrid()
		{
			RecalculateColour(pvc_background, Global::pConfig->pvc_background, Global::pConfig->pvc_background2);
			RecalculateColour(pvc_separator, Global::pConfig->pvc_separator, Global::pConfig->pvc_separator2);
			RecalculateColour(pvc_row4beat, Global::pConfig->pvc_row4beat, Global::pConfig->pvc_row4beat2);
			RecalculateColour(pvc_rowbeat, Global::pConfig->pvc_rowbeat, Global::pConfig->pvc_rowbeat2);
			RecalculateColour(pvc_row, Global::pConfig->pvc_row, Global::pConfig->pvc_row2);
			RecalculateColour(pvc_selection, Global::pConfig->pvc_selection, Global::pConfig->pvc_selection2);
			RecalculateColour(pvc_playbar, Global::pConfig->pvc_playbar, Global::pConfig->pvc_playbar2);
			RecalculateColour(pvc_cursor, Global::pConfig->pvc_cursor, Global::pConfig->pvc_cursor2);
			RecalculateColour(pvc_font, Global::pConfig->pvc_font, Global::pConfig->pvc_font2);
			RecalculateColour(pvc_fontPlay, Global::pConfig->pvc_fontPlay, Global::pConfig->pvc_fontPlay2);
			RecalculateColour(pvc_fontCur, Global::pConfig->pvc_fontCur, Global::pConfig->pvc_fontCur2);
			RecalculateColour(pvc_fontSel, Global::pConfig->pvc_fontSel, Global::pConfig->pvc_fontSel2);
			RecalculateColour(pvc_selectionbeat, ColourDiffAdd(Global::pConfig->pvc_row, Global::pConfig->pvc_rowbeat, Global::pConfig->pvc_selection), ColourDiffAdd(Global::pConfig->pvc_row2, Global::pConfig->pvc_rowbeat2, Global::pConfig->pvc_selection2));
			RecalculateColour(pvc_selection4beat, ColourDiffAdd(Global::pConfig->pvc_row, Global::pConfig->pvc_row4beat, Global::pConfig->pvc_selection), ColourDiffAdd(Global::pConfig->pvc_row2, Global::pConfig->pvc_row4beat2, Global::pConfig->pvc_selection2));
		}

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

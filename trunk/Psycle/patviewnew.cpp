
#define DRAW_DATA		1
#define DRAW_HSCROLL	2
#define DRAW_VSCROLL	4
#define DRAW_TRHEADER	8
#define DRAW_FULL_DATA	16

void CChildView::PreparePatternRefresh(int drawMode)
{
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
			if ( drawMode == DMHScroll )
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
			if ( drawMode == DMHScroll ) 
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
			if ( drawMode == DMHScroll )
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
			if ( drawMode == DMVScroll )
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
			if ( drawMode == DMVScroll ) 
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
			if ( drawMode == DMVScroll )
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
	
	if (drawMode != DMAll && drawMode != DMPattern)
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
	case DMAll: 
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
	case DMPattern: 
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
	case DMPlayback: 
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
	case DMPlaybackChange: 
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
	case DMSelection: 
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
	case DMData: 
		{
			int drawTrackStart = pPatternDraw->drawTrackStart-rntOff;
//			if ( drawTrackStart < 0 ) drawTrackStart = 0;
			int drawTrackEnd = pPatternDraw->drawTrackEnd-(rntOff-1);
//			if ( drawTrackEnd > maxt ) drawTrackEnd = maxt;

			int drawLineStart = pPatternDraw->drawLineStart-rnlOff;
//			if ( drawLineStart < 0 ) drawLineStart = 0;
			int drawLineEnd = pPatternDraw->drawLineEnd-(rnlOff-1);
//			if ( drawLineEnd > maxl ) drawLineEnd = maxl;
			
			rect.left=XOFFSET+ drawTrackStart*ROWWIDTH;
			rect.right=XOFFSET+drawTrackEnd*ROWWIDTH;
			rect.top=YOFFSET+	drawLineStart*ROWHEIGHT;
			rect.bottom=YOFFSET+drawLineEnd*ROWHEIGHT;
			updatePar |= DRAW_DATA;
			InvalidateRect(rect,false);
		}
		break;
	case DMTrackHeader: 
		// header
		rect.top=0; 
		rect.left=XOFFSET;
		rect.bottom=YOFFSET-1;	
		rect.right=XOFFSET+maxt*ROWWIDTH;
		updatePar |= DRAW_TRHEADER;
		InvalidateRect(rect,false);
		break;
//	case DMCursor: 
//		break;
	case DMNone: 
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
		if ((editcur.line != editlast.line) || (editcur.track != editlast.track))
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
	UpdateWindow();
}



/*
Undraw gui/selpos and playpos

if ModeHeader -> Draw header

if ModeScroll  ->  scrollT scrollL

if ModeData -> 	if needed, Draw track headers
				draw inittrack till endtrack
				draw iniline   till endline
				draw data modification.

				fillsolidrect if needed
				fillsolidrect if needed


draw new guipos and playpos and "guipos=newguipos"
*/

#define DF_NONE			0
#define	DF_SELECTION	1
#define DF_PLAYBAR		2
#define DF_CURSOR		4
#define DF_DRAWN		15

void CChildView::DrawPatEditor(CDC *devc)
{
	///////////////////////////////////////////////////////////
	// Prepare pattern for update (Undraw guipos and playpos)
	int scrollT= tOff-rntOff;
	int scrollL= lOff-rnlOff;

	tOff = ntOff = rntOff; 
	lOff = nlOff = rnlOff;

	CBrush* oldBrush;
	CFont* oldfont;
	CPen* oldPen;
	CBrush brush(Global::pConfig->pvc_background); // This affects BOX background
	oldBrush= devc->SelectObject(&brush);
	CPen linepen( PS_SOLID, 1, Global::pConfig->pvc_separator); // this affects BOX foreground
	oldPen = devc->SelectObject(&linepen);
	devc->SetBkColor(Global::pConfig->pvc_background);	// This affects TXT background
	devc->SetTextColor(Global::pConfig->pvc_font);


	// 1 if there is a redraw header, we do that 
	/////////////////////////////////////////////////////////////
	// Update Mute/Solo Indicators
	oldfont= devc->SelectObject(&Global::pConfig->seqFont);
	if ((updatePar & DRAW_TRHEADER) || (abs(scrollT) > VISTRACKS))
	{
		if (XOFFSET!=1)
		{
			BOX(devc, 0, 0, XOFFSET-1, YOFFSET-1); // Draw lines between tracks
			TXT(devc,"Line",1,1,XOFFSET-2,YOFFSET-2);
		}
		CDC memDC;
		CBitmap *oldbmp;
		memDC.CreateCompatibleDC(devc);
		oldbmp = memDC.SelectObject(&stuffbmp);
		int xOffset = XOFFSET-1;

		for(int i=tOff;i<tOff+maxt;i++)
		{
			CBrush newbrush(pvc_background[i+1]); // This affects BOX background
			oldBrush= devc->SelectObject(&newbrush);
			CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
			oldPen = devc->SelectObject(&newpen);
			BOX(devc, xOffset, 0, ROWWIDTH+1, YOFFSET); // Draw lines between track headers
			newbrush.DeleteObject();
			newpen.DeleteObject();

			const int trackx0 = i/10;
			const int track0x = i%10;

			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			devc->BitBlt(xOffset+1,		19-18, 110, 16, &memDC, 148,			65, SRCCOPY);
			devc->BitBlt(xOffset+35-11, 21-18, 7,	 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
			devc->BitBlt(xOffset+42-11, 21-18, 7,	 12, &memDC, 148+track0x*7, 81, SRCCOPY);

			if (Global::_pSong->_trackMuted[i])
				devc->BitBlt(xOffset+71+5, 24-18, 7, 7, &memDC, 258, 49, SRCCOPY);

			if (Global::_pSong->_trackArmed[i])
				devc->BitBlt(xOffset+71-18, 24-18, 7, 7, &memDC, 276, 49, SRCCOPY);

			if (Global::_pSong->_trackSoloed == i )
				devc->BitBlt(xOffset+97, 24-18, 7, 7, &memDC, 267, 49, SRCCOPY);

			xOffset += ROWWIDTH;
		}
		memDC.SelectObject(oldbmp);
		memDC.DeleteDC();
	}

	// 2 if there is a redraw all, we do that then exit
	if ((updatePar & DRAW_FULL_DATA) || (abs(scrollT) > VISTRACKS) || (abs(scrollL) > VISLINES))
	{
		TRACE("DRAW_FULL_DATA\n");
		// draw everything
		if (XOFFSET!=1)
		{
			CBrush newbrush(pvc_background[0]); // This affects BOX background
			oldBrush= devc->SelectObject(&newbrush);
			CPen newpen( PS_SOLID, 1, pvc_separator[0]); // this affects BOX foreground
			oldPen = devc->SelectObject(&newpen);
			BOX(devc, 0, YOFFSET-1, XOFFSET+1, CH-YOFFSET+2); // Draw lines between tracks
			newbrush.DeleteObject();
			newpen.DeleteObject();
		}
		int xOffset = XOFFSET-1;

		for (int i=tOff;i<tOff+maxt;i++)
		{
			CBrush newbrush(pvc_background[i+1]); // This affects BOX background
			oldBrush= devc->SelectObject(&newbrush);
			CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
			oldPen = devc->SelectObject(&newpen);
			BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
			newbrush.DeleteObject();
			newpen.DeleteObject();

			xOffset += ROWWIDTH;
		}
		DrawPatternData(devc,0,VISTRACKS+1,0,VISLINES+1);
		// wipe todo list
		while (pPatternDraw)
		{
			SPatternDraw* temp = pPatternDraw->pPrev;
			delete pPatternDraw;
			pPatternDraw = temp;
		}
		// Fill Bottom Space with Background colour if needed
		if (maxl < VISLINES+1)
		{
			TRACE("DRAW_BOTTOM\n");
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
			TRACE("DRAW_RIGHT\n");
			CRect rect;
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
			// scroll x AND y
			CRect rect2;
			CRgn rgn;
			if (XOFFSET!=1)
			{
				const RECT linR = {0, YOFFSET, XOFFSET, CH};
				devc->ScrollDC(0,scrollL*ROWHEIGHT,&linR,&linR,&rgn,&rect2);
			}
			const RECT patR = {XOFFSET,YOFFSET , CW, CH};
			devc->ScrollDC(scrollT*ROWWIDTH,scrollL*ROWHEIGHT,&patR,&patR,&rgn,&rect2);
			if (updatePar & DRAW_TRHEADER)
			{
				if (scrollT > 0)
				{	
					int xOffset = XOFFSET-1;
					for (int i = 0; i < scrollT; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					xOffset = XOFFSET-1+((VISTRACKS-scrollT)*ROWWIDTH);
					for (i = VISTRACKS-scrollT; i < VISTRACKS+1; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
					DrawPatternData(devc, VISTRACKS-scrollT-1, VISTRACKS+1, 0,VISLINES+1);
					if (scrollL > 0)
					{	
						TRACE("DRAW_HVSCROLL++\n");
						DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1, 0,scrollL);
					}
					else 
					{	
						TRACE("DRAW_HVSCROLL+-\n");
						DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1,VISLINES+scrollL,VISLINES+1);
					}
				}
				else 
				{	
					int xOffset = XOFFSET-1;
					for (int i = 0; i < 1-scrollT; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
					for (i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
					DrawPatternData(devc,0, 1-scrollT, 0, VISLINES+1);
					if (scrollL > 0)
					{	
						TRACE("DRAW_HVSCROLL-+\n");
						DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT, 0,scrollL);
					}
					else
					{	
						TRACE("DRAW_HVSCROLL--\n");
						DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT,VISLINES+scrollL,VISLINES+1);
					}
				}
			}
			else
			{
				// scroll header too
				const RECT trkR = {XOFFSET, 0, CW, YOFFSET-1};
				devc->ScrollDC(scrollT*ROWWIDTH,0,&trkR,&trkR,&rgn,&rect2);
				if (scrollT > 0)
				{	
					int xOffset = XOFFSET-1;
					for (int i = 0; i < scrollT; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					xOffset = XOFFSET-1+((VISTRACKS-scrollT)*ROWWIDTH);
					for (i = VISTRACKS-scrollT; i < VISTRACKS+1; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
					DrawPatternData(devc, VISTRACKS-scrollT-1, VISTRACKS+1, 0,VISLINES+1);
					if (scrollL > 0)
					{	
						TRACE("DRAW_HVSCROLL++H\n");
						DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1, 0,scrollL);
					}
					else 
					{	
						TRACE("DRAW_HVSCROLL+-H\n");
						DrawPatternData(devc, scrollT, VISTRACKS-scrollT-1,VISLINES+scrollL,VISLINES+1);
					}

					CDC memDC;
					CBitmap *oldbmp;
					memDC.CreateCompatibleDC(devc);
					oldbmp = memDC.SelectObject(&stuffbmp);
					xOffset = XOFFSET-1;

					for(i=tOff;i<tOff+scrollT;i++)
					{
						CBrush newbrush(pvc_background[i+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, 0, ROWWIDTH+1, YOFFSET); // Draw lines between track headers
						newbrush.DeleteObject();
						newpen.DeleteObject();

						const int trackx0 = i/10;
						const int track0x = i%10;

						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						devc->BitBlt(xOffset+1,		19-18, 110, 16, &memDC, 148,			65, SRCCOPY);
						devc->BitBlt(xOffset+35-11, 21-18, 7,	 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
						devc->BitBlt(xOffset+42-11, 21-18, 7,	 12, &memDC, 148+track0x*7, 81, SRCCOPY);

						if (Global::_pSong->_trackMuted[i])
							devc->BitBlt(xOffset+71+5, 24-18, 7, 7, &memDC, 258, 49, SRCCOPY);

						if (Global::_pSong->_trackArmed[i])
							devc->BitBlt(xOffset+71-18, 24-18, 7, 7, &memDC, 276, 49, SRCCOPY);

						if (Global::_pSong->_trackSoloed == i )
							devc->BitBlt(xOffset+97, 24-18, 7, 7, &memDC, 267, 49, SRCCOPY);

						xOffset += ROWWIDTH;
					}
					memDC.SelectObject(oldbmp);
					memDC.DeleteDC();
				}
				else
				{	
					int xOffset = XOFFSET-1;
					for (int i = 0; i < 1-scrollT; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
					for (i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
					{
						CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
						newbrush.DeleteObject();
						newpen.DeleteObject();

						xOffset += ROWWIDTH;
					}
					DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
					DrawPatternData(devc,0, 1-scrollT, 0, VISLINES+1);
					if (scrollL > 0)
					{	
						TRACE("DRAW_HVSCROLL-+H\n");
						DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT, 0,scrollL);
					}
					else
					{	
						TRACE("DRAW_HVSCROLL--H\n");
						DrawPatternData(devc, 1-scrollT, VISTRACKS+scrollT,VISLINES+scrollL,VISLINES+1);
					}

					CDC memDC;
					CBitmap *oldbmp;
					memDC.CreateCompatibleDC(devc);
					oldbmp = memDC.SelectObject(&stuffbmp);
					xOffset = XOFFSET-1+((maxt+scrollT-1)*ROWWIDTH);

					for(i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
					{
						CBrush newbrush(pvc_background[i+1]); // This affects BOX background
						oldBrush= devc->SelectObject(&newbrush);
						CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
						oldPen = devc->SelectObject(&newpen);
						BOX(devc, xOffset, 0, ROWWIDTH+1, YOFFSET); // Draw lines between track headers
						newbrush.DeleteObject();
						newpen.DeleteObject();

						const int trackx0 = i/10;
						const int track0x = i%10;

						// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
						devc->BitBlt(xOffset+1,		19-18, 110, 16, &memDC, 148,			65, SRCCOPY);
						devc->BitBlt(xOffset+35-11, 21-18, 7,	 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
						devc->BitBlt(xOffset+42-11, 21-18, 7,	 12, &memDC, 148+track0x*7, 81, SRCCOPY);

						if (Global::_pSong->_trackMuted[i])
							devc->BitBlt(xOffset+71+5, 24-18, 7, 7, &memDC, 258, 49, SRCCOPY);

						if (Global::_pSong->_trackArmed[i])
							devc->BitBlt(xOffset+71-18, 24-18, 7, 7, &memDC, 276, 49, SRCCOPY);

						if (Global::_pSong->_trackSoloed == i )
							devc->BitBlt(xOffset+97, 24-18, 7, 7, &memDC, 267, 49, SRCCOPY);

						xOffset += ROWWIDTH;
					}
					memDC.SelectObject(oldbmp);
					memDC.DeleteDC();
				}
			}
			// Fill Bottom Space with Background colour if needed
			if (maxl < VISLINES+1)
			{
				TRACE("DRAW_BOTTOM\n");
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
				TRACE("DRAW_RIGHT\n");
				CRect rect;
				rect.top=0; 
				rect.bottom=CH;  
				rect.right=CW;
				rect.left=XOFFSET+(maxt*ROWWIDTH)-1;
				devc->FillSolidRect(&rect,Global::pConfig->pvc_separator2);
			}
		}
		else // not scrollT + scrollL
		{
			// h scroll - remember to check the header when scrolling H so no double blits
			//			  add to draw list uncovered area
			if (scrollT)
			{
				CRect rect2;
				CRgn rgn;
				if (updatePar & DRAW_TRHEADER)
				{
					const RECT patR = {XOFFSET,YOFFSET , CW, CH};
					devc->ScrollDC(scrollT*ROWWIDTH,0,&patR,&patR,&rgn,&rect2);
					if ( scrollT > 0 )
					{	
						TRACE("DRAW_HSCROLL+\n");
						int xOffset = XOFFSET-1;
						for (int i = 0; i < scrollT; i++)
						{
							CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
							newbrush.DeleteObject();
							newpen.DeleteObject();

							xOffset += ROWWIDTH;
						}
						DrawPatternData(devc,0, scrollT, 0, VISLINES+1);
					}
					else 
					{	
						TRACE("DRAW_HSCROLL-\n");
						int xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
						for (int i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
						{
							CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
							newbrush.DeleteObject();
							newpen.DeleteObject();

							xOffset += ROWWIDTH;
						}
						DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);
					}
				}
				else
				{
					// scroll header too
					const RECT trkR = {XOFFSET, 0, CW, CH};
					devc->ScrollDC(scrollT*ROWWIDTH,0,&trkR,&trkR,&rgn,&rect2);
					if (scrollT > 0)
					{	
						TRACE("DRAW_HSCROLL+\n");
						int xOffset = XOFFSET-1;
						for (int i = 0; i < scrollT; i++)
						{
							CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
							newbrush.DeleteObject();
							newpen.DeleteObject();

							xOffset += ROWWIDTH;
						}
						DrawPatternData(devc,0, scrollT, 0, VISLINES+1);

						CDC memDC;
						CBitmap *oldbmp;
						memDC.CreateCompatibleDC(devc);
						oldbmp = memDC.SelectObject(&stuffbmp);
						xOffset = XOFFSET-1;

						for(i=tOff;i<tOff+scrollT;i++)
						{
							CBrush newbrush(pvc_background[i+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, 0, ROWWIDTH+1, YOFFSET); // Draw lines between track headers
							newbrush.DeleteObject();
							newpen.DeleteObject();

							const int trackx0 = i/10;
							const int track0x = i%10;

							// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
							devc->BitBlt(xOffset+1,		19-18, 110, 16, &memDC, 148,			65, SRCCOPY);
							devc->BitBlt(xOffset+35-11, 21-18, 7,	 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
							devc->BitBlt(xOffset+42-11, 21-18, 7,	 12, &memDC, 148+track0x*7, 81, SRCCOPY);

							if (Global::_pSong->_trackMuted[i])
								devc->BitBlt(xOffset+71+5, 24-18, 7, 7, &memDC, 258, 49, SRCCOPY);

							if (Global::_pSong->_trackArmed[i])
								devc->BitBlt(xOffset+71-18, 24-18, 7, 7, &memDC, 276, 49, SRCCOPY);

							if (Global::_pSong->_trackSoloed == i )
								devc->BitBlt(xOffset+97, 24-18, 7, 7, &memDC, 267, 49, SRCCOPY);

							xOffset += ROWWIDTH;
						}
						memDC.SelectObject(oldbmp);
						memDC.DeleteDC();
					}
					else 
					{	
						TRACE("DRAW_HSCROLL-\n");
						int xOffset = XOFFSET-1+((VISTRACKS+scrollT)*ROWWIDTH);
						for (int i = VISTRACKS+scrollT; i < VISTRACKS+1; i++)
						{
							CBrush newbrush(pvc_background[i+tOff+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+tOff+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, YOFFSET-1, ROWWIDTH+1, CH-YOFFSET+2); // Draw lines between tracks
							newbrush.DeleteObject();
							newpen.DeleteObject();

							xOffset += ROWWIDTH;
						}
						DrawPatternData(devc,VISTRACKS+scrollT, VISTRACKS+1, 0, VISLINES+1);

						CDC memDC;
						CBitmap *oldbmp;
						memDC.CreateCompatibleDC(devc);
						oldbmp = memDC.SelectObject(&stuffbmp);
						xOffset = XOFFSET-1+((maxt+scrollT-1)*ROWWIDTH);

						for(i=tOff+maxt+scrollT-1;i<tOff+maxt;i++)
						{
							CBrush newbrush(pvc_background[i+1]); // This affects BOX background
							oldBrush= devc->SelectObject(&newbrush);
							CPen newpen( PS_SOLID, 1, pvc_separator[i+1]); // this affects BOX foreground
							oldPen = devc->SelectObject(&newpen);
							BOX(devc, xOffset, 0, ROWWIDTH+1, YOFFSET); // Draw lines between track headers
							newbrush.DeleteObject();
							newpen.DeleteObject();

							const int trackx0 = i/10;
							const int track0x = i%10;

							// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
							devc->BitBlt(xOffset+1,		19-18, 110, 16, &memDC, 148,			65, SRCCOPY);
							devc->BitBlt(xOffset+35-11, 21-18, 7,	 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
							devc->BitBlt(xOffset+42-11, 21-18, 7,	 12, &memDC, 148+track0x*7, 81, SRCCOPY);

							if (Global::_pSong->_trackMuted[i])
								devc->BitBlt(xOffset+71+5, 24-18, 7, 7, &memDC, 258, 49, SRCCOPY);

							if (Global::_pSong->_trackArmed[i])
								devc->BitBlt(xOffset+71-18, 24-18, 7, 7, &memDC, 276, 49, SRCCOPY);

							if (Global::_pSong->_trackSoloed == i )
								devc->BitBlt(xOffset+97, 24-18, 7, 7, &memDC, 267, 49, SRCCOPY);

							xOffset += ROWWIDTH;
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
					TRACE("DRAW_RIGHT\n");
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

				CRect rect2;
				CRgn rgn;
				devc->ScrollDC(0,scrollL*ROWHEIGHT,&linR,&linR,&rgn,&rect2);
				// add visible part to 
				if (scrollL > 0)
				{	
					TRACE("DRAW_VSCROLL+\n");
					DrawPatternData(devc, 0, VISTRACKS+1, 0,scrollL);
				}
				else 
				{	
					TRACE("DRAW_VSCROLL-\n");
					DrawPatternData(devc, 0, VISTRACKS+1,VISLINES+scrollL,VISLINES+1);
				}
				// Fill Bottom Space with Background colour if needed
				if (maxl < VISLINES+1)
				{
					TRACE("DRAW_BOTTOM\n");
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
			TRACE("DRAW_DATA\n");
			////////////////////////////////////////////////
			// Draw Data Changed (DMDataChange)
			while (pPatternDraw)
			{
				pPatternDraw->drawTrackStart-=tOff;
				if ( pPatternDraw->drawTrackStart < 0 ) 
					pPatternDraw->drawTrackStart = 0;
				pPatternDraw->drawTrackEnd -=(tOff-1);
				if ( pPatternDraw->drawTrackEnd > maxt ) 
					pPatternDraw->drawTrackEnd = maxt;

				pPatternDraw->drawLineStart-=lOff;
				if ( pPatternDraw->drawLineStart < 0 ) 
					pPatternDraw->drawLineStart = 0;
				pPatternDraw->drawLineEnd-=(lOff-1);
				if ( pPatternDraw->drawLineEnd > maxl ) 
					pPatternDraw->drawLineEnd = maxl;

				DrawPatternData(devc,pPatternDraw->drawTrackStart,pPatternDraw->drawTrackEnd,pPatternDraw->drawLineStart,pPatternDraw->drawLineEnd);
				
				SPatternDraw* temp = pPatternDraw->pPrev;
				delete pPatternDraw;
				pPatternDraw = temp;
			}
		}
	}

	playpos=newplaypos;
	selpos=newselpos;
	editlast=editcur;

	devc->SelectObject(oldBrush);
	devc->SelectObject(oldPen);
	devc->SelectObject(oldfont);
	brush.DeleteObject();
	linepen.DeleteObject();

	updateMode = DMNone;
	updatePar = 0;
}


// ADVISE! [lOff+lstart..lOff+lend] and [tOff+tstart..tOff+tend] HAVE TO be valid!
void CChildView::DrawPatternData(CDC *devc,int tstart,int tend, int lstart, int lend)
{
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
	COLORREF* pTxt;
	for (int i=lstart;i<lend;i++) // Lines
	{
		// break this up into several more general loops for speed
		if((linecount%_pSong->_ticksPerBeat) == 0)
		{
			if ((linecount%(_pSong->_ticksPerBeat*4)) == 0) 
				pBkg = pvc_row4beat;
			else 
				pBkg = pvc_rowbeat;
		}
		else
		{
			pBkg = pvc_row;
		}
		pTxt = pvc_font;

		if ((XOFFSET!=1) && (tstart == 0))
		{
			if (linecount == newplaypos)
			{
				devc->SetBkColor(pvc_playbar[0]);
				devc->SetTextColor(pvc_fontPlay[0]);
			}
			else 
			{
				devc->SetBkColor(pBkg[0]);
				devc->SetTextColor(pTxt[0]);
			}
			if (Global::pConfig->_linenumbersHex)
			{
				sprintf(tBuf," %.2X",linecount);
				TXTFLAT(devc,tBuf,1,yOffset,XOFFSET-2,ROWHEIGHT-1);	// Print Line Number.
			}
			else
			{
				sprintf(tBuf,"%d",linecount);
				TXTFLAT(devc,tBuf,1,yOffset,XOFFSET-2,ROWHEIGHT-1);	// Print Line Number.
			}
		}

		unsigned char *patOffset = _pSong->pPatternData +
									_pSong->playOrder[editPosition]*MULTIPLY2 +
									(linecount*MULTIPLY) + 	(tstart+tOff)*5;

		int xOffset= XOFFSET+tstart*ROWWIDTH;

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
				devc->SetTextColor(pTxt[trackcount]);
			}
			OutNote(devc,xOffset,yOffset,*patOffset);
			if (*++patOffset == 255 )
			{
				OutData(devc,xOffset+28,yOffset,0,true);
			}
			else
			{
				OutData(devc,xOffset+28,yOffset,*patOffset,false);
			}

			if (*++patOffset == 255 )
			{
				OutData(devc,xOffset+49,yOffset,0,true);
			}
			else 
			{
				OutData(devc,xOffset+49,yOffset,*patOffset,false);
			}

			if (*++patOffset == 0 && *(patOffset+1) == 0 && 
				(*(patOffset-3) <= 120 || *(patOffset-3) == 255 ))
			{
				OutData(devc,xOffset+70,yOffset,0,true);
				patOffset++;
				OutData(devc,xOffset+90,yOffset,0,true);
			}
			else
			{
				OutData(devc,xOffset+70,yOffset,*patOffset,false);
				patOffset++;
				OutData(devc,xOffset+90,yOffset,*patOffset,false);
			}
			// could optimize this check some, make separate loops
			if ((linecount == editcur.line) && (trackcount == editcur.track))
			{
				devc->SetBkColor(pvc_cursor[trackcount]);
				devc->SetTextColor(pvc_fontCur[trackcount]);
				switch (editcur.col)
				{
				case 0:
					OutNote(devc,xOffset,yOffset,*(patOffset-4));
					break;
				case 1:
					if (*(patOffset-3) == 255 )
					{
						OutData4(devc,xOffset+28,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+28,yOffset,(*(patOffset-3))>>4,false);
					}
					break;
				case 2:
					if (*(patOffset-3) == 255 )
					{
						OutData4(devc,xOffset+38,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+38,yOffset,*(patOffset-3),false);
					}
					break;
				case 3:
					if (*(patOffset-2) == 255 )
					{
						OutData4(devc,xOffset+49,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+49,yOffset,(*(patOffset-2))>>4,false);
					}
					break;
				case 4:
					if (*(patOffset-2) == 255 )
					{
						OutData4(devc,xOffset+59,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+59,yOffset,*(patOffset-2),false);
					}
					break;
				case 5:
					if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
						(*(patOffset-4) <= 120 || *(patOffset-4) == 255 ))
					{
						OutData4(devc,xOffset+70,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+70,yOffset,(*(patOffset-1))>>4,false);
					}
					break;
				case 6:
					if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
						(*(patOffset-4) <= 120 || *(patOffset-4) == 255 ))
					{
						OutData4(devc,xOffset+80,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+80,yOffset,(*(patOffset-1)),false);
					}
					break;
				case 7:
					if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
						(*(patOffset-4) <= 120 || *(patOffset-4) == 255 ))
					{
						OutData4(devc,xOffset+90,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+90,yOffset,(*(patOffset))>>4,false);
					}
					break;
				case 8:
					if (*(patOffset-1) == 0 && *(patOffset) == 0 && 
						(*(patOffset-4) <= 120 || *(patOffset-4) == 255 ))
					{
						OutData4(devc,xOffset+100,yOffset,0,true);
					}
					else
					{
						OutData4(devc,xOffset+100,yOffset,(*(patOffset)),false);
					}
					break;
				}
			}
			trackcount++;
			patOffset++;
			xOffset+=ROWWIDTH;
		}
		linecount++;
		yOffset+=ROWHEIGHT;
	}
}

/*

// ADVISE! [lOff+lstart..lOff+lend] and [tOff+tstart..tOff+tend] HAVE TO be valid!
void CChildView::DrawMultiPatternData(CDC *devc,int tstart,int tend, int lstart, int lend)
{
	int c_4beat = Global::pConfig->pvc_row4beat;
	int c_beat = Global::pConfig->pvc_rowbeat;
	int c_row = Global::pConfig->pvc_row;
	devc->SetTextColor(Global::pConfig->pvc_font);

	int editPattern , lines , linecount;
	int ordercount=editPosition-1;
	if ( lstart+lOff < 0 )
	{
		editPattern= _pSong->playOrder[ordercount];
		lines= _pSong->patternLines[editPattern];
		linecount = lines+lstart+lOff;
	}
	else
	{
		editPattern= _pSong->playOrder[++ordercount];
		lines= _pSong->patternLines[editPattern];
		if ( lstart+lOff >= lines )
		{
			linecount = lstart+lOff-lines;
			editPattern=_pSong->playOrder[++ordercount];
			lines= _pSong->patternLines[editPattern];
		}
		else linecount = lstart+lOff;
	}

	int yOffset=lstart*ROWHEIGHT+YOFFSET;
	char tBuf[32];

	for (int i=lstart;i<lend;i++) // Lines
	{
		if ( linecount >= lines )
		{
			editPattern=_pSong->playOrder[++ordercount];
			lines= _pSong->patternLines[editPattern];
			linecount=0;
		}
		if((linecount%_pSong->_ticksPerBeat) == 0)
		{
			if ((linecount%(_pSong->_ticksPerBeat*4)) == 0) devc->SetBkColor(c_4beat);
			else devc->SetBkColor(c_beat);
		}
		else devc->SetBkColor(c_row);

		if (XOFFSET!=1)
		{
			sprintf(tBuf,"%d",linecount);
			TXTFLAT(devc,tBuf,0,yOffset,XOFFSET-2,12);	// Print Line Number.
		}

		unsigned char *patOffset = _pSong->pPatternData +	(editPattern*MULTIPLY2) +
									(linecount*MULTIPLY) +	(tstart+tOff)*5;
		for (int t=tstart;t<tend;t++)
		{
			const int xOffset= XOFFSET+t*ROWWIDTH;

			OutNote(devc,xOffset,yOffset,*patOffset);
			if (*++patOffset == 255 )
			{
				OutData(devc,xOffset+28,yOffset,*patOffset,true);
			}else OutData(devc,xOffset+28,yOffset,*patOffset,false);

			if (*++patOffset == 255 )
			{
				OutData(devc,xOffset+49,yOffset,*patOffset,true);
			}else OutData(devc,xOffset+49,yOffset,*patOffset,false);

			if (*++patOffset == 0 && *(patOffset+1) == 0 && 
				(*(patOffset-3) <= 120 || *(patOffset-3) == 255 ))
			{
				OutData(devc,xOffset+70,yOffset,*patOffset,true);
				patOffset++;
				OutData(devc,xOffset+90,yOffset,*patOffset,true);
			}
			else
			{
				OutData(devc,xOffset+70,yOffset,*patOffset,false);
				patOffset++;
				OutData(devc,xOffset+90,yOffset,*patOffset,false);
			}
			
			patOffset++;
		}
		linecount++;
		yOffset+=ROWHEIGHT;
	}
}
*/

void CChildView::NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd)
{
	if (viewMode == VMPattern)
	{
		// inserts pattern data to be drawn into the list
		SPatternDraw* pNew = new SPatternDraw;
		pNew->pPrev = pPatternDraw;
		pPatternDraw = pNew;
		pNew->drawTrackStart = drawTrackStart;
		pNew->drawTrackEnd = drawTrackEnd;
		pNew->drawLineStart = drawLineStart;
		pNew->drawLineEnd = drawLineEnd;
	}
	else
	{
		while (pPatternDraw)
		{
			SPatternDraw* temp = pPatternDraw->pPrev;
			delete pPatternDraw;
			pPatternDraw = temp;
		}
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
		pDest[i] = (f2i(p0*0x10000)&0xff0000)
					| (f2i(p1*0x100)&0xff00)
					| (f2i(p2)&0xff);
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


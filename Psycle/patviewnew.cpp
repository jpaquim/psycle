/*
multiPattern : It needs MANY modifications to work properly. Basically in the
               "PreparePatternRefresh". Also the parts of the "DrawPatEditor" that
			   references it, needs some changes.

when appgetsfocus -> full update!

*/
#define DRAW_HEADER 1
#define DRAW_DATA	2
#define DRAW_SCROLL 4
#define DRAW_TRHEADER 8


void CChildView::PreparePatternRefresh(int drawMode)
{
	if ( updateMode == DMNone ) updatePar=0;
	updateMode=drawMode;
	CRect rect;

	if (drawMode == DMPatternHeader )
	{
		rect.top=0; rect.left=0;
		rect.bottom=18;	rect.right=CW;
		updatePar |= DRAW_HEADER;
		InvalidateRect(rect,false);
	}
	else if (drawMode == DMTrackHeader )
	{
		rect.top=18; rect.left=XOFFSET;
		rect.bottom=YOFFSET-1;	rect.right=XOFFSET+maxt*ROWWIDTH;
		updatePar |= DRAW_TRHEADER;
		InvalidateRect(rect,false);
	}
	else if (drawMode == DMSelection )//define or undefine the block.
	{
		if ( blockSelected )
		{
			if ((blockSel.end.track<tOff) || (blockSel.end.line<lOff) ||
				(blockSel.start.track>=tOff+VISTRACKS) ||
				(blockSel.start.line>=lOff+VISLINES))
			{
				newselpos.bottom = 0;
			}
			else 
			{
				if(blockSel.start.track<tOff ) newselpos.left=XOFFSET;
				else newselpos.left=XOFFSET+(blockSel.start.track-tOff)*ROWWIDTH;
				
				if(blockSel.start.line<=lOff ) newselpos.top=YOFFSET;
				else newselpos.top=YOFFSET+(blockSel.start.line-lOff)*ROWHEIGHT;
				
				if(blockSel.end.track>=tOff+VISTRACKS) newselpos.right=CW;
				else newselpos.right=XOFFSET+(blockSel.end.track-tOff+1)*ROWWIDTH;

				if(blockSel.end.line>=lOff+VISLINES ) newselpos.bottom=CH;
				else newselpos.bottom=YOFFSET+(blockSel.end.line-lOff+1)*ROWHEIGHT;
				
				InvalidateRect(newselpos,false);
			}
			if (selpos.bottom != 0 ) InvalidateRect(selpos,false);
		}
		else if ( selpos.bottom != 0)
		{
			newselpos.bottom=0;
			InvalidateRect(selpos,false);
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////
		// Set the offsets and positions of data on the screen.
		int tofs, lofs;
		const int snt = _pSong->SONGTRACKS;
		const int plines = _pSong->patternLines[_pSong->playOrder[editPosition]];

		if ( editcur.track >= snt )	{ editcur.track = snt-1; }
		if ( editcur.line >= plines ) { editcur.line = plines-1; }
		
		// Track Offset
		if ( snt <= VISTRACKS)	{ maxt = snt; tofs = 0; }
		else
		{
			if (Global::pConfig->_centerCursor)
			{
				if ( drawMode == DMScroll ) tofs = ntOff;
				else tofs = editcur.track - (VISTRACKS/2);

				if (tofs >= snt-VISTRACKS)	{	tofs = snt-VISTRACKS;	maxt = VISTRACKS;	}
				else 
				{
					if ( tofs < 0 ) { tofs = 0; }
					maxt = VISTRACKS+1;
				}
			}
			else
			{
				if ( drawMode == DMScroll )
				{
					tofs = ntOff;
					if ( tofs >= snt-VISTRACKS ) maxt = VISTRACKS;
					else maxt = VISTRACKS+1;
				}
				else
				{
					if ( tOff+VISTRACKS > snt )
					{
						tofs = snt-VISTRACKS;
						maxt=VISTRACKS;
					}
					else if ( editcur.track < tOff ) { tofs = editcur.track; maxt = VISTRACKS+1; }
					else
					{
						if ( editcur.track >= tOff+VISTRACKS ) tofs =editcur.track-VISTRACKS+1;
						else tofs = tOff;
					
						if ( tofs >= snt-VISTRACKS ) maxt = VISTRACKS;
						else maxt = VISTRACKS+1;
					}
				}
			}
		}
		// Line Offset
		if ( plines <=  VISLINES)	{ maxl = plines; lofs = 0; }
		else 
		{
			if (Global::pConfig->_centerCursor)
			{
				if ( drawMode == DMScroll ) lofs = nlOff;
				else lofs = editcur.line - (VISLINES/2);

				if (lofs >= plines-VISLINES) { lofs = plines-VISLINES; maxl = VISLINES; }
				else 
				{
					if ( lofs < 0 ) { lofs = 0; }
					maxl = VISLINES+1;
				}
			}
			else
			{
				if ( drawMode == DMScroll )
				{
					lofs = nlOff;
					if ( lofs >= plines-VISLINES ) maxl = VISLINES;
					else maxl = VISLINES+1;
				}
				else 
				{
					if ( lOff+VISLINES > plines )
					{
						lofs = plines-VISLINES;
						maxl=VISLINES;
					}
					else if ( editcur.line < lOff ) { lofs = editcur.line; maxl = VISLINES+1; }
					else 
					{
						if ( editcur.line >= lOff+VISLINES ) lofs =editcur.line-VISLINES+1;
						else lofs = lOff;
	
						if ( lofs >= plines-VISLINES ) maxl = VISLINES;
						else maxl = VISLINES+1;
					}
				}
			}
		}

		////////////////////////////////////////////////////////////////////
		// Determines if Scroll is needed or not.
		
		scrollT=0;scrollL=0;

		if (drawMode != DMAll && drawMode != DMPatternChange)
		{
			rect=guipos;
			if ( lofs != lOff )
			{
				rect.top=YOFFSET;	rect.left=0;
				rect.bottom=CH;		rect.right=XOFFSET+maxt*ROWWIDTH;
				scrollL= lOff-lofs;
				updatePar |= DRAW_SCROLL | DRAW_DATA;
			}
			if ( tofs != tOff )
			{
				rect.top=18;		rect.left=XOFFSET;
				rect.bottom=YOFFSET+ (maxl*ROWHEIGHT)+1;	rect.right=CW;
				scrollT= tOff-tofs;
				updatePar |= DRAW_SCROLL | DRAW_DATA;
			}
			InvalidateRect(rect,false);
		}

		////////////////////////////////////////////////////////////////////
		// Checks for specific code to update.
		
		if (drawMode == DMPlayback )
		{
			int pos = Global::pPlayer->_lineCounter-lofs;
			if (( pos >= 0 ) &&  ( pos <maxl ) &&
				(_pSong->playOrder[editPosition] == _pSong->playOrder[Global::pPlayer->_playPosition]))
			{
				newplaypos.top= YOFFSET+ pos*ROWHEIGHT;
				newplaypos.bottom=newplaypos.top+ROWHEIGHT;	// left never changes and is set at ChildView init.
				newplaypos.right=XOFFSET+(maxt)*ROWWIDTH;
				InvalidateRect(newplaypos,false);
			}
			else newplaypos.bottom=0;
			if ( playpos.bottom != 0 ) InvalidateRect(playpos,false);
		}
		else if ( playpos.bottom != 0 && !Global::pPlayer->_playing ) 
		{
			newplaypos.bottom=0;
			if ( playpos.bottom != 0 ) InvalidateRect(playpos,false);
		}

		if (drawMode == DMDataChange )
		{
			drawTrackStart-=tofs;
			if ( drawTrackStart < 0 ) drawTrackStart = 0;
			drawTrackEnd-=(tofs-1);
			if ( drawTrackEnd > maxt ) drawTrackEnd = maxt;

			drawLineStart-=lofs;
			if ( drawLineStart < 0 ) drawLineStart = 0;
			drawLineEnd-=(lofs-1);
			if ( drawLineEnd > maxl ) drawLineEnd = maxl;
			updatePar |= DRAW_DATA;
			
			rect.left=XOFFSET+ drawTrackStart*ROWWIDTH;
			rect.right=XOFFSET+drawTrackEnd*ROWWIDTH;
			rect.top=YOFFSET+drawLineStart*ROWHEIGHT;
			rect.bottom=YOFFSET+drawLineEnd*ROWHEIGHT;
			InvalidateRect(rect,false);
		}
		else if (drawMode == DMPatternChange ) // Need of Modification. Currently a copy of DMAll.
		{
			updatePar |= DRAW_DATA | DRAW_HEADER;
			scrollT = maxt;
			rect.left=0;	rect.top=0;
			rect.right=CW;	rect.bottom=CH;

			if ( snt > VISTRACKS )
			{	ShowScrollBar(SB_HORZ,TRUE);
				SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
			}
			
			else
			{	ShowScrollBar(SB_HORZ,FALSE); }

			if ( plines > VISLINES )
			{	ShowScrollBar(SB_VERT,TRUE);
				SetScrollRange(SB_VERT,0,plines-VISLINES);
			}

			else
			{	ShowScrollBar(SB_VERT,FALSE); }

			InvalidateRect(rect,false);
		}
		else if (drawMode == DMAll )
		{
			updatePar |= DRAW_DATA | DRAW_HEADER;
			scrollT = maxt;
			rect.left=0;	rect.top=0;
			rect.right=CW;	rect.bottom=CH;

			if ( snt > VISTRACKS )
			{	ShowScrollBar(SB_HORZ,TRUE);
				SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
			}
			
			else
			{	ShowScrollBar(SB_HORZ,FALSE); }

			if ( plines > VISLINES )
			{	ShowScrollBar(SB_VERT,TRUE);
				SetScrollRange(SB_VERT,0,plines-VISLINES);
			}

			else
			{	ShowScrollBar(SB_VERT,FALSE); }

			InvalidateRect(rect,false);
		}

		// Edit cursor.
		int x = XOFFSET+(editcur.track-tofs)*ROWWIDTH , w = 10;
		switch(editcur.col)
		{
			case 0: w+=17;break;
			case 1: x+=28;break;
			case 2: x+=38;break;
			case 3: x+=49;break;
			case 4: x+=59;break;
			case 5: x+=70;break;
			case 6: x+=80;break;
			case 7: x+=90;break;
			case 8: x+=100;break;
			default:break;
		}
		newguipos.left = x;		newguipos.top = YOFFSET+(editcur.line-lofs)*ROWHEIGHT;
		newguipos.right = x + w;	newguipos.bottom = newguipos.top + ROWHEIGHT;

		if ( newguipos.left < XOFFSET || newguipos.top < YOFFSET ) newguipos.bottom=0;
		else InvalidateRect(newguipos,false);

		
		// Selection block
		if (blockSelected)
		{
			//Update Selection block due to scroll or full repaint
			if ((blockSel.end.track<tofs) || (blockSel.end.line<lofs) ||
				(blockSel.start.track>=tofs+VISTRACKS) ||
				(blockSel.start.line>=lofs+VISLINES)) { newselpos.bottom = 0; }
			else 
			{
				if(blockSel.start.track<=tofs ) newselpos.left=XOFFSET;
				else newselpos.left=XOFFSET+(blockSel.start.track-tofs)*ROWWIDTH;
				
				if(blockSel.start.line<=lofs ) newselpos.top=YOFFSET;
				else newselpos.top=YOFFSET+(blockSel.start.line-lofs)*ROWHEIGHT;
				
				if(blockSel.end.track>=tofs+VISTRACKS ) newselpos.right=CW;
				else if (blockSel.end.track>=tofs+maxt) newselpos.right=XOFFSET+(maxt/*-1+1*/)*ROWWIDTH; 
				else newselpos.right=XOFFSET+(blockSel.end.track-tofs+1)*ROWWIDTH;

				if(blockSel.end.line>=lofs+VISLINES ) newselpos.bottom=CH;
				else if (blockSel.end.line>=lofs+maxl) newselpos.bottom=YOFFSET+(maxl/*-1+1*/)*ROWHEIGHT;
				else newselpos.bottom=YOFFSET+(blockSel.end.line-lofs+1)*ROWHEIGHT;

				//"else if's" are added to adapt the selection when changing the pattern and
				// being them of different sizes.
				
				InvalidateRect(newselpos,false);
			}
			if ( selpos.bottom != 0 ) InvalidateRect(selpos,false);
		}
		tOff = tofs; lOff = lofs;
		SetScrollPos(SB_HORZ,tOff);
		SetScrollPos(SB_VERT,lOff);
	}
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


void CChildView::DrawPatEditor(CDC *devc)
{
	///////////////////////////////////////////////////////////
	// Prepare pattern for update (Undraw guipos and playpos)
	int prevmode = devc->SetROP2(R2_NOT);
	if ( guipos.bottom != 0 ) BOX(devc,guipos);
	if ( playpos.bottom != 0 ) BOX(devc,playpos);
	if ( selpos.bottom != 0 ) BOX(devc,selpos);
	devc->SetROP2(prevmode);

	CBrush brush;
	CBrush* oldBrush;
	CFont* oldfont;
	brush.CreateSolidBrush(Global::pConfig->pvc_background); // This affects BOX background
	oldBrush= devc->SelectObject(&brush);
	devc->SetBkColor(Global::pConfig->pvc_background);	// This affects TXT background
	int editPattern = _pSong->playOrder[editPosition];

	////////////////////////////////////////////////////////////
	// Draw Header
	if ( updatePar & DRAW_HEADER )
	{
		char buffer[256];
		if(bEditMode)
			sprintf(buffer," Pattern Editor [%.2X: %s]   Octave: %d   EDIT MODE: On",editPattern,_pSong->patternName[editPattern],_pSong->currentOctave);
		else
			sprintf(buffer," Pattern Editor [%.2X: %s]   Octave: %d   EDIT MODE: Off",editPattern,_pSong->patternName[editPattern],_pSong->currentOctave);

		devc->SetTextColor(Global::pConfig->pvc_font);
		TXT(devc,buffer,0,0,CW,18);
	}

	////////////////////////////////////////////////////////////
	// Do Scroll if needed.
	if ( updatePar & DRAW_SCROLL )
	{
		const RECT patR = {XOFFSET,YOFFSET , CW, CH};
		const RECT trkR = {XOFFSET, 18, CW, YOFFSET-1};
		const RECT linR = {0, YOFFSET, XOFFSET, CH};

		CRect rect2;
		CRgn rgn;
		if (scrollT != 0 ) devc->ScrollDC(scrollT*ROWWIDTH,0,&trkR,&trkR,&rgn,&rect2);
		if (scrollL != 0 ) devc->ScrollDC(0,scrollL*ROWHEIGHT,&linR,&linR,&rgn,&rect2);
		devc->ScrollDC(scrollT*ROWWIDTH,scrollL*ROWHEIGHT,&patR,&patR,&rgn,&rect2);
	}
	/////////////////////////////////////////////////////////////
	// Update Mute/Solo Indicators. This is small duplication of code, but I think It's better
	if ( updatePar & DRAW_TRHEADER ) // to duplicate it avoiding complex solutions.
	{
		CDC memDC;
		CBitmap *oldbmp;
		memDC.CreateCompatibleDC(devc);
		oldbmp = memDC.SelectObject(&stuffbmp);
		int xOffset = XOFFSET-1;

		for(int i=tOff;i<tOff+maxt;i++)
		{
			BOX(devc, xOffset, 18, ROWWIDTH+1, YOFFSET-18); // Draw lines between tracks
			const int trackx0 = i/10;
			const int track0x = i%10;

			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			devc->BitBlt(xOffset+1, 19, 110, 16, &memDC, 148, 65, SRCCOPY);
			devc->BitBlt(xOffset+35-11, 21, 7, 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
			devc->BitBlt(xOffset+42-11, 21, 7, 12, &memDC, 148+track0x*7, 81, SRCCOPY);

			if (Global::_pSong->_trackMuted[i])
				devc->BitBlt(xOffset+71+5, 24, 7, 7, &memDC, 258, 49, SRCCOPY);

			if (Global::_pSong->_trackArmed[i])
				devc->BitBlt(xOffset+71-18, 24, 7, 7, &memDC, 276, 49, SRCCOPY);

			if (Global::_pSong->_trackSoloed == i )
				devc->BitBlt(xOffset+97, 24, 7, 7, &memDC, 267, 49, SRCCOPY);

			xOffset += ROWWIDTH;
		}
		memDC.SelectObject(oldbmp);
		memDC.DeleteDC();
	}

	////////////////////////////////////////////////////////////
	// Draw Pattern data.
	oldfont= devc->SelectObject(&seqFont);
	if ( updatePar & DRAW_DATA )
	{
		// Fill Bottom Space with Background colour if needed
		if ( maxl < VISLINES+1 )
		{
			CRect rect;
			rect.left=0; rect.right=CW; rect.bottom=CH;
			rect.top=YOFFSET+maxl*ROWHEIGHT+1; 
			devc->FillSolidRect(&rect,Global::pConfig->pvc_background);

		}
		// Fill Right Space with Background colour if needed
		if ( maxt < VISTRACKS+1)
		{
			CRect rect;
			rect.top=0; rect.bottom=CH;  rect.right=CW;
			rect.left=XOFFSET+ maxt*ROWWIDTH;
			devc->FillSolidRect(&rect,Global::pConfig->pvc_background);
		}

		////////////////////////////////////////////////
		// Draw Track Header, box and tracks. Also used when Full repaint.
		int ScrollTrackStart,ScrollTrackEnd, ScrollLineTrackStart,ScrollLineTrackEnd;
		if ( scrollT > 0 )
		{	ScrollTrackStart = 0; ScrollTrackEnd=scrollT;
			ScrollLineTrackStart =scrollT; ScrollLineTrackEnd=maxt;
			if ( ScrollTrackEnd > maxt ) ScrollTrackEnd=maxt;
		}
		else if ( scrollT < 0 )
		{	ScrollTrackStart=VISTRACKS+scrollT; ScrollTrackEnd=maxt;
			ScrollLineTrackStart=0; ScrollLineTrackEnd=VISTRACKS+scrollT-1;
			if ( ScrollTrackStart < 0 ) ScrollTrackStart = 0;
		}
		else { ScrollLineTrackStart=0; ScrollLineTrackEnd=maxt; }

		if ( scrollT )
		{
			CDC memDC;
			CBitmap *oldbmp;
			memDC.CreateCompatibleDC(devc);
			oldbmp = memDC.SelectObject(&stuffbmp);
			int xOffset = XOFFSET-1+ScrollTrackStart*ROWWIDTH;

			BOX(devc, -1, 18, XOFFSET, YOFFSET+maxl*ROWHEIGHT-17);	// Line Column
			TXT(devc, "Line", 0, 19, XOFFSET-2, 16);

			for(int i=tOff+ScrollTrackStart;i<tOff+ScrollTrackEnd;i++)
			{
				BOX(devc, xOffset, 18, ROWWIDTH+1, YOFFSET+maxl*ROWHEIGHT-17); // Draw background column
				const int trackx0 = i/10;
				const int track0x = i%10;

				// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
				devc->BitBlt(xOffset+1, 19, 110, 16, &memDC, 148, 65, SRCCOPY);
				devc->BitBlt(xOffset+35-11, 21, 7, 12, &memDC, 148+trackx0*7, 81, SRCCOPY);
				devc->BitBlt(xOffset+42-11, 21, 7, 12, &memDC, 148+track0x*7, 81, SRCCOPY);

				if (Global::_pSong->_trackMuted[i])
					devc->BitBlt(xOffset+71+5, 24, 7, 7, &memDC, 258, 49, SRCCOPY);

				if (Global::_pSong->_trackArmed[i])
					devc->BitBlt(xOffset+71-18, 24, 7, 7, &memDC, 276, 49, SRCCOPY);

				if (Global::_pSong->_trackSoloed == i )
					devc->BitBlt(xOffset+97, 24, 7, 7, &memDC, 267, 49, SRCCOPY);
				xOffset += ROWWIDTH;
			}
			BOX(devc,0,YOFFSET-1,XOFFSET+maxt*ROWWIDTH,1);
			memDC.SelectObject(oldbmp);
			memDC.DeleteDC();

			if ( multiPattern )
			{
				// Some other code in here
				DrawMultiPatternData(devc,ScrollTrackStart,ScrollTrackEnd,0,maxl);
			}
			else DrawSinglePatternData(devc,ScrollTrackStart,ScrollTrackEnd,0,maxl);
			
		}
		////////////////////////////////////////////////
		// Draw Lines in case of vertical Scroll.
		if ( scrollL )
		{
			int ScrollLineStart,ScrollLineEnd;
			if ( scrollL > 0 ) { ScrollLineStart = 0; ScrollLineEnd=scrollL; }
			else if ( scrollL < 0 ) { ScrollLineStart=VISLINES+scrollL; ScrollLineEnd=maxl; }

			if ( multiPattern )
			{
				// Some other code in here
				DrawMultiPatternData(devc,ScrollLineTrackStart,ScrollLineTrackEnd,ScrollLineStart,ScrollLineEnd);
			}
			else DrawSinglePatternData(devc,ScrollLineTrackStart,ScrollLineTrackEnd,ScrollLineStart,ScrollLineEnd);

		}
		////////////////////////////////////////////////
		// Draw Data Changed (DMDataChange)
		if (drawTrackStart != -1)
		{
			if ( multiPattern )
			{
				// Some other code in here
				DrawMultiPatternData(devc,drawTrackStart,drawTrackEnd,drawLineStart,drawLineEnd);
			}
			else DrawSinglePatternData(devc,drawTrackStart,drawTrackEnd,drawLineStart,drawLineEnd);
			drawTrackStart = -1;
		}
	}

	///////////////////////////////////////////////////////////
	// Draw Cursors
	prevmode = devc->SetROP2(R2_NOT);

	if (newplaypos.bottom != 0 ) BOX(devc,newplaypos);
	playpos=newplaypos;
	if (newselpos.bottom != 0 ) BOX(devc,newselpos);
	selpos=newselpos;
	if (newguipos.bottom != 0 ) BOX(devc,newguipos);
	guipos=newguipos;

	devc->SetROP2(prevmode);
	devc->SelectObject(oldBrush);
	devc->SelectObject(oldfont);
	brush.DeleteObject();

	updateMode = DMNone;
	updatePar = DRAW_DATA | DRAW_HEADER;
	scrollT = maxt;
}


// ADVISE! [lOff+lstart..lOff+lend] and [tOff+tstart..tOff+tend] HAVE TO be valid!
void CChildView::DrawSinglePatternData(CDC *devc,int tstart,int tend, int lstart, int lend)
{
	int yOffset=lstart*ROWHEIGHT+YOFFSET;
	int linecount=lOff+ lstart;
	char tBuf[16];

	int c_4beat = Global::pConfig->pvc_row4beat;
	int c_beat = Global::pConfig->pvc_rowbeat;
	int c_row = Global::pConfig->pvc_row;
	devc->SetTextColor(Global::pConfig->pvc_font);

	for (int i=lstart;i<lend;i++) // Lines
	{
		if((linecount%_pSong->_ticksPerBeat) == 0)
		{
			if ((linecount%(_pSong->_ticksPerBeat*4)) == 0) devc->SetBkColor(c_4beat);
			else devc->SetBkColor(c_beat);
		}
		else devc->SetBkColor(c_row);

		sprintf(tBuf,"%d",linecount);
		TXTFLAT(devc,tBuf,0,yOffset,XOFFSET-2,12);	// Print Line Number.

		unsigned char *patOffset = _pSong->pPatternData +
									_pSong->playOrder[editPosition]*MULTIPLY2 +
									(linecount*MULTIPLY) + 	(tstart+tOff)*5;
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

		sprintf(tBuf,"%d",linecount);
		TXTFLAT(devc,tBuf,0,yOffset,XOFFSET-2,12);	// Print Line Number.

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


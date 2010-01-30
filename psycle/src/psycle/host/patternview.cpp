// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include "patternview.hpp"

#include <psycle/helpers/math.hpp>
#include <psycle/helpers/hexstring_to_integer.hpp>

#include <psycle/core/machine.h>
#include <psycle/core/player.h>
#include <psycle/core/song.h>

#include "BlockTransposeCommand.hpp"
#include "ChangeGenCommand.hpp"
#include "ChangeInsCommand.hpp"
#include "ClearCurrCommand.hpp"
#include "Configuration.hpp"
#include "EnterDataCommand.hpp"
#include "DeleteBlockCommand.hpp"
#include "DeleteCurrCommand.hpp"
#include "InputHandler.hpp"
#include "InterpolateCurveDlg.hpp"
#include "InsertCurrCommand.hpp"
#include "MainFrm.hpp"
#include "PasteBlockCommand.hpp"
#include "PatDeleteCommand.hpp"
#include "PatDlg.hpp"
#include "PatPasteCommand.hpp"
#include "PatTransposeCommand.hpp"
#include "SwingFillDlg.hpp"
#include "TransformPatternDlg.hpp"

#ifdef _MSC_VER
#undef min
#undef max
#endif
#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { 
	namespace host {

		using namespace helpers;
		using namespace helpers::math;

		#define DRAW_DATA		1
		#define DRAW_HSCROLL	2
		#define DRAW_VSCROLL	4
		#define DRAW_TRHEADER	8
		#define DRAW_FULL_DATA	16

		///\todo extemely toxic pollution
		#define TWOPI_F (2.0f*3.141592665f)
	

		PatternView::PatternView(Project* project)						
			:  blockSelected(false),
			   blockStart(false),
			   blockswitch(false),
			   blockSelectBarState(1),
			   bScrollDetatch(false),
			   bEditMode(true),
			   patStep(1),
			   editPosition(0),
			   prevEditPosition(0),
			   ChordModeOffs(0),
			   updateMode(0),
			   updatePar(0),			  			   
			   textLeftEdge(2),
			   hbmPatHeader(0),
			   playpos(-1),
			   newplaypos(-1),
			   numPatternDraw(0),			   	
			   maxt(1),
			   maxl(1),
			   tOff(0),
			   lOff(0),
			   ntOff(0),
			   nlOff(0),
			   rntOff(0),
			   rnlOff(0),
			   isBlockCopied(false),
			   blockNTracks(0),
			   blockNLines(0),
			   patBufferLines(0),
			   patBufferCopy(false),
			   bFT2HomeBehaviour(true),
			   bShiftArrowsDoSelect(false),
			   bDoingSelection(false),
			   maxView(false),
			   project_(project),
			   pattern_(0) {
			selpos.bottom=0;
			newselpos.bottom=0;
			szBlankParam[0]='\0';
			szBlankNote[0]='\0';
			MBStart.x=0;
			MBStart.y=0;
		}
		
		Song* PatternView::song() { 
			return &main_->projects()->active_project()->song(); 
		}
		
		Project* PatternView::project() {
			return main_->projects()->active_project();
		}

		Project* PatternView::project() const {
			return main_->projects()->active_project();
		}

		void PatternView::Draw(CDC *devc, const CRgn& rgn) {
			DrawPatEditor(devc);
		}

		void PatternView::OnSize(UINT nType, int cx, int cy) {
			CW = cx;
			CH = cy;
			RecalcMetrics();
		}

		bool PatternView::CheckUnsavedSong() {
			return (project()->cmd_manager()->UndoSize() == 0);
		}

		void PatternView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			// undo code not required, enter note handles it
			CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
			if (cmd.GetType() == CT_Note)
			{
				const int outnote = cmd.GetNote();
				if(bEditMode && Global::pPlayer->playing() && Global::pConfig->_followSong && Global::pConfig->_RecordNoteoff)
				{ 
					EnterNote(outnote,0,true);	// note end
				}
				else
				{
					Global::pInputHandler->StopNote(outnote);
				}
			}
			else if ((nChar == 16) && ((nFlags & 0xC000) == 0xC000) && ChordModeOffs)
			{
				// shift is up, abort chord mode
				editcur.line = ChordModeLine;
				editcur.track = ChordModeTrack;
				ChordModeOffs = 0;
				AdvanceLine(patStep,Global::pConfig->_wrapAround,true);
		//		pParentMain->StatusBarIdle();
		//		Repaint(draw_modes::cursor);
			}

		}

		bool PatternView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			BOOL bRepeat = nFlags&0x4000;
				if (!(Global::pPlayer->playing() && Global::pConfig->_followSong && bRepeat))
				{
					CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);
					if ( editcur.col == 0 && cmd.GetType() != CT_Note)
						return false; else
					if ( !(::GetKeyState(VK_CONTROL)>=0 && ::GetKeyState(VK_SHIFT)>=0))
						return false;
					else if (editcur.col != 0) {
						if	(	nChar>='0'		&&	nChar<='9')			{ }
						else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ }
						else if(nChar>='A'		&&	nChar<='F')			{ }
						else											{ return false; }
					}
					// add data
					project()->cmd_manager()->ExecuteCommand(
						new EnterDataCommand(this, nChar, nFlags));
					return true;
				}
				return false;
		}

		void PatternView::EnterData(UINT nChar, UINT nFlags)
		{
			if (editcur.col == 0) {
				// get command
				CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar, nFlags);
				EnterNote(cmd.GetNote());
			} else {
				MSBPut(nChar);
			}
		}

		void PatternView::PreparePatternRefresh(int drawMode)
		{
		#ifdef _DEBUG_PATVIEW
			TRACE("PreparePatternRefresh\n");
		#endif
			Song* song = this->song();
			CRect rect;	
			updateMode=drawMode;					// this is ununsed for patterns		
			int beat_zoom = project()->beat_zoom();
			const int plines = pattern()->beats() * beat_zoom;
			const int snt = song->tracks();
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
					child_view()->InvalidateRect(rect,false);
				}
				if ( rntOff != tOff )
				{
					rect.top=0;		
					rect.left=XOFFSET;
					rect.bottom=CH;
					rect.right=CW;
					updatePar |= DRAW_HSCROLL;
					child_view()->InvalidateRect(rect,false);
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
				child_view()->InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					child_view()->ShowScrollBar(SB_HORZ,TRUE);
					child_view()->SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					child_view()->ShowScrollBar(SB_VERT,TRUE);
					child_view()->SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_VERT,FALSE); 
				}
				break;
			case draw_modes::pattern: 
				// all data
				rect.top=YOFFSET;		
				rect.left=0;
				rect.bottom=CH;
				rect.right=CW;
				updatePar |= DRAW_FULL_DATA;
				child_view()->InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					child_view()->ShowScrollBar(SB_HORZ,TRUE);
					child_view()->SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					child_view()->ShowScrollBar(SB_VERT,TRUE);
					child_view()->SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_VERT,FALSE); 
				}
				break;
			case draw_modes::playback:
			{
				psycle::core::Player & player(psycle::core::Player::singleton());
				int ticks = static_cast<int>(project()->beat_zoom());
				int pos = 0;
				psycle::core::SequenceEntry* entry = this->main()->m_wndSeq.selected_entry();
				pos = (player.playPos() - entry->tickPosition()) * ticks;
				if (( pos-rnlOff >= 0 ) &&  ( pos-rnlOff <maxl ) ) {
						if (pos != playpos)
						{
							newplaypos = pos;

							rect.top= YOFFSET+ ((pos-rnlOff)*ROWHEIGHT);
							rect.bottom=rect.top+ROWHEIGHT;	// left never changes and is set at ChildView init.
							rect.left = 0;
							rect.right=CW;
							NewPatternDraw(0, song->tracks(), pos, pos);
							updatePar |= DRAW_DATA;
							child_view()->InvalidateRect(rect,false);
							if ((playpos >= 0) && (playpos != newplaypos))
							{
								rect.top = YOFFSET+ ((playpos-rnlOff)*ROWHEIGHT);
								rect.bottom = rect.top+ROWHEIGHT;
								rect.left = 0;
								rect.right = CW;
								NewPatternDraw(0, song->tracks(), playpos, playpos);
								updatePar |= DRAW_DATA;
								playpos =-1;
								child_view()->InvalidateRect(rect,false);
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
							NewPatternDraw(0, song->tracks(), playpos, playpos);
							updatePar |= DRAW_DATA;
							playpos = -1;
							child_view()->InvalidateRect(rect,false);
						}
					}
				}
				break;
			case draw_modes::playback_change: 
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo
				{
					int a = 10;
				}
#else
				if (song->playOrder[editPosition] == song->playOrder[Global::pPlayer->_sequencePosition])
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
				child_view()->InvalidateRect(rect,false);
				if ( snt > VISTRACKS )
				{	
					child_view()->ShowScrollBar(SB_HORZ,TRUE);
					child_view()->SetScrollRange(SB_HORZ,0,snt-VISTRACKS);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_HORZ,FALSE); 
				}

				if ( plines > VISLINES )
				{	
					child_view()->ShowScrollBar(SB_VERT,TRUE);
					child_view()->SetScrollRange(SB_VERT,0,plines-VISLINES);
				}
				else
				{	
					child_view()->ShowScrollBar(SB_VERT,FALSE); 
				}
#endif
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
							child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
								child_view()->InvalidateRect(rect,false);
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
					child_view()->InvalidateRect(rect,false);
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
					child_view()->InvalidateRect(rect,false);
				}
				break;
			case draw_modes::track_header: 
				// header
				rect.top=0; 
				rect.left=XOFFSET;
				rect.bottom=YOFFSET-1;	
				rect.right=XOFFSET+maxt*ROWWIDTH;
				updatePar |= DRAW_TRHEADER;
				child_view()->InvalidateRect(rect,false);
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
				child_view()->InvalidateRect(rect,false);
				if (editcur.line != editlast.line)
				{
					if (XOFFSET!=1)
					{
						rect.left = 0;
						rect.right = XOFFSET;
						child_view()->InvalidateRect(rect,false);
					}
					rect.left = XOFFSET+(editlast.track-rntOff)*ROWWIDTH;
					rect.right = rect.left+ROWWIDTH;
					rect.top = YOFFSET+(editlast.line-rnlOff)*ROWHEIGHT;
					rect.bottom = rect.top+ROWWIDTH;
					NewPatternDraw(editlast.track, editlast.track, editlast.line, editlast.line);
					child_view()->InvalidateRect(rect,false);
					if (XOFFSET!=1)
					{
						rect.left = 0;
						rect.right = XOFFSET;
						child_view()->InvalidateRect(rect,false);
					}
				}
				else if (editcur.track != editlast.track)
				{
					rect.left = XOFFSET+(editlast.track-rntOff)*ROWWIDTH;
					rect.right = rect.left+ROWWIDTH;
					rect.top = YOFFSET+(editlast.line-rnlOff)*ROWHEIGHT;
					rect.bottom = rect.top+ROWWIDTH;
					NewPatternDraw(editlast.track, editlast.track, editlast.line, editlast.line);
					child_view()->InvalidateRect(rect,false);
				}
			}

			// turn off play line if not playing
			if (playpos >= 0 && !Global::pPlayer->playing()) 
			{
				newplaypos=-1;
				rect.top = YOFFSET+ (playpos-rnlOff)*ROWHEIGHT;
				rect.bottom = rect.top+ROWHEIGHT;
				rect.left = 0;
				rect.right = XOFFSET+(maxt)*ROWWIDTH;
				NewPatternDraw(0, song->tracks(), playpos, playpos);
				playpos =-1;
				updatePar |= DRAW_DATA;
				child_view()->InvalidateRect(rect,false);
			}

			////////////////////////////////////////////////////////////////////
			// Checks for specific code to update.

			child_view()->SetScrollPos(SB_HORZ,rntOff);
			child_view()->SetScrollPos(SB_VERT,rnlOff);
			//\todo: This line has been commented out because it is what is causing the breaks in sound when
			//  using follow song and the pattern changes. Commenting it out should only mean that the refresh
			//	is delayed for the next idle time, and not forced now, inside the timer, (which has a lock).
//			UpdateWindow();
		}


		void PatternView::TransparentBlt
			(
				CDC* pDC,
				int xStart,  int yStart,
				int wWidth,  int wHeight,
				CDC* pTmpDC,
				CBitmap* bmpMask,
				int xSource, // = 0
				int ySource // = 0
			)
		{
			// We are going to paint the two DDB's in sequence to the destination.
			// 1st the monochrome bitmap will be blitted using an AND operation to
			// cut a hole in the destination. The color image will then be ORed
			// with the destination, filling it into the hole, but leaving the
			// surrounding area untouched.
			CDC hdcMem;
			hdcMem.CreateCompatibleDC(pDC);
			CBitmap* hbmT = hdcMem.SelectObject(bmpMask);
			pDC->SetTextColor(RGB(0,0,0));
			pDC->SetBkColor(RGB(255,255,255));
			if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource, 
				SRCAND))
			{
				TRACE("Transparent Blit failure SRCAND");
			}
			// Also note the use of SRCPAINT rather than SRCCOPY.
			if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
				SRCPAINT))
			{
				TRACE("Transparent Blit failure SRCPAINT");
			}
			// Now, clean up.
			hdcMem.SelectObject(hbmT);
			hdcMem.DeleteDC();
		}

		#define DF_NONE			0
		#define	DF_SELECTION	1
		#define DF_PLAYBAR		2
		#define DF_CURSOR		4
		#define DF_DRAWN		15

		void PatternView::DrawPatEditor(CDC *devc)
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
						if (song()->sequence().trackMuted(i))
							TransparentBlt(devc,
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
								1+PatHeaderCoords.dMuteOn.y, 
								PatHeaderCoords.sMuteOn.width, 
								PatHeaderCoords.sMuteOn.height, 
								&memDC, 
								&patternheadermask,
								PatHeaderCoords.sMuteOn.x, 
								PatHeaderCoords.sMuteOn.y);

						if (song()->sequence().trackArmed(i))
							TransparentBlt(devc,
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
								1+PatHeaderCoords.dRecordOn.y, 
								PatHeaderCoords.sRecordOn.width, 
								PatHeaderCoords.sRecordOn.height, 
								&memDC, 
								&patternheadermask,
								PatHeaderCoords.sRecordOn.x, 
								PatHeaderCoords.sRecordOn.y);

						if (song()->_trackSoloed == i )
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
						if (song()->sequence().trackMuted(i))
							devc->BitBlt(
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
								1+PatHeaderCoords.dMuteOn.y, 
								PatHeaderCoords.sMuteOn.width, 
								PatHeaderCoords.sMuteOn.height, 
								&memDC, 
								PatHeaderCoords.sMuteOn.x, 
								PatHeaderCoords.sMuteOn.y, 
								SRCCOPY);

						if (song()->sequence().trackArmed(i))
							devc->BitBlt(
								xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
								1+PatHeaderCoords.dRecordOn.y, 
								PatHeaderCoords.sRecordOn.width, 
								PatHeaderCoords.sRecordOn.height, 
								&memDC, 
								PatHeaderCoords.sRecordOn.x, 
								PatHeaderCoords.sRecordOn.y, 
								SRCCOPY);

						if (song()->_trackSoloed == i )
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
									if (Global::song()->_trackMuted[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y);

									if (Global::song()->_trackArmed[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y);

									if (Global::song()->_trackSoloed == i )
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
									if (Global::song()->_trackMuted[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y, 
											SRCCOPY);

									if (Global::song()->_trackArmed[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y, 
											SRCCOPY);

									if (Global::song()->_trackSoloed == i )
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
									if (Global::song()->_trackMuted[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y);

									if (Global::song()->_trackArmed[i])
										TransparentBlt(devc,
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											&patternheadermask,
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y);

									if (Global::song()->_trackSoloed == i )
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
									if (Global::song()->_trackMuted[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
											1+PatHeaderCoords.dMuteOn.y, 
											PatHeaderCoords.sMuteOn.width, 
											PatHeaderCoords.sMuteOn.height, 
											&memDC, 
											PatHeaderCoords.sMuteOn.x, 
											PatHeaderCoords.sMuteOn.y, 
											SRCCOPY);

									if (Global::song()->_trackArmed[i])
										devc->BitBlt(
											xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
											1+PatHeaderCoords.dRecordOn.y, 
											PatHeaderCoords.sRecordOn.width, 
											PatHeaderCoords.sRecordOn.height, 
											&memDC, 
											PatHeaderCoords.sRecordOn.x, 
											PatHeaderCoords.sRecordOn.y, 
											SRCCOPY);

									if (Global::song()->_trackSoloed == i )
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
										if (song()->sequence().trackMuted(i))
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y);

										if (song()->sequence().trackArmed(i))
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y);

										if (song()->_trackSoloed == i )
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
										if (song()->sequence().trackMuted(i))
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y, 
												SRCCOPY);

										if (song()->sequence().trackArmed(i))
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y, 
												SRCCOPY);

										if (song()->_trackSoloed == i )
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
										if (song()->sequence().trackMuted(i))
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y);

										if (song()->sequence().trackArmed(i))
											TransparentBlt(devc,
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												&patternheadermask,
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y);

										if (song()->_trackSoloed == i )
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
										if (song()->sequence().trackMuted(i))
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dMuteOn.x, 
												1+PatHeaderCoords.dMuteOn.y, 
												PatHeaderCoords.sMuteOn.width, 
												PatHeaderCoords.sMuteOn.height, 
												&memDC, 
												PatHeaderCoords.sMuteOn.x, 
												PatHeaderCoords.sMuteOn.y, 
												SRCCOPY);

										if (song()->sequence().trackArmed(i))
											devc->BitBlt(
												xOffset+1+HEADER_INDENT+PatHeaderCoords.dRecordOn.x, 
												1+PatHeaderCoords.dRecordOn.y, 
												PatHeaderCoords.sRecordOn.width, 
												PatHeaderCoords.sRecordOn.height, 
												&memDC, 
												PatHeaderCoords.sRecordOn.x, 
												PatHeaderCoords.sRecordOn.y, 
												SRCCOPY);

										if (song()->_trackSoloed == i )
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
		void PatternView::DrawPatternData(CDC *devc,int tstart,int tend, int lstart, int lend)
		{
			if ( (lstart > VISLINES && lstart > maxl) || tstart > maxt || tend < 0)
				return;
			lstart = std::max(lstart,0);
			lend = std::min(std::max(lend,0), maxl);
			tstart = std::min(std::max(0, tstart), maxt);
			tend = std::min(std::max(0, tend), maxt);

#if PSYCLE__CONFIGURATION__USE_PSYCORE
			//todo: Use editPositionEntry when it is ready.
			Pattern* patt = pattern();
			
			double beat_zoom = static_cast<int>(project()->beat_zoom());		
			char tBuf[16];

			int top = (lstart)*ROWHEIGHT+YOFFSET;
			int height = (lend-lstart)*ROWHEIGHT;
			COLORREF* pBkg = pvc_row;

			for ( int l = lstart; l < lend; ++l) {
				// break this up into several more general loops for speed
				int linecount = l + lOff;
				int top = l*ROWHEIGHT+YOFFSET;
				if((linecount%(int)(beat_zoom)) == 0) {
					if ((linecount%(int)(beat_zoom*Global::pConfig->pv_timesig)) == 0) {
						pBkg = pvc_row4beat;
					} else {
						pBkg = pvc_rowbeat;
					}
				} else {
					pBkg = pvc_row;
				}
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

				if (XOFFSET != -1) {
					int yOffset= l*ROWHEIGHT + YOFFSET;
					if (Global::pConfig->_linenumbersHex) {
						sprintf(tBuf," %.2X", linecount);
						TXTFLAT(devc, tBuf, 1, yOffset, XOFFSET-2, ROWHEIGHT-1);	// Print Line Number.
					} else {
						sprintf(tBuf, "%3i", linecount);
						TXTFLAT(devc, tBuf, 1, yOffset, XOFFSET-2, ROWHEIGHT-1);	// Print Line Number.
					}
				}
				int yOffset= YOFFSET+(l*ROWHEIGHT);
				for ( int t = tstart; t < tend; ++t ) {
					int trackcount = t + tOff;
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

					int xOffset= XOFFSET+(t*ROWWIDTH);					
					OutNote(devc,xOffset+COLX[0],yOffset,255);
					OutData(devc,xOffset+COLX[1],yOffset,0,true);
					OutData(devc,xOffset+COLX[3],yOffset,0,true);
					OutData(devc,xOffset+COLX[5],yOffset,0,true);
					OutData(devc,xOffset+COLX[7],yOffset,0,true);
					//
					// could optimize this check some, make separate loops
					if ((linecount == editcur.line) && (trackcount == editcur.track))
					{
						devc->SetBkColor(pvc_cursor[trackcount]);
						devc->SetTextColor(pvc_fontCur[trackcount]);
						switch (editcur.col) {
						case 0:
							OutNote(devc,xOffset+COLX[0],yOffset,255);
							break;
						case 1:					
							OutData4(devc,xOffset+COLX[1],yOffset,0,true);							
							break;
						case 2:					
							OutData4(devc,xOffset+COLX[2],yOffset,0,true);							
							break;
						case 3:
							OutData4(devc,xOffset+COLX[3],yOffset,0,true);
							break;
						case 4:
							OutData4(devc,xOffset+COLX[4],yOffset,0,true);
							break;
						case 5:
							OutData4(devc,xOffset+COLX[5],yOffset,0,true);
							break;
						case 6:
							OutData4(devc,xOffset+COLX[6],yOffset,0,true);
							break;
						case 7:
							OutData4(devc,xOffset+COLX[7],yOffset,0,true);
							break;
						case 8:
							OutData4(devc,xOffset+COLX[8],yOffset,0,true);
							break;
						}
					}
				}
			}


			Pattern::iterator it = patt->lower_bound(line_pos(lstart+lOff, true)); 
			for ( ; it != patt->end(); ++it )  {						
				psycle::core::PatternEvent& ev = it->second;
				if ( ev.track() < tstart+tOff || ev.track() > tend+tOff)
					continue;
				int trackcount = ev.track() + tOff;
				double pos = it->first;
				int line = static_cast<int>(pos * beat_zoom) - lOff;
				int linecount = line + lOff;
				if((linecount%(int)(beat_zoom)) == 0) {
				if ((linecount%(int)(beat_zoom*Global::pConfig->pv_timesig)) == 0) {
						pBkg = pvc_row4beat;
					} else {
						pBkg = pvc_rowbeat;
					}
				} else {
					pBkg = pvc_row;
				}
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

				int yOffset=(line)*ROWHEIGHT+YOFFSET;
				int xOffset= XOFFSET+((ev.track()-tOff)*ROWWIDTH);

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

				OutNote(devc,xOffset+COLX[0],yOffset, ev.note());
				if (ev.instrument() == 255) {
					OutData(devc,xOffset+COLX[1],yOffset,0,true);
				} else {
					OutData(devc,xOffset+COLX[1],yOffset,ev.instrument(),false);
				}
				if (ev.machine() == 255) {
					OutData(devc,xOffset+COLX[3],yOffset,0,true);
				} else  {
					OutData(devc,xOffset+COLX[3],yOffset,ev.machine(),false);
				}

				if ((ev.command()) == 0 && (ev.parameter()) == 0 && 
						((ev.note()) <= notecommands::release || (ev.note()) == 255 )) {
					OutData(devc,xOffset+COLX[5],yOffset,0,true);
					OutData(devc,xOffset+COLX[7],yOffset,0,true);
				} else {
					OutData(devc,xOffset+COLX[5],yOffset,ev.command(),false);
					OutData(devc,xOffset+COLX[7],yOffset,ev.parameter(),false);
				}
				// could optimize this check some, make separate loops
				if ((linecount == editcur.line) && (trackcount == editcur.track))
				{
					devc->SetBkColor(pvc_cursor[trackcount]);
					devc->SetTextColor(pvc_fontCur[trackcount]);
					switch (editcur.col) {
						case 0:
							OutNote(devc,xOffset+COLX[0],yOffset,ev.note());
							break;
						case 1:
							if (ev.instrument() == 255) {
								OutData4(devc,xOffset+COLX[1],yOffset,0,true);
							} else{
								OutData4(devc,xOffset+COLX[1],yOffset,ev.instrument()>>4,false);
							}
							break;
						case 2:
							if (ev.instrument() == 255 ) {
								OutData4(devc,xOffset+COLX[2],yOffset,0,true);
							} else {
								OutData4(devc,xOffset+COLX[2],yOffset,ev.instrument(),false);
							}
							break;
						case 3:
							if (ev.machine() == 255 ) {
								OutData4(devc,xOffset+COLX[3],yOffset,0,true);
							}
							else {
								OutData4(devc,xOffset+COLX[3],yOffset,ev.machine()>>4,false);
							}
							break;
						case 4:
							if (ev.machine() == 255 ) {
								OutData4(devc,xOffset+COLX[4],yOffset,0,true);
							} else {
								OutData4(devc,xOffset+COLX[4],yOffset,ev.machine(),false);
							}
							break;
						case 5:
							if (ev.command() == 0 && ev.parameter() == 0 && 
								(ev.note() <= notecommands::release || ev.note() == 255 ))
							{
								OutData4(devc,xOffset+COLX[5],yOffset,0,true);
							} else {
								OutData4(devc,xOffset+COLX[5],yOffset,ev.command()>>4,false);
							}
							break;
						case 6:
							if (ev.command() == 0 && ev.parameter() == 0 && 
								(ev.note() <= notecommands::release || ev.note() == 255 ))
							{
								OutData4(devc,xOffset+COLX[6],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[6],yOffset,ev.command(),false);
							}
							break;
						case 7:
							if (ev.command() == 0 && ev.parameter() == 0 && 
								(ev.note() <= notecommands::release || ev.note() == 255 ))
							{
								OutData4(devc,xOffset+COLX[7],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[7],yOffset,ev.parameter()>>4,false);
							}
							break;
						case 8:
							if (ev.command() == 0 && ev.parameter() == 0 && 
								(ev.note() <= notecommands::release || ev.note() == 255 ))
							{
								OutData4(devc,xOffset+COLX[8],yOffset,0,true);
							}
							else
							{
								OutData4(devc,xOffset+COLX[8],yOffset,ev.parameter(),false);
							}
							break;
						}
					}
			}
	

#else
		#ifdef _DEBUG_PATVIEW
			TRACE("DrawPatternData\n");
		#endif


			int yOffset=lstart*ROWHEIGHT+YOFFSET;
			int linecount=lOff+ lstart;
			char tBuf[16];

			COLORREF* pBkg;
			for (int i=lstart;i<lend;i++) // Lines
			{
				// break this up into several more general loops for speed
				if((linecount%song()->LinesPerBeat()) == 0)
				{
					if ((linecount%(song()->LinesPerBeat()*Global::pConfig->pv_timesig)) == 0) 
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
#endif
		}

		void PatternView::NewPatternDraw(int drawTrackStart, int drawTrackEnd, int drawLineStart, int drawLineEnd)
		{
			if (child_view()->viewMode == view_modes::pattern)
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

		void PatternView::LoadPatternHeaderSkin()
		{
			std::string szOld;
			if (!Global::pConfig->pattern_header_skin.empty())
			{
				szOld = Global::pConfig->pattern_header_skin;
				// ok so...
				if (szOld != std::string(PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN))
				{
					BOOL result = FALSE;
					FindPatternHeaderSkin(Global::pConfig->GetSkinDir().c_str(),Global::pConfig->pattern_header_skin.c_str(), &result);
					if (result)
					{
						return;
					}
				}
				// load defaults
				szOld = PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN;
				// and coords
			#if defined PSYCLE__PATH__CONFIGURATION__SKIN__UGLY_DEFAULT
				PatHeaderCoords.sBackground.x=0;
				PatHeaderCoords.sBackground.y=0;
				PatHeaderCoords.sBackground.width=109;
				PatHeaderCoords.sBackground.height=16;
				PatHeaderCoords.sNumber0.x = 0;
				PatHeaderCoords.sNumber0.y = 16;
				PatHeaderCoords.sNumber0.width = 7;
				PatHeaderCoords.sNumber0.height = 12;
				PatHeaderCoords.sRecordOn.x = 70;
				PatHeaderCoords.sRecordOn.y = 16;
				PatHeaderCoords.sRecordOn.width = 7;
				PatHeaderCoords.sRecordOn.height = 7;
				PatHeaderCoords.sMuteOn.x = 77;
				PatHeaderCoords.sMuteOn.y = 16;
				PatHeaderCoords.sMuteOn.width = 7;
				PatHeaderCoords.sMuteOn.height = 7;
				PatHeaderCoords.sSoloOn.x = 84;
				PatHeaderCoords.sSoloOn.y = 16;
				PatHeaderCoords.sSoloOn.width = 7;
				PatHeaderCoords.sSoloOn.height = 7;
				PatHeaderCoords.dDigitX0.x = 23;
				PatHeaderCoords.dDigitX0.y = 2;
				PatHeaderCoords.dDigit0X.x = 30;
				PatHeaderCoords.dDigit0X.y = 2;
				PatHeaderCoords.dRecordOn.x = 52;
				PatHeaderCoords.dRecordOn.y = 5;
				PatHeaderCoords.dMuteOn.x = 75;
				PatHeaderCoords.dMuteOn.y = 5;
				PatHeaderCoords.dSoloOn.x = 96;
				PatHeaderCoords.dSoloOn.y = 5;
				PatHeaderCoords.bHasTransparency = false;
			#else
				PatHeaderCoords.sBackground.x=0;
				PatHeaderCoords.sBackground.y=0;
				PatHeaderCoords.sBackground.width=109;
				PatHeaderCoords.sBackground.height=18;//16
				PatHeaderCoords.sNumber0.x = 0;
				PatHeaderCoords.sNumber0.y = 18;//16
				PatHeaderCoords.sNumber0.width = 7;
				PatHeaderCoords.sNumber0.height = 12;
				PatHeaderCoords.sRecordOn.x = 70;
				PatHeaderCoords.sRecordOn.y = 18;//16
				PatHeaderCoords.sRecordOn.width = 11;//7;
				PatHeaderCoords.sRecordOn.height = 11;//7;
				PatHeaderCoords.sMuteOn.x = 81;//77;
				PatHeaderCoords.sMuteOn.y = 18;//16;
				PatHeaderCoords.sMuteOn.width = 11;//7;
				PatHeaderCoords.sMuteOn.height = 11;//7;
				PatHeaderCoords.sSoloOn.x = 92;//84;
				PatHeaderCoords.sSoloOn.y = 18;//16;
				PatHeaderCoords.sSoloOn.width = 11;//7;
				PatHeaderCoords.sSoloOn.height = 11;//7;
				PatHeaderCoords.dDigitX0.x = 24;//22;
				PatHeaderCoords.dDigitX0.y = 3;//2;
				PatHeaderCoords.dDigit0X.x = 31;//29;
				PatHeaderCoords.dDigit0X.y = 3;//2;
				PatHeaderCoords.dRecordOn.x = 52;
				PatHeaderCoords.dRecordOn.y = 3;//5;
				PatHeaderCoords.dMuteOn.x = 75;
				PatHeaderCoords.dMuteOn.y = 3;//5;
				PatHeaderCoords.dSoloOn.x = 97;//96;
				PatHeaderCoords.dSoloOn.y = 3;//5;
				PatHeaderCoords.bHasTransparency = false;
			#endif
				patternheader.DeleteObject();
				DeleteObject(hbmPatHeader);
				patternheadermask.DeleteObject();
				patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
			}
		}

		void PatternView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*");	// check for subfolders.
			while (loop) 
			{		
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					FindPatternHeaderSkin(finder.GetFilePath(),findName,result);
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\" + findName + ".psh"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psh, does it have a valid matching .bmp?
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					char szOpenName[MAX_PATH];
					std::sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);
					patternheader.DeleteObject();
					if (hbmPatHeader)DeleteObject(hbmPatHeader);
					patternheadermask.DeleteObject();
					hbmPatHeader = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hbmPatHeader)
					{
						if (patternheader.Attach(hbmPatHeader))
						{	
							memset(&PatHeaderCoords,0,sizeof(PatHeaderCoords));
							// load settings
							FILE* hfile;
							sprintf(szOpenName,"%s\\%s.psh",findDir,sName);
							if(!(hfile=fopen(szOpenName,"rb")))
							{
								child_view()->MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
								return;
							}
							char buf[512];
							while (fgets(buf, 512, hfile))
							{
								if (strstr(buf,"\"background_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sBackground.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sBackground.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sBackground.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sBackground.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"number_0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sNumber0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sNumber0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sNumber0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sNumber0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"record_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sRecordOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sRecordOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sRecordOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sRecordOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"mute_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sMuteOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sMuteOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sMuteOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sMuteOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"solo_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sSoloOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sSoloOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sSoloOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sSoloOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"digit_x0_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dDigitX0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dDigitX0.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"digit_0x_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dDigit0X.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dDigit0X.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"record_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dRecordOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dRecordOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"mute_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dMuteOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dMuteOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"solo_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dSoloOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dSoloOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"transparency\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										hexstring_to_integer(q+1, PatHeaderCoords.cTransparency);
										PatHeaderCoords.bHasTransparency = TRUE;
									}
								}
							}
							if (PatHeaderCoords.bHasTransparency)
							{
								PrepareMask(&patternheader,&patternheadermask,PatHeaderCoords.cTransparency);
							}
							fclose(hfile);
							*result = TRUE;
							break;
						}
					}
				}
			}
			finder.Close();
		}

		void PatternView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
		{
			BITMAP bm;
			// Get the dimensions of the source bitmap
			pBmpSource->GetObject(sizeof(BITMAP), &bm);
			// Create the mask bitmap
			pBmpMask->DeleteObject();
			pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
			// We will need two DCs to work with. One to hold the Image
			// (the source), and one to hold the mask (destination).
			// When blitting onto a monochrome bitmap from a color, pixels
			// in the source color bitmap that are equal to the background
			// color are blitted as white. All the remaining pixels are
			// blitted as black.
			CDC hdcSrc, hdcDst;
			hdcSrc.CreateCompatibleDC(NULL);
			hdcDst.CreateCompatibleDC(NULL);
			// Load the bitmaps into memory DC
			CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
			CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
			// Change the background to trans color
			hdcSrc.SetBkColor(clrTrans);
			// This call sets up the mask bitmap.
			hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
			// Now, we need to paint onto the original image, making
			// sure that the "transparent" area is set to black. What
			// we do is AND the monochrome image onto the color Image
			// first. When blitting from mono to color, the monochrome
			// pixel is first transformed as follows:
			// if  1 (black) it is mapped to the color set by SetTextColor().
			// if  0 (white) is is mapped to the color set by SetBkColor().
			// Only then is the raster operation performed.
			hdcSrc.SetTextColor(RGB(255,255,255));
			hdcSrc.SetBkColor(RGB(0,0,0));
			hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
			// Clean up by deselecting any objects, and delete the
			// DC's.
			hdcSrc.SelectObject(hbmSrcT);
			hdcDst.SelectObject(hbmDstT);
			hdcSrc.DeleteDC();
			hdcDst.DeleteDC();
		}

		void PatternView::RecalcMetrics()
		{
			if (Global::pConfig->pattern_draw_empty_data)
			{
				strcpy(szBlankParam,".");
				strcpy(szBlankNote,"---");
			}
			else
			{
				strcpy(szBlankParam," ");
				strcpy(szBlankNote,"   ");
			}
			TEXTHEIGHT = Global::pConfig->pattern_font_y;
			ROWHEIGHT = TEXTHEIGHT+1;
			TEXTWIDTH = Global::pConfig->pattern_font_x;
			for (int c=0; c<256; c++)	
			{ 
				FLATSIZES[c]=Global::pConfig->pattern_font_x; 
			}
			COLX[0] = 0;
			COLX[1] = (TEXTWIDTH*3)+2;
			COLX[2] = COLX[1]+TEXTWIDTH;
			COLX[3] = COLX[2]+TEXTWIDTH+1;
			COLX[4] = COLX[3]+TEXTWIDTH;
			COLX[5] = COLX[4]+TEXTWIDTH+1;
			COLX[6] = COLX[5]+TEXTWIDTH;
			COLX[7] = COLX[6]+TEXTWIDTH;
			COLX[8] = COLX[7]+TEXTWIDTH;
			COLX[9] = COLX[8]+TEXTWIDTH+1;
			ROWWIDTH = COLX[9];
			HEADER_ROWWIDTH = PatHeaderCoords.sBackground.width+1;
			HEADER_HEIGHT = PatHeaderCoords.sBackground.height+2;
			if (ROWWIDTH < HEADER_ROWWIDTH)
			{
				int temp = (HEADER_ROWWIDTH-ROWWIDTH)/2;
				ROWWIDTH = HEADER_ROWWIDTH;
				for (int i = 0; i < 10; i++)
				{
					COLX[i] += temp;
				}
			}
			HEADER_INDENT = (ROWWIDTH - HEADER_ROWWIDTH)/2;
			if (Global::pConfig->_linenumbers)
			{
				XOFFSET = (4*TEXTWIDTH);
				YOFFSET = TEXTHEIGHT+2;
				if (YOFFSET < HEADER_HEIGHT)
				{
					YOFFSET = HEADER_HEIGHT;
				}
			}
			else
			{
				XOFFSET = 1;
				YOFFSET = HEADER_HEIGHT;
			}
			VISTRACKS = (CW-XOFFSET)/ROWWIDTH;
			VISLINES = (CH-YOFFSET)/ROWHEIGHT;
			if (VISLINES < 1) 
			{ 
				VISLINES = 1; 
			}
			if (VISTRACKS < 1) 
			{ 
				VISTRACKS = 1; 
			}
		}

		void PatternView::RecalculateColour(COLORREF* pDest, COLORREF source1, COLORREF source2)
		{
			// makes an array of colours between source1 and source2
			float p0 = float((source1 >> 16) & 0xff);
			float p1 = float((source1 >>  8) & 0xff);
			float p2 = float( source1        & 0xff);

			float d0 = float((source2 >> 16) & 0xff);
			float d1 = float((source2 >> 8 ) & 0xff);
			float d2 = float( source2        & 0xff);

			int len = song()->tracks() + 1;

			float a0 = (d0 - p0) / len;
			float a1 = (d1 - p1) / len;
			float a2 = (d2 - p2) / len;

			for (int i = 0; i < len; i++)
			{
				pDest[i] =
					(lround<int32_t>(p0 * 0x10000) & 0xff0000) |
					(lround<int32_t>(p1 * 0x00100) & 0x00ff00) |
					(lround<int32_t>(p2          ) & 0x0000ff);
				p0 += a0;
				p1 += a1;
				p2 += a2;

				if(p0 < 0) p0 = 0;
				else if(p0 > 255) p0 = 255;

				if(p1 < 0) p1 = 0;
				else if(p1 > 255) p1 = 255;

				if(p2 < 0) p2 = 2;
				else if(p2 > 255) p2 = 255;
			}
		}

		COLORREF PatternView::ColourDiffAdd(COLORREF base, COLORREF adjust, COLORREF add)
		{
			int a0 = ((add>>16)&0x0ff)+((adjust>>16)&0x0ff)-((base>>16)&0x0ff);
			int a1 = ((add>>8 )&0x0ff)+((adjust>>8 )&0x0ff)-((base>>8 )&0x0ff);
			int a2 = ((add    )&0x0ff)+((adjust    )&0x0ff)-((base    )&0x0ff);

			if(a0 < 0) a0 = 0;
			else if(a0 > 255) a0 = 255;

			if(a1 < 0) a1 = 0;
			else if(a1 > 255) a1 = 255;

			if(a2 < 0) a2 = 0;
			else if (a2 > 255) a2 = 255;

			COLORREF pa = (a0<<16) | (a1<<8) | (a2);
			return pa;
		}

		void PatternView::RecalculateColourGrid()
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
		
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// MidiPatternNote
		//
		// DESCRIPTION	  : Called by the MIDI input interface to insert pattern notes
		// PARAMETERS     : int outnote - note to insert . int velocity - velocity of the note
		// RETURNS		  : <void>
		// 

		//
		// Mark!!!!! Please, check if the following function is ok. I have some doubts about the
		// NoteOff. And check if "if(outnote >= 0 && outnote <= 120)" is necessary.
		//

		void PatternView::MidiPatternNote(int outnote, int velocity)
		{
			// undo code not required, enter note handles it
		/*	if(outnote >= 0 && outnote <= 120)  // I really believe this is not necessary.
			{									// outnote <= 120 is checked before calling this function
												// and outnote CAN NOT be negative since it's taken from
												//	(dwParam1 & 0xFF00) >>8;
			*/
				if(bEditMode)
				{ 
					// add note
					if(velocity > 0 && outnote != notecommands::release)
					{
						EnterNote(outnote,velocity,false);
					}
					else
					{
						if(Global::pConfig->_RecordNoteoff && Global::pPlayer->playing() && Global::pConfig->_followSong)
						{
							EnterNote(outnote,0,false);	// note end
						}
						else
						{
							Global::pInputHandler->StopNote(outnote,false);	// note end
						}
					}			
				}
				else 
				{
					// play note
					if(velocity>0)
						Global::pInputHandler->PlayNote(outnote,velocity,false);
					else
						Global::pInputHandler->StopNote(outnote,false);
				}
		//	}
		}

		void PatternView::MidiPatternTweak(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

			// build entry
			PatternEvent entry;
			entry.setMachine(song()->seqBus);
			entry.setCommand((value>>8)&255);
			entry.setParameter(value&255);
			entry.setInstrument(command);
			entry.setNote(notecommands::tweak);

			if(bEditMode)
			{ 
				// write effect				
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset; 

				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{		
						// play it
						Machine* pMachine = song()->machine(song()->seqBus);

						// play
						if (pMachine)
						{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
							pMachine->Tick(editcur.track,entry);
#else
							pMachine->Tick(editcur.track,&entry);
#endif
						}
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *pentry = (PatternEvent*) toffset;
				if (pentry->note() >= notecommands::release)
				{
					if ((pentry->machine() != entry.machine()) 
						|| (pentry->command() != entry.command())
						|| (pentry->parameter() != entry.parameter()) 
						|| (pentry->instrument() != entry.instrument()) 
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						|| ((pentry->note() != notecommands::tweak) && (pentry->note() != notecommands::tweak_slide)))
#else
						|| ((pentry->note() != notecommands::tweak) && (pentry->note() != notecommands::tweakeffect) && (pentry->note() != notecommands::tweakslide)))
#endif
					{
						pentry->setMachine(entry.machine());
						pentry->setCommand(entry.command());
						pentry->setParameter(entry.parameter());
						pentry->setInstrument(entry.instrument());
						pentry->setNote(entry.note());

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				// play it
				Machine* pMachine = song()->machine(song()->seqBus);

				if (pMachine)
				{
					// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					pMachine->Tick(editcur.track,entry);
#else
					pMachine->Tick(editcur.track,&entry);
#endif
				}
			}
		}

		void PatternView::MidiPatternTweakSlide(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

			// build entry
			PatternEvent entry;
			entry.setMachine(song()->seqBus);
			entry.setCommand((value>>8)&255);
			entry.setParameter(value&255);
			entry.setInstrument(command);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			entry.setNote(notecommands::tweak_slide);
#else
			entry.setNote(notecommands::tweakslide);
#endif

			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset; 

				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{		
						Machine* pMachine = song()->machine(song()->seqBus);

						if (pMachine)
						{
							// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
							pMachine->Tick(editcur.track,entry);
#else
							pMachine->Tick(editcur.track,&entry);
#endif
						}
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *pentry = (PatternEvent*) toffset;
				if (pentry->note() >= notecommands::release)
				{
					if ((pentry->machine() != entry.machine()) 
						|| (pentry->command() != entry.command())
						|| (pentry->parameter() != entry.parameter()) 
						|| (pentry->instrument() != entry.instrument()) 
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						|| ((pentry->note() != notecommands::tweak) && (pentry->note() != notecommands::tweak_slide)))
#else
						|| ((pentry->note() != notecommands::tweak) && (pentry->note() != notecommands::tweakeffect) && (pentry->note() != notecommands::tweakslide)))
#endif
					{
						pentry->setMachine(entry.machine());
						pentry->setCommand(entry.command());
						pentry->setParameter(entry.parameter());
						pentry->setInstrument(entry.instrument());
						pentry->setNote(entry.note());

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				Machine* pMachine = song()->machine(song()->seqBus);

				if (pMachine)
				{
					// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					pMachine->Tick(editcur.track,entry);
#else
					pMachine->Tick(editcur.track,&entry);
#endif
				}
			}
		}

		void PatternView::MidiPatternCommand(int command, int value)
		{
			// UNDO CODE MIDI PATTERN
			if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xff) value = 0xff; // no else incase of neg overflow

			// build entry
			PatternEvent entry;
			entry.setMachine(song()->seqBus);
			entry.setInstrument(song()->auxcolSelected);
			entry.setCommand(command);
			entry.setParameter(value);
			entry.setNote(notecommands::empty);

			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset; 

				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{		
						Machine* pMachine = song()->machine(song()->seqBus);

						if (pMachine)
						{
							// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
							pMachine->Tick(editcur.track,entry);
#else
							pMachine->Tick(editcur.track,&entry);
#endif
						}
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *pentry = (PatternEvent*) toffset;
				if ((pentry->machine() != entry.machine()) 
					|| (pentry->instrument() != entry.instrument()) 
					|| (pentry->command() != entry.command()) 
					|| (pentry->parameter() != entry.parameter()))
				{
					pentry->setMachine(entry.machine());
					pentry->setCommand(entry.command());
					pentry->setParameter(entry.parameter());
					pentry->setInstrument(entry.instrument());

					NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
					Repaint(draw_modes::data);
				}
			}
		//	else
			{
				// play it
				Machine* pMachine = song()->machine(song()->seqBus);

				if (pMachine)
				{
					// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					pMachine->Tick(editcur.track,entry);
#else
					pMachine->Tick(editcur.track,&entry);
#endif
				}
			}
		}

		void PatternView::MidiPatternMidiCommand(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			PatternEvent entry;
			entry.setMachine(song()->seqBus);
			entry.setCommand((value&0xFF00)>>8);
			entry.setParameter(value&0xFF);
			entry.setInstrument(command);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			entry.setNote(notecommands::midi_cc);
#else
			entry.setNote(notecommands::midicc);
#endif

			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset; 

				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{		
						Machine* pMachine = song()->machine(song()->seqBus);

						if (pMachine)
						{
							// play
							pMachine->Tick(editcur.track,entry);
						}
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *pentry = (PatternEvent*) toffset;
				if (pentry->note() >= notecommands::release)
				{
					if ((pentry->machine() != entry.machine()) 
						|| (pentry->command() != entry.command()) 
						|| (pentry->parameter() != entry.parameter()) 
						|| (pentry->instrument() != entry.instrument()) 
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						|| (pentry->note() != notecommands::midi_cc))
#else
						|| (pentry->note() != notecommands::midicc))
#endif
					{
						pentry->setMachine(entry.machine());
						pentry->setCommand(entry.command());
						pentry->setParameter(entry.parameter());
						pentry->setInstrument(entry.instrument());
						pentry->setNote(entry.note());

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				Machine* pMachine = song()->machine(song()->seqBus);

				if (pMachine)
				{
					// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					pMachine->Tick(editcur.track,entry);
#else
					pMachine->Tick(editcur.track,&entry);
#endif
				}
			}
		}

		void PatternView::MidiPatternInstrument(int value)
		{
			// UNDO CODE MIDI PATTERN
			if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xff) value = 0xff; // no else incase of neg overflow

			// build entry
			PatternEvent entry;
			entry.setMachine(song()->seqBus);
			entry.setInstrument(value);
			entry.setCommand(255);
			entry.setParameter(255);
			entry.setNote(notecommands::empty);

			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset; 

				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{		
						Machine* pMachine = song()->machine(song()->seqBus);

						if (pMachine)
						{
							// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
							pMachine->Tick(editcur.track,entry);
#else
							pMachine->Tick(editcur.track,&entry);
#endif
						}
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *pentry = (PatternEvent*) toffset;
				if ((pentry->machine() != entry.machine()) 
					|| (pentry->instrument() != entry.instrument()))
				{
					pentry->setMachine(entry.machine());
					pentry->setInstrument(entry.instrument());

					NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
					Repaint(draw_modes::data);
				}
			}
		//	else
			{
				Machine* pMachine = song()->machine(song()->seqBus);

				if (pMachine)
				{
					// play
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					pMachine->Tick(editcur.track,entry);
#else
					pMachine->Tick(editcur.track,&entry);
#endif
				}
			}
		}

		void PatternView::MousePatternTweak(int machine, int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value < 0) value = 0x8000-value;
			// no else incase of neg overflow
			if (value > 0xffff) value = 0xffff;

			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				//todo: Add line positioning code				
				psycle::core::Player& player(psycle::core::Player::singleton());
				int ticks = static_cast<int>(project()->beat_zoom());				
				psycle::core::SequenceEntry* entry = this->main()->m_wndSeq.selected_entry();
				int line = (player.playPos() - entry->tickPosition()) * ticks;			

				if (player.playing() && Global::pConfig->_followSong) {
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{	
						return;
					}
				} else {
					line = editcur.line;
				}
				// build entry
				PatternEvent ev;
				if (ev.note() >= notecommands::release) {
					if ((ev.machine() != machine) || 
						(ev.command() != ((value>>8)&255)) || 
						(ev.parameter() != (value&255)) || 
						(ev.instrument() != command) || 
						((ev.note() != notecommands::tweak) && (ev.note() != notecommands::tweak_slide))) {
						ev.setMachine(machine);
						ev.setCommand((value>>8)&255);
						ev.setParameter(value&255);
						ev.setInstrument(command);
						ev.setNote(notecommands::tweak);
						ev.set_track(editcur.track);				
						pattern()->insert(line_pos(line,true), ev);
						NewPatternDraw(editcur.track, editcur.track, editcur.line, editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		}

		void PatternView::MousePatternTweakSlide(int machine, int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
			if(child_view()->viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo: Add line positioning code
				int line = 0;
#else
				int line = Global::pPlayer->_lineCounter;
#endif
				unsigned char * toffset;
				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{
					if(song()->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::pConfig->_RecordUnarmed)
					{	
						return;
					}
				}
				else
				{
					line = editcur.line;
				}

				// build entry
				PatternEvent *entry = (PatternEvent*) toffset;
				if (entry->note() >= notecommands::release)
				{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					if ((entry->machine() != machine) || (entry->command() != ((value>>8)&255)) || (entry->parameter() != (value&255)) || (entry->instrument() != command) || ((entry->note() != notecommands::tweak) && (entry->note() != notecommands::tweak_slide)))
#else
					if ((entry->machine() != machine) || (entry->command() != ((value>>8)&255)) || (entry->parameter() != (value&255)) || (entry->instrument() != command) || ((entry->note() != notecommands::tweak) && (entry->note() != notecommands::tweakeffect) && (entry->note() != notecommands::tweakslide)))
#endif
					{
						entry->setMachine(machine);
						entry->setCommand((value>>8)&255);
						entry->setParameter(value&255);
						entry->setInstrument(command);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						entry->setNote(notecommands::tweak_slide);
#else
						entry->setNote(notecommands::tweakslide);
#endif

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		}

#if PSYCLE__CONFIGURATION__USE_PSYCORE

		psycle::core::Pattern::iterator PatternView::GetEventOnPos(double insert_pos, int track) {
			double beat_zoom = project()->beat_zoom();
			int line = static_cast<int>(project()->beat_zoom() * insert_pos);
			psycle::core::Pattern::iterator it;
			double low = (line - 0.5) / beat_zoom;
			double up  = (line + 0.5) / beat_zoom;

			it = pattern()->lower_bound(low);

			// no entry on the beatpos
			if (it == pattern()->end() || 
				!(it->first >= low && it->first < up)
				) {
				return pattern()->end();
			} else {
			// this is implicit: if (it->first >= low && it->first < up)
				psycle::core::Pattern::iterator track_it = it;
				bool found = false;
				for ( ; it != pattern()->end() && it->first < up; ++it ) {
					psycle::core::PatternEvent& ev = it->second;
					if (ev.track() == track ) {
						return it;
					}
				}
				return pattern()->end();
			} 
		}

		psycle::core::Pattern::iterator PatternView::GetEventOnCursor() {
			return GetEventOnPos(editcur.line / static_cast<double>(project()->beat_zoom()), editcur.track);
		}
#endif


		void PatternView::EnterNote(int note, int velocity, bool bTranspose)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			int currentOctave = 4;
			if (note < 0 || note >= notecommands::invalid )
				return;

			// octave offset
			if(note<notecommands::release) {
				if(bTranspose)
					note += currentOctave*12;
				note = std::min(note, 119);
			}

			psycle::core::Pattern::iterator it = GetEventOnCursor();

			int line = editcur.line;			
			double insert_pos = editcur.line / static_cast<double>(project()->beat_zoom());
					
			if (it == pattern()->end()) {
				// no entry on the beatpos
				psycle::core::PatternEvent ev;
				ev.setNote(note);
				ev.set_track(editcur.track);
				ev.setMachine(song()->seqBus);
				pattern()->insert(insert_pos, ev);
			} else {
				it->second.setNote(note);
			}

			NewPatternDraw(editcur.track,editcur.track,line,line);
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);

			// realtime note entering
/*			if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
			{
				if(song()->_trackArmedCount)
				{
					if (velocity == 0)
					{
						int i;
						for (i = 0; i < song()->tracks(); i++)
						{
							if (song()->_trackArmed[i])
							{
								if (Global::pInputHandler->notetrack[i] == note)
								{
									editcur.track = i;
									break;
								}
							}
						}
						///\todo : errm.. == or !=  ????
						if (i == song()->tracks())
						{
							Global::pInputHandler->StopNote(note,false);
							return;
						}
					}
					else
					{
						SelectNextTrack();
					}
				}
				else if (!Global::pConfig->_RecordUnarmed)
				{
					// build entry
					PatternEvent entry;
					entry.note() = note;
					entry.machine() = song()->seqBus;

					if ( note < notecommands::release)
					{
						if (Global::pConfig->_RecordTweaks)
						{
							if (Global::pConfig->midi().raw())
							{
								entry.command() = 0x0c;
								entry.parameter() = velocity*2;
							}
							else if (Global::pConfig->midi().velocity().record())
							{
								// command
								entry.command() = Global::pConfig->midi().velocity().command();
								int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
								if (par > 255) 
								{
									par = 255;
								}
								else if (par < 0) 
								{
									par = 0;
								}
								entry.parameter() = par;
							}
						}
					}

					if (note>notecommands::release)
					{
						entry.instrument() = song()->auxcolSelected;
					}

					Machine *tmac = song()->machine(entry.machine());
					// implement lock sample to machine here.
					// if the current machine is a sampler, check 
					// if current sample is locked to a machine.
					// if so, switch entry.machine() to that machine number

					if (tmac)
					{
						if (((Machine*)song()->machine(song()->seqBus))->_type == MACH_SAMPLER)
						{
							if ((song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index != -1)
								&& (song()->_pInstrument[song()->auxcolSelected]->_locked_to_machine == true))
							{
								entry.machine() = song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index;
								tmac = song()->machine(entry.machine());
								if (!tmac) return;
							}
						}
						if (tmac->_type == MACH_SAMPLER || tmac->_type == MACH_XMSAMPLER)
						{
							entry.instrument() = song()->auxcolSelected;
						}
						else if (tmac->_type == MACH_VST) // entry->instrument() is the MIDI channel for VSTi's
						{
							entry.instrument() = song()->auxcolSelected;
						}
						
						if ( note < notecommands::release)
						{
							tmac->Tick(editcur.track, &entry);
						}
					}
					Global::pInputHandler->notetrack[editcur.track]=note;
					return;
				}
				line = Global::pPlayer->_lineCounter;
				toffset = _ptrack(ps)+(line*MULTIPLY);
				ChordModeOffs = 0;
			}
			else 
			{
				if ((GetKeyState(VK_SHIFT)<0) && (note != notecommands::tweak) && (note != notecommands::tweakeffect) && (note != notecommands::tweakslide) && (note != notecommands::midicc))
				{
					if (ChordModeOffs == 0)
					{
						ChordModeLine = editcur.line;
						ChordModeTrack = editcur.track;
					}
					editcur.track = (ChordModeTrack+ChordModeOffs)%song()->tracks();
					editcur.line = line = ChordModeLine;
					toffset = _ptrackline(ps, editcur.track, line);
					ChordModeOffs++;
				}
				else
				{
					if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
					{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
						editcur.line = ChordModeLine;
						editcur.track = ChordModeTrack;
						ChordModeOffs = 0;
						AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
					}
					line = editcur.line;
					toffset = _ptrackline(ps);
				}
			}

			// build entry
			PatternEvent *entry = (PatternEvent*) toffset;
			if (velocity==0)
			{
				Global::pInputHandler->StopNote(note,false);
				if (entry->note() == note)
				{
					return;
				}
				note = notecommands::release;
			}
			AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
			entry->setNote(note);
			entry->setMachine(song()->seqBus);

			if ( song()->seqBus < MAX_MACHINES && song()->machine(song()->seqBus) != 0 ) 
			{

					// implement lock sample to machine here.
					// if the current machine is a sampler, check 
					// if current sample is locked to a machine.
					// if so, switch entry.machine() to that machine number
					if (((Machine*)song()->machine(song()->seqBus))->_type == MACH_SAMPLER)
					{
						if ((song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index != -1)
							&& (song()->_pInstrument[song()->auxcolSelected]->_locked_to_machine == true))
						{
							entry->setMachine(song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index);
						}
					}
			}
			if ( note < notecommands::release)
			{
				if (Global::pConfig->_RecordTweaks)
				{
					if (Global::pConfig->midi().raw())
					{
						entry->setCommand(0x0c);
						entry->setParameter(velocity * 2);
					}
					else if (Global::pConfig->midi().velocity().record())
					{
						// command
						entry->setCommand(Global::pConfig->midi().velocity().command());
						int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
						if (par > 255) 
						{
							par = 255;
						}
						else if (par < 0) 
						{
							par = 0;
						}
						entry->setParameter(par);
					}
				}
			}

			if (note>notecommands::release)
			{
				entry->setInstrument(song()->auxcolSelected);
			}

			//Machine *tmac = song()->machine(song()->seqBus);
//altered for locking sample to machine by alk
			Machine *tmac = song()->machine(entry->machine());
			if (tmac)
			{
				if (tmac->_type == MACH_SAMPLER || tmac->_type == MACH_XMSAMPLER)
				{
					entry->setInstrument(song()->auxcolSelected);
				}
				else if (tmac->_type == MACH_VST) // entry->instrument() is the MIDI channel for VSTi's
				{
					entry->setInstrument(song()->auxcolSelected);
				}
				
				if ( note < notecommands::release)
				{
					tmac->Tick(editcur.track, entry);
				}
			}

			Global::pInputHandler->notetrack[editcur.track]=note;
			NewPatternDraw(editcur.track,editcur.track,line,line);
			if (!(Global::pPlayer->playing()&&Global::pConfig->_followSong))
			{
				if (ChordModeOffs)
				{
					AdvanceLine(-1,Global::pConfig->_wrapAround,false); //Advance track?
				}
				else
				{
					AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
				}
			}

*/
#else
			int line;

			// UNDO CODE ENTER NOTE
			const int ps = _ps();
			unsigned char * toffset;
			
			if (note < 0 || note >= notecommands::invalid ) return;

			// octave offset
			if(note<notecommands::release)
			{
				if(bTranspose)
					note+=song()->currentOctave*12;

				if (note > 119) 
					note = 119;
			}
			
			// realtime note entering
			if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
			{
				if(song()->_trackArmedCount)
				{
					if (velocity == 0)
					{
						int i;
						for (i = 0; i < song()->tracks(); i++)
						{
							if (song()->_trackArmed[i])
							{
								if (Global::pInputHandler->notetrack[i] == note)
								{
									editcur.track = i;
									break;
								}
							}
						}
						///\todo : errm.. == or !=  ????
						if (i == song()->tracks())
						{
							Global::pInputHandler->StopNote(note,false);
							return;
						}
					}
					else
					{
						SelectNextTrack();
					}
				}
				else if (!Global::pConfig->_RecordUnarmed)
				{
					// build entry
					PatternEvent entry;
					entry.setNote(note);
					entry.setMachine(song()->seqBus);

					if ( note < notecommands::release)
					{
						if (Global::pConfig->_RecordTweaks)
						{
							if (Global::pConfig->midi().raw())
							{
								entry.setCommand(0x0c);
								entry.setParameter(velocity*2);
							}
							else if (Global::pConfig->midi().velocity().record())
							{
								// command
								entry.setCommand(Global::pConfig->midi().velocity().command());
								int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
								if (par > 255) 
								{
									par = 255;
								}
								else if (par < 0) 
								{
									par = 0;
								}
								entry.setParameter(par);
							}
						}
					}

					if (note>notecommands::release)
					{
						entry.setInstrument(song()->auxcolSelected);
					}

					Machine *tmac = song()->machine(entry.machine());
					// implement lock sample to machine here.
					// if the current machine is a sampler, check 
					// if current sample is locked to a machine.
					// if so, switch entry.machine() to that machine number

					if (tmac)
					{
						if (((Machine*)song()->machine(song()->seqBus))->_type == MACH_SAMPLER)
						{
							if ((song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index != -1)
								&& (song()->_pInstrument[song()->auxcolSelected]->_locked_to_machine == true))
							{
								entry.setMachine(song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index);
								tmac = song()->machine(entry.machine());
								if (!tmac) return;
							}
						}
						if (tmac->_type == MACH_SAMPLER || tmac->_type == MACH_XMSAMPLER)
						{
							entry.setInstrument(song()->auxcolSelected);
						}
						else if (tmac->_type == MACH_VST) // entry->instrument() is the MIDI channel for VSTi's
						{
							entry.setInstrument(song()->auxcolSelected);
						}
						
						if ( note < notecommands::release)
						{
							tmac->Tick(editcur.track, &entry);
						}
					}
					Global::pInputHandler->notetrack[editcur.track]=note;
					return;
				}
				line = Global::pPlayer->_lineCounter;
				toffset = _ptrack(ps)+(line*MULTIPLY);
				ChordModeOffs = 0;
			}
			else 
			{
				if ((GetKeyState(VK_SHIFT)<0) && (note != notecommands::tweak) && (note != notecommands::tweakeffect) && (note != notecommands::tweakslide) && (note != notecommands::midicc))
				{
					if (ChordModeOffs == 0)
					{
						ChordModeLine = editcur.line;
						ChordModeTrack = editcur.track;
					}
					editcur.track = (ChordModeTrack+ChordModeOffs)%song()->tracks();
					editcur.line = line = ChordModeLine;
					toffset = _ptrackline(ps, editcur.track, line);
					ChordModeOffs++;
				}
				else
				{
					if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
					{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
						editcur.line = ChordModeLine;
						editcur.track = ChordModeTrack;
						ChordModeOffs = 0;
						AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
					}
					line = editcur.line;
					toffset = _ptrackline(ps);
				}
			}

			// build entry
			PatternEvent *entry = (PatternEvent*) toffset;
			if (velocity==0)
			{
				Global::pInputHandler->StopNote(note,false);
				if (entry->note() == note)
				{
					return;
				}
				note = notecommands::release;
			}
			AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
			entry->setNote(note);
			entry->setMachine(song()->seqBus);

			if ( song()->seqBus < MAX_MACHINES && song()->machine(song()->seqBus) != 0 ) 
			{

					// implement lock sample to machine here.
					// if the current machine is a sampler, check 
					// if current sample is locked to a machine.
					// if so, switch entry.machine() to that machine number
					if (((Machine*)song()->machine(song()->seqBus))->_type == MACH_SAMPLER)
					{
						if ((song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index != -1)
							&& (song()->_pInstrument[song()->auxcolSelected]->_locked_to_machine == true))
						{
							entry->setMachine(song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index);
						}
					}
			}
			if ( note < notecommands::release)
			{
				if (Global::pConfig->_RecordTweaks)
				{
					if (Global::pConfig->midi().raw())
					{
						entry->setCommand(0x0c);
						entry->setParameter(velocity * 2);
					}
					else if (Global::pConfig->midi().velocity().record())
					{
						// command
						entry->setCommand(Global::pConfig->midi().velocity().command());
						int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
						if (par > 255) 
						{
							par = 255;
						}
						else if (par < 0) 
						{
							par = 0;
						}
						entry->setParameter(par);
					}
				}
			}

			if (note>notecommands::release)
			{
				entry->setInstrument(song()->auxcolSelected);
			}

			//Machine *tmac = song()->machine(song()->seqBus);
//altered for locking sample to machine by alk
			Machine *tmac = song()->machine(entry->machine());
			if (tmac)
			{
				if (tmac->_type == MACH_SAMPLER || tmac->_type == MACH_XMSAMPLER)
				{
					entry->setInstrument(song()->auxcolSelected);
				}
				else if (tmac->_type == MACH_VST) // entry->instrument() is the MIDI channel for VSTi's
				{
					entry->setInstrument(song()->auxcolSelected);
				}
				
				if ( note < notecommands::release)
				{
					tmac->Tick(editcur.track, entry);
				}
			}

			Global::pInputHandler->notetrack[editcur.track]=note;
			NewPatternDraw(editcur.track,editcur.track,line,line);
			if (!(Global::pPlayer->playing()&&Global::pConfig->_followSong))
			{
				if (ChordModeOffs)
				{
					AdvanceLine(-1,Global::pConfig->_wrapAround,false); //Advance track?
				}
				else
				{
					AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
				}
			}

#endif
			bScrollDetatch=false;
			Global::pInputHandler->bDoingSelection = false;
			Repaint(draw_modes::data);
		}

		void PatternView::EnterNoteoffAny()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#else
			if (child_view()->viewMode == view_modes::pattern)
			{			
				// realtime note entering
				if (Global::pPlayer->playing()&&Global::pConfig->_followSong)
				{

					//todo: Add line positioning code
					// toffset = _ptrack(ps)+(0*MULTIPLY);
					toffset = _ptrack(ps)+(Global::pPlayer->_lineCounter*MULTIPLY);
				}
				else
				{
					// toffset = _ptrackline(ps);
				}

				// build entry
				// PatternEvent *entry = (PatternEvent*) toffset; 
				entry->setNote(notecommands::release);

				Global::pInputHandler->notetrack[editcur.track]=notecommands::release;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

				if (!(Global::pPlayer->playing()&&Global::pConfig->_followSong))
				{
					AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
				}

				bScrollDetatch=false;
				Global::pInputHandler->bDoingSelection = false;
				Repaint(draw_modes::data);
			}
#endif
		}

		bool PatternView::MSBPut(int nChar)
		{
			int sValue = -1;
			if	(	nChar>='0'		&&	nChar<='9')			{ sValue = nChar - '0'; }
			else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ sValue = nChar - VK_NUMPAD0; }
			else if(nChar>='A'		&&	nChar<='F')			{ sValue = nChar - 'A' + 10; }
			else											{ return false; }

			int line = editcur.line;
			psycle::core::Pattern::iterator it = GetEventOnCursor();
			double insert_pos = editcur.line / static_cast<double>(project()->beat_zoom());
			
			if (it == pattern()->end()) {
				// no entry on the beatpos
				psycle::core::PatternEvent ev;
				int old_value = 0;
				int new_value = 0;
				switch ((editcur.col+1)%2) {
					case 0:	
					new_value = (old_value&0xF)+(sValue<<4); 
					break;		
					case 1:	
					new_value = (old_value&0xF0)+(sValue); 
					break;
				}
				if ( editcur.col == 1 || editcur.col == 2) {
					ev.setInstrument(new_value);
				} else
				if ( editcur.col == 3 || editcur.col == 4) {
					ev.setMachine(new_value);
				} else
				if ( editcur.col == 5 || editcur.col == 6) {
					ev.setCommand(new_value);
				} else
				if ( editcur.col == 7 || editcur.col == 8) {
					ev.setParameter(new_value);
				}
				ev.set_track(editcur.track);				
				pattern()->insert(insert_pos, ev);
			} else {
				psycle::core::PatternEvent& ev = it->second;
				int old_value = 0;
				int new_value = 0;
				if ( editcur.col == 1 || editcur.col == 2) {
					old_value = ev.instrument();
				} else
				if ( editcur.col == 3 || editcur.col == 4) {
					old_value = ev.machine();
				} else
				if ( editcur.col == 5 || editcur.col == 6) {
					old_value = ev.command();
				} else
				if ( editcur.col == 7 || editcur.col == 8) {
					old_value = ev.parameter();
				}
				switch ((editcur.col+1)%2) {
						case 0:	
							new_value = (old_value&0xF)+(sValue<<4); 
						break;		
						case 1:	
							new_value = (old_value&0xF0)+(sValue); 
						break;
					}
				if ( editcur.col == 1 || editcur.col == 2) {
					ev.setInstrument(new_value);
				} else
				if ( editcur.col == 3 || editcur.col == 4) {
					ev.setMachine(new_value);
				} else
				if ( editcur.col == 5 || editcur.col == 6) {
					ev.setCommand(new_value);
				} else
				if ( editcur.col == 7 || editcur.col == 8) {
					ev.setParameter(new_value);
				}
				ev.set_track(editcur.track);				
			}

			if (Global::pConfig->_cursorAlwaysDown) {
				AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			}
			else
			{
				switch (editcur.col)
				{
				case 0:
					AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
					break;
				case 1:
				case 3:
				case 5:
				case 6:
				case 7:
					NextCol(false,false);
					break;
				case 8:
					PrevCol(false,false);
					PrevCol(false,false);
				case 2:
				case 4:
					PrevCol(false,false);
					AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
					break;
				}
			}
			bScrollDetatch=false;
			Global::pInputHandler->bDoingSelection = false;
			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(draw_modes::data);
			return true;
		}

		void PatternView::ClearCurr() { // delete content at Cursor pos.
			psycle::core::Pattern* pat = pattern();
			double beat_zoom = project()->beat_zoom();
			psycle::core::Pattern::iterator it;
			double low = (editcur.line - 0.5) / beat_zoom;
			double up  = (editcur.line + 0.5) / beat_zoom;
			double insert_pos = editcur.line / beat_zoom;
			it = pat->lower_bound(low);			
			int track = editcur.track;
			if (it != pat->end())	{
				psycle::core::Pattern::iterator track_it = it;				
				for ( ; it != pat->end() && it->first < up; ++it ) {
					psycle::core::PatternEvent& ev = it->second;
					if (ev.track() == editcur.track ) {
						if ( editcur.col == 0)
							it = pat->erase(it);
						else {
							switch (editcur.col) {
								case 1: ev.setInstrument(255);
								break;
								case 2: ev.setInstrument(255);
								break;
								case 3: ev.setMachine(255);
								break;
								case 4: ev.setMachine(255);
								break;
								case 5: ev.setCommand(0);
								break;
								case 6: ev.setCommand(0);
								break;
								case 7: ev.setParameter(0);
								break;
								case 8: ev.setParameter(0);
								break;
							}
						}
						break;
					}
				}
			} 
			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}

		void PatternView::DeleteCurr() {
			psycle::core::Pattern* pat = pattern();
			double beat_zoom = project()->beat_zoom();
			int patlines = static_cast<int>(beat_zoom * pat->beats());
			if ( Global::pInputHandler->bFT2DelBehaviour )
			{
				if(editcur.line==0)
					return;
				else
					editcur.line--;
			}
			psycle::core::Pattern::iterator it;
			double low = (editcur.line - 0.5) / beat_zoom;
			double up  = (editcur.line + 0.5) / beat_zoom;
			double insert_pos = editcur.line / beat_zoom;
			it = pat->lower_bound(low);
			
			int track = editcur.track;
			if (it != pat->end() )	{
				psycle::core::Pattern::iterator track_it = it;				
				for ( ; it != pat->end() && it->first < up; ++it ) {
					psycle::core::PatternEvent& ev = it->second;
					if (ev.track() == editcur.track ) {						
						it = pat->erase(it);
						break;
					}
				}
				for ( ; it != pat->end(); ++it) {
					psycle::core::PatternEvent& ev = it->second;
					if (ev.track() == editcur.track ) {
						psycle::core::PatternEvent old_event = it->second;
						double old_pos = it->first;
						it = pat->erase(it);
						it = pat->insert(old_pos - 1 / beat_zoom, old_event);
					}
				}
			} 
			NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);
			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}

		void PatternView::InsertCurr() {
			psycle::core::Pattern* pat = pattern();
			double beat_zoom = project()->beat_zoom();
			int patlines = static_cast<int>(beat_zoom * pat->beats());				
			psycle::core::Pattern::iterator it;
			double low = (editcur.line - 0.5) / beat_zoom;
			double up  = (editcur.line + 0.5) / beat_zoom;
			double insert_pos = editcur.line / beat_zoom;
			
			psycle::core::Pattern::reverse_iterator rit = pat->rbegin();
			for ( ; rit != pat->rend() && rit->first > low ; ++rit) {
					psycle::core::PatternEvent& ev = rit->second;
					if (ev.track() == editcur.track ) {
						psycle::core::Pattern::iterator it = rit.base();
						--it;
						psycle::core::PatternEvent old_event = it->second;
						double old_pos = it->first;
						double new_pos = old_pos + 1 / beat_zoom;
						it = pat->erase(it);
						if (new_pos >= pat->beats())
							break;
						it = pat->insert(new_pos, old_event);
					}
			}
			NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);
			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}



		//////////////////////////////////////////////////////////////////////
		// Plays entire row of the pattern

		void PatternView::PlayCurrentRow(void)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#else
			if (Global::pConfig->_followSong)
			{
				bScrollDetatch=false;
			}
			PatternEvent* pEntry = (PatternEvent*)_ptrackline(_ps(),0,editcur.line);
			for (int i=0; i<song()->tracks();i++)
			{
				if (pEntry->machine() < MAX_MACHINES && !song()->_trackMuted[i])
				{
					Machine *pMachine = song()->machine(pEntry->machine());
					if (pMachine)
					{
						if ( !pMachine->_mute)	
						{
							pMachine->Tick(i,*pEntry);
						}
					}
				}
				pEntry++;
			}
#endif
		}

		void PatternView::PlayCurrentNote(void)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#else
			if (Global::pConfig->_followSong)
			{
				bScrollDetatch=false;
			}

			PatternEvent* pEntry = (PatternEvent*)_ptrackline();
			if (pEntry->machine() < MAX_MACHINES)
			{
				Machine *pMachine = song()->machine(pEntry->machine());
				if (pMachine)
				{
					if ( !pMachine->_mute)	
					{
						pMachine->Tick(editcur.track,*pEntry);
					}
				}
			}
#endif
		}


		//////////////////////////////////////////////////////////////////////
		// Cursor Moving Functions
		void PatternView::PrevCol(bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;

			if(--editcur.col < 0)
			{
				editcur.col=8;
				main()->StatusBarIdle();
				if (editcur.track == 0)
				{
					if ( wrap )
						editcur.track = song()->tracks()-1;
					else 
						editcur.col=0;
				}
				else 
					--editcur.track;
			}
			if (updateDisplay) 
			{
				Repaint(draw_modes::cursor);
			}
		}

		void PatternView::NextCol(bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;

			if (++editcur.col > 8)
			{
				editcur.col = 0;
				main()->StatusBarIdle();
				if (editcur.track == song()->tracks()-1)
				{
					if ( wrap ) 
						editcur.track = 0;
					else 
						editcur.col=8;
				}
				else 
					++editcur.track;
			}
			if (updateDisplay) 
			{
				Repaint(draw_modes::cursor);
			}
		}

		void PatternView::PrevLine(int x, bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;
			const int nl = static_cast<int>(project()->beat_zoom() * pattern()->beats());

			editcur.line -= x;

			if(editcur.line<0)
			{
				if(wrap)
				{ 
					editcur.line = nl + editcur.line % nl; 
				}
				else	
				{ 
					editcur.line = 0;	
				}
			}
			main()->StatusBarIdle();
			if (updateDisplay)
				Repaint(draw_modes::cursor);
		}

		void PatternView::AdvanceLine(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;
			const int nl = static_cast<int>(project()->beat_zoom() * pattern()->beats());
			// <sampler> a bit recoded. 
			if (x<0) //kind of trick used to advance track (related to chord mode).
			{
				editcur.track+=1;
				if (editcur.track >= song()->tracks())
				{
					editcur.track=0;
					editcur.line+=1;
				}
			}
			else //advance corresponding rows
			{
				if (wrap)
				{
					editcur.line += x;
					editcur.line = editcur.line % nl; 
				}
				else
				{
					if (editcur.line + x < nl) editcur.line = editcur.line + x;
					else editcur.line = nl -1; //as it has worked always.
				}
				
			}

			main()->StatusBarIdle();
			if (updateDisplay)
				Repaint(draw_modes::cursor);
		}

		void PatternView::AdvanceTrack(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;

			editcur.track+=x;
			editcur.col=0;
			if(editcur.track>= song()->tracks())
			{
				if ( wrap ) editcur.track=0;
				else editcur.track=song()->tracks()-1;
			}
			
			main()->StatusBarIdle();
			if (updateDisplay)
				Repaint(draw_modes::cursor);
		}

		void PatternView::PrevTrack(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			blockSelectBarState = 1;

			editcur.track-=x;
			editcur.col=0;
			
			if(editcur.track<0)
			{
				if (wrap) editcur.track=song()->tracks()-1;
				else editcur.track=0;
			}
			
			main()->StatusBarIdle();
			if (updateDisplay)
				Repaint(draw_modes::cursor);
		}


		//////////////////////////////////////////////////////////////////////
		// Pattern Modifier functions ( Copy&paste , Transpose, ... )

		void PatternView::patCut() {
			if (child_view()->viewMode == view_modes::pattern) {
				patBufferLines = project()->beat_zoom() * pattern()->beats();
				block_buffer_pattern_ = *pattern();
				project()->cmd_manager()->ExecuteCommand(new PatDeleteCommand(this));
				patBufferCopy = true;
				NewPatternDraw(0,song()->tracks(),0,patBufferLines-1);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::patCopy() {
			if(child_view()->viewMode == view_modes::pattern) {
				block_buffer_pattern_ = *pattern();
			}
		}

		void PatternView::patPaste() {
			// UNDO CODE PATT PASTE
			if(patBufferCopy && child_view()->viewMode == view_modes::pattern) {
				pattern()->erase(0, pattern()->beats());
				pattern()->insert(block_buffer_pattern_, 0, 0);
				Repaint(draw_modes::pattern);
			}
		}

		void PatternView::patMixPaste() {
			// UNDO CODE PATT PASTE
			if(patBufferCopy && child_view()->viewMode == view_modes::pattern) {
				pattern()->insert(block_buffer_pattern_, 0, 0);
				Repaint(draw_modes::pattern);
			}
		}

		void PatternView::patDelete() {
			if(child_view()->viewMode == view_modes::pattern) {
				patBufferLines = pattern()->beats() * project()->beat_zoom();
				pattern()->erase(0, pattern()->beats());
				NewPatternDraw(0,song()->tracks(),0,patBufferLines-1);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::patTranspose(int trp) {
			if(child_view()->viewMode == view_modes::pattern) {
				// pattern()->Transpose(trp, 
				const int pLines = pattern()->beats() * project()->beat_zoom();
				double pos = line_pos(editcur.line, true);
				pattern()->Transpose(trp, pos, pattern()->beats());
				NewPatternDraw(0,song()->tracks(),editcur.line,pLines-1);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::StartBlock(int track,int line, int col)
		{

			//reinitialise the select bar state
			blockSelectBarState = 1;

			blockSel.start.track=track;
			blockSel.start.line=line;
			iniSelec = blockSel.start;

			if ( blockSelected )
			{
				if(blockSel.end.line<blockSel.start.line)
					blockSel.end.line=blockSel.start.line;
				
				if(blockSel.end.track<blockSel.start.track)
					blockSel.end.track=blockSel.start.track;
			}
			else
			{
				blockSel.end.line=line;
				blockSel.end.track=track;
			}
			blockSelected=true;
			Repaint(draw_modes::selection);
		}

		void PatternView::ChangeBlock(int track,int line, int col)
		{
			if ( blockSelected )
			{
				if ( track > iniSelec.track )
				{
					blockSel.start.track = iniSelec.track;
					blockSel.end.track = track;
				}
				else
				{
					blockSel.start.track = track;
					blockSel.end.track = iniSelec.track;
				}
				if ( line > iniSelec.line )
				{
					blockSel.start.line = iniSelec.line;
					blockSel.end.line = line;
				}
				else
				{
					blockSel.start.line = line;
					blockSel.end.line = iniSelec.line;
				}
			}
			else
			{
				blockSel.start.track=track;
				blockSel.start.line=line;
				blockSel.end.track=track;
				blockSel.end.line=line;
				iniSelec = blockSel.start;
			}
			blockSelected=true;

			Repaint(draw_modes::selection);
		}

		void PatternView::EndBlock(int track,int line, int col)
		{
			blockSel.end.track=track;
			blockSel.end.line=line;
			
			if ( blockSelected )
			{
				TRACE("%i,%i",blockSel.end.line,blockSel.start.line);
				if(blockSel.end.line<blockSel.start.line)
				{
					int tmp = blockSel.start.line;
					blockSel.start.line=blockSel.end.line;
					blockSel.end.line=tmp;
		//			blockSel.end.line=blockSel.start.line;
				}
				
				TRACE("%i,%i",blockSel.end.track,blockSel.start.track);
				if(blockSel.end.track<blockSel.start.track)
				{
					int tmp = blockSel.start.track;
					blockSel.start.track=blockSel.end.track;
					blockSel.end.track=tmp;
		//			blockSel.end.track=blockSel.start.track;
				}
				
			}
			else
			{
				blockSel.start.track=track;
				blockSel.start.line=line;
				iniSelec = blockSel.start;
			}
			blockSelected=true;

			Repaint(draw_modes::selection);
		}

		void PatternView::BlockUnmark() {
			blockSelected=false;
			//reinitialise the select bar state
			blockSelectBarState = 1;
			Repaint(draw_modes::selection);
		}

		double PatternView::line_pos(int line, bool nearest) const {
			double pos = (line - (nearest ? 0.4999 : 0)) / static_cast<double>(project()->beat_zoom());
			return pos;
		}

		void PatternView::CopyBlock(bool cutit) {
			// UNDO CODE HERE CUT
			if(blockSelected) {
				isBlockCopied=true;
				blockNTracks=(blockSel.end.track-blockSel.start.track)+1;
				blockNLines=(blockSel.end.line-blockSel.start.line)+1;
				block_buffer_pattern_ = pattern()->Clone(line_pos(blockSel.start.line, true),
														 line_pos(blockSel.end.line+1, true),
														 blockSel.start.track,
														 blockSel.end.track+1);
				if (cutit) {
					project()->cmd_manager()->ExecuteCommand(new DeleteBlockCommand(this));				
					NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
					Repaint(draw_modes::data);
				}
			}
		}

		void PatternView::DeleteBlock()
		{
			if (blockSelected) {
				pattern()->erase(line_pos(blockSel.start.line, true),
								 line_pos(blockSel.end.line+1, true),
								 blockSel.start.track,
								 blockSel.end.track+1);
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::PasteBlock(int tx,int lx,bool mix,bool save)
		{
			if (isBlockCopied) {
				if (!mix) {
					pattern()->erase(line_pos(lx, true), line_pos(lx + blockNLines+1, true) + 1,
									 tx, tx + blockNTracks);
				}
				pattern()->insert(block_buffer_pattern_, line_pos(lx, true), tx);
				if (Global::pInputHandler->bMoveCursorPaste)
				{
					const int nl = pattern()->beats() * project()->beat_zoom();
					if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
					else editcur.line = nl-1;
				}
				bScrollDetatch=false;
				NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::SwitchBlock(int destt, int destl)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#else
			if(blockSelected || isBlockCopied)// With shift+arrows, moving the cursor unselects the block, so in this case it is a three step
			{									// operation: select, copy, switch, instead of select, switch.
				int ps=song()->playOrder[editPosition];
				int nl = song()->patternLines[ps];
				bool bSwapTracks = false;
				bool bSwapLines = false;
				int ls=0;
				int ts=0;
				//Where to start reading and writing to free the destination area.
				int startRT=destt;
				int startRL=destl;
				int startWT=blockLastOrigin.start.track;
				int startWL=blockLastOrigin.start.line;
				PatternEvent blank;

				// Copy block(1) if not done already.
				if (blockSelected) CopyBlock(false);
				int stopT=destt+blockNTracks;
				int stopL=destl+blockNLines;

				// We backup the data of the whole block.
				// Do the blocks overlap? Then take care of moving the appropiate data.
				if (abs(blockLastOrigin.start.track-destt) < blockNTracks	&& abs(blockLastOrigin.start.line-destl) < blockNLines )
				{
					if 	( blockLastOrigin.start.track != destt )  //Is the origin and destination track different?
					{
						// ok, then we need to exchange some of the tracks.
						bSwapTracks = true;

						// If the switch moves to the left, exchange the start of the destination block
						if ( blockLastOrigin.start.track > destt)
						{
							startRT=destt;
							startWT=destt+blockNTracks;
							stopT=blockLastOrigin.start.track;
						}
						else // else, exchange the end of the destination block.
						{
							startRT=blockLastOrigin.start.track+blockNTracks;
							startWT=blockLastOrigin.start.track;
							stopT=destt+blockNTracks;
						}
					}
					if ( blockLastOrigin.start.line != destl )  //Is the origin and destination line different?
					{
						// ok, then we need to exchange some of the lines.
						bSwapLines = true;

						// do we have to swap both, tracks and lines?
						if (bSwapTracks)
						{
							// We have a situation like this :  ( - don't care , O origin D destination Q overlap.)
							// The swap cannot be clean, as in maintaining the structure.
							//--OOOO   >>>>> --1234  >>>> --BCDE
							//DDQQOO   >>>>> 56789A  >>>> 123456
							//DDDD--   >>>>> BCDE--  >>>> 789A--

							int startRT2, startRL2, startWT2, startWL2, stopT2, stopL2;
							if ( blockLastOrigin.start.line > destl)
							{
								startRL2=destl;
								startWL2=destl+blockNLines;
								stopL2=blockLastOrigin.start.line;
								startRL=blockLastOrigin.start.line;
								startWL=blockLastOrigin.start.line;
								stopL=destl+blockNLines;
							}
							else
							{
								startRL2=blockLastOrigin.start.line+blockNLines;
								startWL2=blockLastOrigin.start.line;
								stopL2=destl+blockNLines;
								startRL=destl;
								startWL=destl;
								stopL=blockLastOrigin.start.line+blockNLines;
							}
							if ( blockLastOrigin.start.track > destt)
							{
								startRT2=destt;
								startWT2=blockLastOrigin.start.track;
								stopT2=destt+blockNTracks;
							}
							else
							{
								startRT2=destt;
								startWT2=blockLastOrigin.start.track;
								stopT2=destt+blockNTracks;
							}
							// We exchange just the lines here. The loop outside will exchange the tracks.
							ts = startWT2;
							for (int t=startRT2;t<stopT2 && t<song()->tracks() && ts<song()->tracks();t++)
							{
								ls=startWL2;
								for (int l=startRL2;l<stopL2 && l<nl && ls <nl;l++)
								{
									unsigned char *offset_target=_ptrackline(ps,ts,ls);
									unsigned char *offset_source=_ptrackline(ps,t,l);

									memcpy(offset_target,offset_source,EVENT_SIZE);

									++ls;
								}
								++ts;
							}
						}
						else
						{
							// If the switch moves to the top, exchange the start of the destination block
							if ( blockLastOrigin.start.line > destl)
							{
								startRL=destl;
								startWL=destl+blockNLines;
								stopL=blockLastOrigin.start.line;
							}
							else
							{
								startRL=blockLastOrigin.start.line+blockNLines;
								startWL=blockLastOrigin.start.line;
								stopL=destl+blockNLines;
							}
						}
					}
					// There is nothing to Swap. blocks are the same.
					if (!bSwapTracks && !bSwapLines) return;
				}
			
				// do Swap "inplace".
				ts = startWT;
				for (int t=startRT;t<stopT && t<song()->tracks() && ts <song()->tracks();t++)
				{
					ls=startWL;
					for (int l=startRL;l<stopL && l<nl && ls<nl;l++)
					{
						unsigned char *offset_target=_ptrackline(ps,ts,ls);
						unsigned char *offset_source=_ptrackline(ps,t,l);

						memcpy(offset_target,offset_source,EVENT_SIZE);

						++ls;
					}
					++ts;
				}

				// Finally, paste the Original selected block on the freed space.
				PasteBlock(destt, destl, false,false);
				
				NewPatternDraw(0,song()->tracks()-1,0,nl-1);
				Repaint(draw_modes::data);
			}
#endif
		}


		void PatternView::SaveBlock(FILE* file)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			int ps = _ps();
			int nlines = song()->patternLines[ps];
			int ntracks = song()->tracks();
			fwrite(&ntracks, sizeof(int), 1, file);
			fwrite(&nlines, sizeof(int), 1, file);

			for (int t=0;t<song()->tracks();t++)
			{
				for (int l=0;l<nlines;l++)
				{
					unsigned char* offset_source=_ptrackline(ps,t,l);
					
					fwrite(offset_source,sizeof(char),EVENT_SIZE,file);
				}
			}
#endif
		}

		void PatternView::LoadBlock(FILE* file)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			int nt, nl;
			fread(&nt,sizeof(int),1,file);
			fread(&nl,sizeof(int),1,file);

			if ((nt > 0) && (nl > 0))
			{

				int ps = _ps();
				int nlines = song()->patternLines[ps];
				AddUndo(ps,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
				if (nlines != nl)
				{
					AddUndoLength(ps,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					song()->patternLines[ps] = nl;
				}

				for (int t=0;t<nt;t++)
				{
					for (int l=0;l<nl;l++)
					{
						if(l<MAX_LINES && t<MAX_TRACKS)
						{
							unsigned char* offset_target=_ptrackline(ps,t,l);
							fread(offset_target,sizeof(char),EVENT_SIZE,file);
						}
					}
				}
				PatternEvent blank;

				for (int t = nt; t < MAX_TRACKS;t++)
				{
					for (int l = nl; l < MAX_LINES; l++)
					{
						unsigned char* offset_target=_ptrackline(ps,t,l);
						memcpy(offset_target,&blank,EVENT_SIZE);
					}
				}
				Repaint(draw_modes::pattern);
			}
#endif
		}

		void PatternView::DoubleLength()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			// UNDO CODE DOUBLE LENGTH
			unsigned char *toffset;
			PatternEvent blank;
			int st, et, sl, el,nl;

			int ps = _ps();
			if ( blockSelected )
			{
		///////////////////////////////////////////////////////// Add ROW
				st=blockSel.start.track;		
				et=blockSel.end.track+1;
				sl=blockSel.start.line;			
				nl=((blockSel.end.line-sl)/2)+1;
				el=blockSel.end.line;
				AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
			}
			else 
			{
				st=0;		
				et=song()->tracks();		
				sl=0;
				nl= song()->patternLines[ps]/2;	
				el=song()->patternLines[ps]-1;
				AddUndo(ps,0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
			}

			for (int t=st;t<et;t++)
			{
				toffset=_ptrack(ps,t);
				memcpy(toffset+el*MULTIPLY,&blank,EVENT_SIZE);
				for (int l=nl-1;l>0;l--)
				{
					memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,EVENT_SIZE);
					memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,&blank,EVENT_SIZE);
				}
			}

			NewPatternDraw(st,et,sl,el);
			Repaint(draw_modes::data);
#endif
		}

		void PatternView::HalveLength()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			// UNDO CODE HALF LENGTH
			unsigned char *toffset;
			int st, et, sl, el,nl;
			int ps = _ps();
			PatternEvent blank;

			if ( blockSelected )
			{
		///////////////////////////////////////////////////////// Add ROW
				st=blockSel.start.track;	
				et=blockSel.end.track+1;
				sl=blockSel.start.line;		
				nl=blockSel.end.line-sl+1;
				el=nl/2;
				AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
			}
			else 
			{
				st=0;	
				et=song()->tracks();		
				sl=0;
				nl=song()->patternLines[ps];	
				el=song()->patternLines[ps]/2;
				AddUndo(ps,0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);
			}
			
			for (int t=st;t<et;t++)
			{
				toffset=_ptrack(ps,t);
				int l;
				for (l=1;l<el;l++)
				{
					memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,EVENT_SIZE);
				}
				while (l < nl)
				{
					memcpy(toffset+((l+sl)*MULTIPLY),&blank,EVENT_SIZE);
					l++;
				}
			}

			NewPatternDraw(st,et,sl,nl+sl);
			Repaint(draw_modes::data);
#endif
		}


		void PatternView::BlockTranspose(int trp)
		{
			if (blockSelected == true) {
				pattern()->Transpose(trp, 
									 line_pos(blockSel.start.line, true),
								     line_pos(blockSel.end.line+1, true),
								     blockSel.start.track,
								     blockSel.end.track+1);
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}


		void PatternView::BlockGenChange(int x)
		{
			if (blockSelected == true) {
				pattern()->ChangeMac(x, 
									 line_pos(blockSel.start.line, true),
								     line_pos(blockSel.end.line+1, true),
								     blockSel.start.track,
								     blockSel.end.track+1);
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::BlockInsChange(int x)
		{
			if (blockSelected == true) {
				pattern()->ChangeInst(x, 
									  line_pos(blockSel.start.line, true),
								      line_pos(blockSel.end.line+1, true),
								      blockSel.start.track,
								      blockSel.end.track+1);
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void PatternView::BlockParamInterpolate(int *points, int twktype)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			if (blockSelected)
			{
				const int ps = _ps();
				///////////////////////////////////////////////////////// Add ROW
				unsigned char *toffset=_ppattern(ps);
				
				AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);
				
				const int initvalue = 
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.start.line*MULTIPLY+3) * 0x100 +
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.start.line*MULTIPLY+4);
				const int endvalue =
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.end.line*MULTIPLY+3) * 0x100 +
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.end.line*MULTIPLY+4);
				const float addvalue = float(endvalue - initvalue) / (blockSel.end.line - blockSel.start.line);
				const int firstrow = (blockSel.start.track*EVENT_SIZE)+(blockSel.start.line*MULTIPLY);
				int displace = firstrow;
				if ( toffset[firstrow] == notecommands::tweak || toffset[firstrow] == notecommands::tweak_slide || toffset[firstrow] == notecommands::midi_cc || twktype != notecommands::empty)
				{
					unsigned char note = (twktype != notecommands::empty)?twktype:toffset[firstrow];
					unsigned char aux = (twktype != notecommands::empty)?song()->auxcolSelected:toffset[firstrow+1];
					unsigned char mac = (twktype != notecommands::empty)?song()->seqBus:toffset[firstrow+2];
					for (int l=blockSel.start.line;l<=blockSel.end.line;l++)
					{
						toffset[displace]=note;
						toffset[displace+1]=aux;
						toffset[displace+2]=mac;
						int val= (points)? points[l-blockSel.start.line]: /* round toward zero */ static_cast<int>(initvalue+addvalue*(l-blockSel.start.line));
						if ( val == -1 ) continue;
						toffset[displace+3]=static_cast<unsigned char>(val/0x100);
						toffset[displace+4]=static_cast<unsigned char>(val%0x100);
						displace+=MULTIPLY;
					}
				}
				else
				{
					unsigned char mac = toffset[firstrow+2];
					for (int l=blockSel.start.line;l<=blockSel.end.line;l++)
					{
						int val = (points)? points[l-blockSel.start.line]: /* round toward zero */ static_cast<int>(initvalue+addvalue*(l-blockSel.start.line));
						if ( val == -1 ) continue;
						toffset[displace+2]=mac;
						toffset[displace+3]=static_cast<unsigned char>(val/0x100);
						toffset[displace+4]=static_cast<unsigned char>(val%0x100);
						displace+=MULTIPLY;
					}
				}
#endif
			NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
			Repaint(draw_modes::data);
		}
	

		void PatternView::SelectMachineUnderCursor()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			unsigned char *toffset=_ptrackline();

			PatternEvent *entry = (PatternEvent*) toffset;

			if ( entry->machine() < MAX_BUSES*2 ) song()->seqBus = entry->machine();
			main()->ChangeGen(song()->seqBus);
			if ( entry->instrument() != 255 ) song()->auxcolSelected = entry->instrument();
			main()->ChangeIns(song()->auxcolSelected);
#endif
		}

		void PatternView::SelectNextTrack()
		{
			int i;
			for (i = editcur.track+1; i < song()->tracks(); ++i)
			{
				if (song()->sequence().trackArmed(i))
				{
					if (Global::pInputHandler->notetrack[i] == notecommands::release)
					{
						break;
					}
				}
			}
			if (i >= song()->tracks())
			{
				for (i = 0; i <= editcur.track; ++i)
				{
					if (song()->sequence().trackArmed(i))
					{
						if (Global::pInputHandler->notetrack[i] == notecommands::release)
						{
							break;
						}
					}
				}
			}
			editcur.track = i;
			while(song()->sequence().trackArmed(editcur.track) == 0)
			{
				if(++editcur.track >= song()->tracks())
					editcur.track=0;
			}
			editcur.col = 0;
		}

		void PatternView::patTrackMute()
		{
			if (child_view()->viewMode == view_modes::pattern) {
				song()->sequence().setMutedTrack(editcur.track, 
					!song()->sequence().trackMuted(editcur.track));
				Repaint(draw_modes::track_header);
			}
		}

		void PatternView::patTrackSolo()
		{			
			if (child_view()->viewMode == view_modes::pattern)
			{
				if (song()->_trackSoloed == editcur.track)
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						song()->sequence().setMutedTrack(i, false);
					}
					song()->_trackSoloed = -1;
				}
				else
				{
					for (int i = 0; i < MAX_TRACKS; ++i) {
						song()->sequence().setMutedTrack(i, true);
					}
					song()->sequence().setMutedTrack(editcur.track, false);
					song()->_trackSoloed = editcur.track;
				}
				Repaint(draw_modes::track_header);
			}
		}

		void PatternView::patTrackRecord() {
			if (child_view()->viewMode == view_modes::pattern) {
				song()->sequence().setArmedTrack(editcur.track,
					!song()->sequence().trackArmed(editcur.track));				
				Repaint(draw_modes::track_header);
			}
		}

		void PatternView::OnRButtonDown(UINT nFlags, CPoint point) {	
		}
		
		void PatternView::OnRButtonUp( UINT nFlags, CPoint point ) {
		}

		void PatternView::OnContextMenu(CWnd* pWnd, CPoint point) {
				CMenu menu;
				VERIFY(menu.LoadMenu(IDR_POPUPMENU));
				CMenu* pPopup = menu.GetSubMenu(0);
				ASSERT(pPopup != NULL);
				pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
				
				menu.DestroyMenu();
		//		Repaint(draw_modes::cursor);
		}


		void PatternView::OnLButtonDown(UINT nFlags, CPoint point)
		{			
				int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
				if ( ttm >= song()->tracks() ) ttm = song()->tracks()-1;
				else if ( ttm < 0 ) ttm = 0;
				
				if (point.y >= 0 && point.y < YOFFSET ) // Mouse is in Track Header.
				{	
					int pointpos= ((point.x-XOFFSET)%ROWWIDTH) - HEADER_INDENT;

					if (InRect(pointpos,point.y,PatHeaderCoords.dRecordOn,PatHeaderCoords.sRecordOn))
					{
						song()->sequence().setArmedTrack(ttm,
							!song()->sequence().trackArmed(ttm));
						song()->_trackArmedCount = 0;
						for ( int i=0;i<MAX_TRACKS;i++ )
						{
							if (song()->sequence().trackArmed(i))
							{
								song()->_trackArmedCount++;
							}
						}
					}
					else if (InRect(pointpos,point.y,PatHeaderCoords.dMuteOn,PatHeaderCoords.sMuteOn))
					{
						song()->sequence().setMutedTrack(ttm,
							!song()->sequence().trackMuted(ttm));
					}
					else if (InRect(pointpos,point.y,PatHeaderCoords.dSoloOn,PatHeaderCoords.sSoloOn))
					{
						if (song()->_trackSoloed != ttm )
						{
							for ( int i=0;i<MAX_TRACKS;i++ )
							{
								song()->sequence().setMutedTrack(i, true);
							}
							song()->sequence().setMutedTrack(ttm, false);
							song()->_trackSoloed = ttm;
						}
						else
						{
							for ( int i=0;i<MAX_TRACKS;i++ )
							{
								song()->sequence().setMutedTrack(i, false);
							}
							song()->_trackSoloed = -1;
						}
					}
					oldm.track = -1;
					Repaint(draw_modes::track_header);
				}
				else if ( point.y >= YOFFSET )
				{
					oldm.track=ttm;

					const int plines = pattern()->beats() * project()->beat_zoom();
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
		}

		void PatternView::OnLButtonUp(UINT nFlags, CPoint point)
		{
			if ( (blockStart) &&
					( point.y > YOFFSET && point.y < YOFFSET+(maxl*ROWHEIGHT)) &&
					(point.x > XOFFSET && point.x < XOFFSET+(maxt*ROWWIDTH)))
				{
					editcur.track = tOff + char((point.x-XOFFSET)/ROWWIDTH);
		//			if ( editcur.track >= song()->tracks() ) editcur.track = song()->tracks()-1;
		//			else if ( editcur.track < 0 ) editcur.track = 0;

		//			int plines = song()->patternLines[_ps()];
					editcur.line = lOff + (point.y-YOFFSET)/ROWHEIGHT;
		//			if ( editcur.line >= plines ) {  editcur.line = plines - 1; }
		//			else if ( editcur.line < 0 ) editcur.line = 0;

					editcur.col = _xtoCol((point.x-XOFFSET)%ROWWIDTH);
					Repaint(draw_modes::cursor);
					main()->StatusBarIdle();
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
		}


		void PatternView::OnMouseMove( UINT nFlags, CPoint point )
		{
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
						if ( ttm >= song()->tracks() ) // Out of Range		
						{	
							ttm = song()->tracks()-1;
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE			
				const int plines = pattern()->beats() * project()->beat_zoom();
#else
				const int plines = song()->patternLines[_ps()];
#endif
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
							if (blockLastOrigin.end.track+(ttm-editcur.track) >= song()->tracks()) tstart = song()->tracks()-blockLastOrigin.end.track-1;
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
						const int nlines = pattern()->beats() * project()->beat_zoom();
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
							else if (nPos>song()->tracks()-VISTRACKS)
								ntOff=song()->tracks()-VISTRACKS;
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
							else if (nPos>song()->tracks()-VISTRACKS)
								ntOff=song()->tracks()-VISTRACKS;
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
		}



		void PatternView::OnLButtonDblClk( UINT nFlags, CPoint point )
		{
			if (( point.y >= YOFFSET ) && (point.x >= XOFFSET)) {
				const int ttm = tOff + (point.x-XOFFSET)/ROWWIDTH;
#if PSYCLE__CONFIGURATION__USE_PSYCORE			
				const int nlines = pattern()->beats() * project()->beat_zoom();
#else
				const int nlines = song()->patternLines[song()->playOrder[editPosition]];
#endif
				StartBlock(ttm,0,0);
				EndBlock(ttm,nlines-1,8);
				blockStart = false;
			}
		}


		void PatternView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE			
			const int nlines = pattern()->beats() * project()->beat_zoom();
#else
			const int nlines = song()->patternLines[_ps()];
#endif
			int nPos = lOff - (zDelta/30);
			if (nPos > lOff) {
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
		}

		void PatternView::OnMButtonDown( UINT nFlags, CPoint point )
		{
			MBStart.x = point.x;
			MBStart.y = point.y;
		}

		void PatternView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
		{

#if PSYCLE__CONFIGURATION__USE_PSYCORE			
			const int plines = pattern()->beats() * project()->beat_zoom();
#else
			const int plines = song()->patternLines[_ps()];
#endif

			switch(nSBCode)
			{
				case SB_LINEDOWN:
					if ( lOff<plines-VISLINES)
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
						if ( lOff<plines-VISLINES)
						{
							const int nl = plines-VISLINES;
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
							const int nl = plines-VISLINES;
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
		}


		void PatternView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
		{
				switch(nSBCode)
				{
					case SB_LINERIGHT:
					case SB_PAGERIGHT:
						if ( tOff<song()->tracks()-VISTRACKS)
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
							const int nt = song()->tracks();
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
		}

		void PatternView::Repaint(draw_modes::draw_mode drawMode)
		{
			if (drawMode >= draw_modes::pattern || drawMode == draw_modes::all )	
			{
				PreparePatternRefresh(drawMode);				
			}
		}


		void PatternView::PerformCmd(CmdDef &cmd, BOOL brepeat)
		{
			switch(cmd.GetID())
			{
			case cdefNull:
				break;

			case cdefPatternCut:
				patCut();
				break;

			case cdefPatternCopy:
				patCopy();
				break;

			case cdefPatternPaste:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new PatPasteCommand(this));
				break;

			case cdefPatternMixPaste:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new PatPasteCommand(this, true));
				break;

			case cdefPatternDelete:
				project()->cmd_manager()->ExecuteCommand(new PatDeleteCommand(this));
				break;

			case cdefPatternTrackMute:
				patTrackMute();
				break;

			case cdefPatternTrackSolo:
				patTrackSolo();
				break;

			case cdefPatternTrackRecord:
				patTrackRecord();
				break;

			case cdefFollowSong:	

				//used by song follow toggle
				//CButton*cb=(CButton*)pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW);

				if (((CButton*)main()->m_wndSeq.GetDlgItem(IDC_FOLLOW))->GetCheck() == 0) 
				{
					((CButton*)main()->m_wndSeq.GetDlgItem(IDC_FOLLOW))->SetCheck(1);
				} 
				else
				{
					((CButton*)main()->m_wndSeq.GetDlgItem(IDC_FOLLOW))->SetCheck(0);
				}
				main()->OnFollowSong();
				break;

			case cdefKeyStopAny:
				EnterNoteoffAny();
				break;

			case cdefColumnNext:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				AdvanceTrack(1,Global::pConfig->_wrapAround);
				break;

			case cdefColumnPrev:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				PrevTrack(1,Global::pConfig->_wrapAround);
				break;

			case cdefNavLeft:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					PrevCol(Global::pConfig->_wrapAround);
					if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
						BlockUnmark();
				}
				else
				{
					if ( !blockSelected )
					{
						StartBlock(editcur.track,editcur.line,editcur.col);
					}
					PrevTrack(1,Global::pConfig->_wrapAround);
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}

				bDoingSelection = false;
				break;
			case cdefNavRight:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					NextCol(Global::pConfig->_wrapAround);
					if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
						BlockUnmark();
				}
				else
				{
					if ( !blockSelected)
					{
						StartBlock(editcur.track,editcur.line,editcur.col);
					}
					AdvanceTrack(1,Global::pConfig->_wrapAround);
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}

				bDoingSelection = false;
				break;
			case cdefNavUp:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( bDoingSelection && !blockSelected)
				{
					StartBlock(editcur.track,editcur.line,editcur.col);
				}
				if (patStep == 0)
					PrevLine(1,Global::pConfig->_wrapAround);
				else
					//if added by sampler. New option.
					if (!Global::pConfig->_NavigationIgnoresStep)
						PrevLine(patStep,Global::pConfig->_wrapAround);//before
					else
						PrevLine(1,Global::pConfig->_wrapAround);//new option
				if ( bDoingSelection )
				{
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
					BlockUnmark();
				bDoingSelection = false;
				break;
			case cdefNavDn:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( bDoingSelection && !blockSelected)
				{
					StartBlock(editcur.track,editcur.line,editcur.col);
				}
				if (patStep == 0)
					AdvanceLine(1,Global::pConfig->_wrapAround);
				else
					//if added by sampler. New option.
					if (!Global::pConfig->_NavigationIgnoresStep)
						AdvanceLine(patStep,Global::pConfig->_wrapAround); //before
					else
						AdvanceLine(1,Global::pConfig->_wrapAround);//new option
				if ( bDoingSelection )
				{
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
					BlockUnmark();
				bDoingSelection = false;
				break;
			case cdefNavPageUp:
				{
					int stepsize(0);
					if ( Global::pConfig->_pageUpSteps == 0) stepsize = song()->LinesPerBeat();
					else if ( Global::pConfig->_pageUpSteps == 1)stepsize = song()->LinesPerBeat()*Global::pConfig->pv_timesig;
					else stepsize = Global::pConfig->_pageUpSteps;

					//if added by sampler to move backward 16 lines when playing
					if (Global::pPlayer->playing() && Global::pConfig->_followSong)
					{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						
						PlayerTimeInfo& tinfo = Player::singleton().timeInfo();
						if (Player::singleton().loopEnabled()) {

							if (tinfo.playBeatPos() >= tinfo.cycleStartPos()) {
								Player::singleton().skip(-stepsize);
							}
							else {
								Player::singleton().skipTo(tinfo.cycleStartPos());
							}
						}
						else {
							if (tinfo.playBeatPos() >= tinfo.cycleStartPos()) {
								Player::singleton().skip(stepsize);
							}
							else {
								if (tinfo.playBeatPos() > 0 ) {
									Player::singleton().skip(-stepsize);
								}
								else {
									Player::singleton().skipTo(0);
								}
							}
						}
#else
						if (Global::pPlayer->_playBlock )
						{
							if (Global::pPlayer->_lineCounter >= stepsize) Global::pPlayer->_lineCounter -= stepsize;
							else
							{
								Global::pPlayer->_lineCounter = 0;
								Global::pPlayer->ExecuteLine();
							}
						}
						else
						{
							if (Global::pPlayer->_lineCounter >= stepsize) Global::pPlayer->_lineCounter -= stepsize;
							else
							{
								if (Global::pPlayer->_sequencePosition > 0)
								{
									Global::pPlayer->_sequencePosition -= 1;
									Global::pPlayer->_lineCounter = song()->patternLines[Global::pPlayer->_sequencePosition] - stepsize;												
								}
								else
								{
									Global::pPlayer->_lineCounter = 0;
									Global::pPlayer->ExecuteLine();
								}
							}
						}
#endif
					}
					//end of if added by sampler
					else
					{
						bScrollDetatch=false;
						ChordModeOffs = 0;

						if (bDoingSelection && !blockSelected)
						{
							StartBlock(editcur.track,editcur.line,editcur.col);
						}
						PrevLine(stepsize,false);
						if (bDoingSelection)
						{
							ChangeBlock(editcur.track,editcur.line,editcur.col);
						}
						else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
							BlockUnmark();
					}
				}
				break;

			case cdefNavPageDn:
				{
					int stepsize(0);
					if ( Global::pConfig->_pageUpSteps == 0) stepsize = song()->LinesPerBeat();
					else if ( Global::pConfig->_pageUpSteps == 1)stepsize = song()->LinesPerBeat()*Global::pConfig->pv_timesig;
					else stepsize = Global::pConfig->_pageUpSteps;

					//if added by sampler
					if (Global::pPlayer->playing() && Global::pConfig->_followSong)
					{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						Player::singleton().skip(stepsize);
#else
						Global::pPlayer->_lineCounter += stepsize;
#endif
					}
					//end of if added by sampler
					else
					{
						bScrollDetatch=false;
						ChordModeOffs = 0;
						if ( bDoingSelection && !blockSelected)
						{
							StartBlock(editcur.track,editcur.line,editcur.col);
						}
						AdvanceLine(stepsize,false);
						if ( bDoingSelection )
						{
							ChangeBlock(editcur.track,editcur.line,editcur.col);
						}
						else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
							BlockUnmark();
					}
				}
				break;
			
			case cdefNavTop:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( bDoingSelection && !blockSelected)
				{
					StartBlock(editcur.track,editcur.line,editcur.col);
				}
				if(bFT2HomeBehaviour)
				{
					editcur.line=0;
				}
				else
				{
					if (editcur.col != 0) 
						editcur.col = 0;
					else 
						if (editcur.track != 0 ) 
							editcur.track = 0;
						else 
							editcur.line = 0;
				}
				if (bDoingSelection)
				{
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
					BlockUnmark();

				Repaint(draw_modes::cursor);
				break;
			
			case cdefNavBottom:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if ( bDoingSelection && !blockSelected)
				{
					StartBlock(editcur.track,editcur.line,editcur.col);
				}
				if(bFT2HomeBehaviour)
				{
					editcur.line= pattern()->beats() * project()->beat_zoom() -1;
				}
				else
				{		
					if (editcur.col != 8) 
						editcur.col = 8;
					else if ( editcur.track != song()->tracks()-1 ) 
						editcur.track = song()->tracks()-1;
					else 
						editcur.line= pattern()->beats() * project()->beat_zoom() -1;
				}
				if ( bDoingSelection )
				{
					ChangeBlock(editcur.track,editcur.line,editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) 
					BlockUnmark();

				Repaint(draw_modes::cursor);
				break;
			
			case cdefRowInsert:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new InsertCurrCommand(this));
				break;

			case cdefRowDelete:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if (blockSelected && Global::pConfig->_windowsBlocks)
				{
					project()->cmd_manager()->ExecuteCommand(new DeleteBlockCommand(this));
				}
				else
				{
					project()->cmd_manager()->ExecuteCommand(new DeleteCurrCommand(this));
				}
				break;

			case cdefRowClear:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				if (blockSelected && Global::pConfig->_windowsBlocks)
				{
					project()->cmd_manager()->ExecuteCommand(new DeleteBlockCommand(this));
				}
				else
				{
					project()->cmd_manager()->ExecuteCommand(new ClearCurrCommand(this));
				}
				break;

			case cdefBlockStart:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				StartBlock(editcur.track,editcur.line,editcur.col);
				break;

			case cdefBlockEnd:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				EndBlock(editcur.track,editcur.line,editcur.col);
				break;

			case cdefSelectAll:
				{
					const int nl = pattern()->beats() * project()->beat_zoom();
					StartBlock(0,0,0);
					EndBlock(song()->tracks()-1,nl-1,8);
				}
				break;
				
			case cdefSelectCol:
				{
					const int nl = pattern()->beats() * project()->beat_zoom();
					StartBlock(editcur.track,0,0);
					EndBlock(editcur.track,nl-1,8);
				}
				break;

			case cdefSelectBar:
			//selects 4*tpb lines, 8*tpb lines 16*tpb lines, etc. up to number of lines in pattern
				{
					const int nl = pattern()->beats() * project()->beat_zoom();		
								
					bScrollDetatch=false;
					ChordModeOffs = 0;
					
					if (blockSelectBarState == 1) 
					{
						StartBlock(editcur.track,editcur.line,editcur.col);
					}

					int blockLength = (4 * blockSelectBarState * song()->LinesPerBeat())-1;

					if ((editcur.line + blockLength) >= nl-1)
					{
						EndBlock(editcur.track,nl-1,8);	
						blockSelectBarState = 1;
					}
					else
					{
						EndBlock(editcur.track,editcur.line + blockLength,8);
						blockSelectBarState *= 2;
					}	
					
				}
				break;

			case cdefEditQuantizeDec:
				main()->EditQuantizeChange(-1);
				break;

			case cdefEditQuantizeInc:
				main()->EditQuantizeChange(1);
				break;

			case cdefTransposeChannelInc:
				project()->cmd_manager()->ExecuteCommand(new PatTransposeCommand(this, 1));
				break;
			case cdefTransposeChannelDec:
				project()->cmd_manager()->ExecuteCommand(new PatTransposeCommand(this, -1));
				break;
			case cdefTransposeChannelInc12:
				project()->cmd_manager()->ExecuteCommand(new PatTransposeCommand(this, 12));
				break;
			case cdefTransposeChannelDec12:
				project()->cmd_manager()->ExecuteCommand(new PatTransposeCommand(this, -12));
				break;

			case cdefTransposeBlockInc:
				project()->cmd_manager()->ExecuteCommand(new BlockTransposeCommand(this,1));
				break;
			case cdefTransposeBlockDec:
				project()->cmd_manager()->ExecuteCommand(new BlockTransposeCommand(this,-1));
				break;
			case cdefTransposeBlockInc12:
				project()->cmd_manager()->ExecuteCommand(new BlockTransposeCommand(this,12));
				break;
			case cdefTransposeBlockDec12:
				project()->cmd_manager()->ExecuteCommand(new BlockTransposeCommand(this,-12));
				break;


			case cdefBlockUnMark:
				BlockUnmark();
				break;

			case cdefBlockDouble:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				DoubleLength();
				break;

			case cdefBlockHalve:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				HalveLength();
				break;

			case cdefBlockCut:
				CopyBlock(true);
				break;

			case cdefBlockCopy:
				CopyBlock(false);
				break;

			case cdefBlockPaste:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new PasteBlockCommand(this, editcur.track,editcur.line,false));
				break;

			case cdefBlockMix:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new PasteBlockCommand(this, editcur.track,editcur.line,false));
				break;

			case cdefBlockDelete:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->ExecuteCommand(new DeleteBlockCommand(this));
				break;

			case cdefBlockInterpolate:
				BlockParamInterpolate();
				break;

			case cdefBlockSetMachine:
				project()->cmd_manager()->ExecuteCommand(new ChangeGenCommand(this, song()->seqBus));
				break;

			case cdefBlockSetInstr:
				project()->cmd_manager()->ExecuteCommand(new ChangeInsCommand(this, song()->auxcolSelected));
				break;

			case cdefOctaveUp:
				main()->ShiftOctave(1);
				break;

			case cdefOctaveDn:
				main()->ShiftOctave(-1);
				break;
		

			case cdefEditToggle:
				bEditMode = !bEditMode;
				ChordModeOffs = 0;
				
//				if(bCtrlPlay) Stop();
				
		//		pChildView->Repaint(draw_modes::patternHeader);
				break;
				

			case cdefInfoPattern:
				if ( child_view()->viewMode == view_modes::pattern )
				{
					child_view()->OnPopPattenproperties();
				}
				break;

			
			case cdefUndo:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->Undo();
				break;

			case cdefRedo:
				bScrollDetatch=false;
				ChordModeOffs = 0;
				project()->cmd_manager()->Redo();
				break;
			}
		}

		void PatternView::Stop() {			
			parent_->OnBarstop();
		}

		void PatternView::PlaySong()  {
			child_view()->OnBarplay();
		}

		void PatternView::PlayFromCur() 
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			//todo: add proper code
			Player::singleton().start(0);
#else
			Global::pPlayer->Start(editPosition,editcur.line);
#endif
			main()->StatusBarIdle();
		}


		void PatternView::OnUpdateUndo(CCmdUI* pCmdUI)
		{
			// todo shouldnt be with a timer ..
			pCmdUI->Enable(project()->cmd_manager()->UndoSize() != 0);
		}

		void PatternView::OnUpdateRedo(CCmdUI* pCmdUI)
		{
			// todo shouldnt be with a timer ..
			pCmdUI->Enable(project()->cmd_manager()->RedoSize() != 0);
		}

		void PatternView::ShowSwingFillDlg(bool bTrackMode)
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
#else
			int st = song()->BeatsPerMin();
			static int sw = 2;
			static float sv = 13.0f;
			static float sp = -90.0f;
			static BOOL of = true;
			CSwingFillDlg dlg;
			dlg.tempo = st;
			dlg.width = sw;
			dlg.variance = sv;
			dlg.phase = sp;
			dlg.offset = true;

			dlg.DoModal();
			if (dlg.bGo)
			{
				st = dlg.tempo;
				sw = dlg.width;
				sv = dlg.variance;
				sp = dlg.phase;
				of = dlg.offset;
				float var = (sv/100.0f);

				// time to do our fill
				// first some math
				// our range has to go from spd+var to spd-var and back in width+1 lines
				float step = TWOPI_F/(sw);
				float index = sp*TWOPI_F/360;

				int l;
				int x;
				int y;
				int ny;
				if (bTrackMode)
				{
					x = editcur.track;
					y = 0;
					ny = song()->patternLines[_ps()];
				}
				else
				{
					x = blockSel.start.track;
					y = blockSel.start.line;
					ny = 1+blockSel.end.line-blockSel.start.line;
				}

				// remember we are at each speed for the length of time it takes to do one tick
				// this approximately calculates the offset
				float dcoffs = 0;
				if (of)
				{
					float swing=0;
					for (l=0;l<sw;l++)
					{
						float val = ((sinf(index)*var*st)+st);
						swing += (val/st)*(val/st);
						index+=step;
					}
					dcoffs = ((swing-sw)*st)/sw;
				}

				// now fill the pattern
				unsigned char *base = _ppattern();
				if (base)
				{
					AddUndo(_ps(),x,y,1,ny,editcur.track,editcur.line,editcur.col,editPosition);
					for (l=y;l<y+ny;l++)
					{
						int const displace=x*EVENT_SIZE+l*MULTIPLY;
						
						unsigned char *offset=base+displace;
						
						PatternEvent *entry = (PatternEvent*) offset;
						entry->setCommand(0xff);
						int val = lround<int32_t>(((std::sin(index)*var*st)+st)+dcoffs);//-0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
						if(val < 1) val = 1;
						else if(val > 255) val = 255;
						entry->setParameter(unsigned char (val));
						index+=step;
					}
					NewPatternDraw(x,x,y,y+ny);	
					Repaint(draw_modes::data);
				}
			}
#endif
		}

		void PatternView::ShowPatternDlg(void)
		{
			CPatDlg dlg(parent_, pattern());
			int nlines = pattern()->beats() * project()->beat_zoom();
			char name[32];
			std::strcpy(name,pattern()->name().c_str());

			dlg.patLines= nlines;
			std::strcpy(dlg.patName,name);
			main()->m_wndSeq.UpdateSequencer();
			
			if (dlg.DoModal() == IDOK)
			{
				double old_len = pattern()->beats();
				psycle::core::SequenceLine* line = *(song()->sequence().begin()+1);
				psycle::core::SequenceLine::iterator it = line->find(main()->m_wndSeq.selected_entry());
				if ( it != line->end()) {
					++it;
					if ( it != line->end()) {
						line->moveEntries(it->second, pattern()->beats() - old_len);
					}
				}
				if ( nlines != dlg.patLines )
				{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					///todo: this operation shouldn't be needed with psycore
#else
					AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					song()->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
					if ( std::strcmp(name,dlg.patName) != 0 )
					{
						std::strcpy(song()->patternName[patNum],dlg.patName);
						main()->StatusBarIdle();
					}
					Repaint(draw_modes::all);
#endif
				}
				else if ( strcmp(name,dlg.patName) != 0 )
				{
					// std::strcpy(song()->patternName[patNum],dlg.patName);
					main()->m_wndSeq.UpdatePlayOrder(true);
					main()->m_wndSeq.UpdateSequencer();
					//Repaint(draw_modes::patternHeader);
				}
			}
		}

		void PatternView::ShowTransformPatternDlg() {
			CTransformPatternDlg dlg(this);
			if (dlg.DoModal() == IDOK){
			}
		}

		void PatternView::OnPopMixpaste() {
			project()->cmd_manager()->ExecuteCommand(
				new PasteBlockCommand(this, editcur.track, editcur.line, false));
		}

		void PatternView::OnPopBlockswitch() {
			SwitchBlock(editcur.track,editcur.line);
		}

		void PatternView::OnPopPaste() {
			project()->cmd_manager()->ExecuteCommand(
				new PasteBlockCommand(this, editcur.track, editcur.line, false));
		}

		void PatternView::OnPopInterpolateCurve()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// todo
#else
			CInterpolateCurve dlg(blockSel.start.line,blockSel.end.line,song()->LinesPerBeat());
			
			int *valuearray = new int[blockSel.end.line-blockSel.start.line+1];
			int ps=song()->playOrder[editPosition];
			for (int i=0; i<=blockSel.end.line-blockSel.start.line; i++)
			{
				unsigned char *offset_target=_ptrackline(ps,blockSel.start.track,i+blockSel.start.line);
				if (*offset_target <= notecommands::release || *offset_target == notecommands::empty)
				{
					if ( *(offset_target+3) == 0 && *(offset_target+4) == 0 ) valuearray[i]=-1;
					else valuearray[i]= *(offset_target+3)*0x100 + *(offset_target+4);
				}
				else valuearray[i] = *(offset_target+3)*0x100 + *(offset_target+4);
			}
			unsigned char *offset_target=_ptrackline(ps,blockSel.start.track,blockSel.start.line);
			if ( *offset_target == notecommands::tweak ) dlg.AssignInitialValues(valuearray,0);

			else if ( *offset_target == notecommands::tweak_slide ) dlg.AssignInitialValues(valuearray,1);
			else if ( *offset_target == notecommands::midi_cc ) dlg.AssignInitialValues(valuearray,2);
			else dlg.AssignInitialValues(valuearray,-1);
			
			if (dlg.DoModal() == IDOK )
			{
				int twktype(notecommands::empty);
				if ( dlg.kftwk == 0 ) twktype = notecommands::tweak;
				else if ( dlg.kftwk == 1 ) twktype = notecommands::tweak_slide;
				else if ( dlg.kftwk == 2 ) twktype = notecommands::midi_cc;
				BlockParamInterpolate(dlg.kfresult,twktype);
			}
			delete valuearray;
#endif
		}

		/*
		bool InputHandler::EnterData(UINT nChar,UINT nFlags)
		{
			if ( pChildView->editcur.col == 0 )
			{
				// get command
				CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

		//		BOOL bRepeat = nFlags&0x4000;
				if ( cmd.GetType() == CT_Note )
				{
		//			if ((!bRepeat) || (cmd.GetNote() == notecommands::tweak) || (cmd.GetNote() == notecommands::tweakslide) || (cmd.GetNote() == notecommands::midicc))
		//			{
						pChildView->EnterNote(cmd.GetNote());
						return true;
		//			}
				}
				return false;
			}
			else if ( GetKeyState(VK_CONTROL)>=0 && GetKeyState(VK_SHIFT)>=0 )
			{
				return pChildView->MSBPut(nChar);
			}
			return false;
		}

		void InputHandler::StopNote(int note, bool bTranspose,Machine*pMachine)
		{
			assert(note>=0 && note < 128);

			if(note<0)
				return;

			// octave offset 
			if(note<120)
			{
				if(bTranspose)
					note+=song()->currentOctave*12;

				if (note > 119) 
					note = 119;
			}

			if(pMachine==NULL)
			{
				int mgn = song()->seqBus;

				if (mgn < MAX_MACHINES)
				{
					pMachine = song()->machine(mgn);
				}
			}

			for(int i=0;i<song()->tracks();i++)
			{
				if(notetrack[i]==note)
				{
					notetrack[i]=120;
					// build entry
					PatternEvent entry;
					entry.note() = 120;
					entry.instrument() = song()->auxcolSelected;
					entry.machine() = song()->seqBus;
					entry.command() = 0;
					entry.parameter() = 0;	

					// play it

					if (pMachine)
					{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
						pMachine->Tick(i,entry);
#else
						pMachine->Tick(i,&entry);
#endif
					}
				}
			}
		}

		// velocity range 0 -> 127
		void InputHandler::PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine)
		{
			// stop any notes with the same value
			StopNote(note,bTranspose,pMachine);

			if(note<0)
				return;

			// octave offset
			if(note<120)
			{
				if(bTranspose)
					note+=song()->currentOctave*12;

				if (note > 119) 
					note = 119;
			}

			// build entry
			PatternEvent entry;
			entry.note() = note;
			entry.instrument() = song()->auxcolSelected;
			entry.machine() = song()->seqBus;

			if(velocity != 127 && Global::pConfig->midi().velocity().record())
			{
				int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
				if (par > 255) par = 255; else if (par < 0) par = 0;
				switch(Global::pConfig->midi().velocity().type())
				{
					case 0:
						entry.command() = Global::pConfig->midi().velocity().command();
						entry.parameter() = par;
						break;
					case 3:
						entry.instrument() = par;
						break;
				}
			}
			else
			{
				entry.command()=0;
				entry.parameter()=0;
			}

			// play it
			if(pMachine==NULL)
			{
				if (entry.machine() < MAX_MACHINES)
				{
					pMachine = song()->machine(entry.machine());
				}
			}	

			if (pMachine)
			{
				// implement lock sample to machine here.
				// if the current machine is a sampler, check 
				// if current sample is locked to a machine.
				// if so, switch entry.machine() to that machine number
				if (pMachine->_type == MACH_SAMPLER)
				{
					if ((song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index != -1)
						&& (song()->_pInstrument[song()->auxcolSelected]->_locked_to_machine == true))
					{
						entry.machine() = song()->_pInstrument[song()->auxcolSelected]->_locked_machine_index;
						pMachine = song()->machine(entry.machine());
						if ( !pMachine) return;
					}
				}
				// pick a track to play it on	
				if(bMultiKey)
				{
					int i;
					for (i = outtrack+1; i < song()->tracks(); i++)
					{
						if (notetrack[i] == 120)
						{
							break;
						}
					}
					if (i >= song()->tracks())
					{
						for (i = 0; i <= outtrack; i++)
						{
							if (notetrack[i] == 120)
							{
								break;
							}
						}
					}
					outtrack = i;
				}
				else 
				{
					outtrack=0;
				}
				// this should check to see if a note is playing on that track
				if (notetrack[outtrack] < 120)
				{
					StopNote(notetrack[outtrack], bTranspose, pMachine);
				}

				// play
				notetrack[outtrack]=note;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				pMachine->Tick(outtrack,entry);
#else
				pMachine->Tick(outtrack,&entry);
#endif
			}
		}*/

	
	}  // namespace host
}  // namespace psycle


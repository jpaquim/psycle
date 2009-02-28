///\file
///\brief keyboard handler for psycle::host::CChildView, private header
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		void CChildView::KeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			pattern_view_.OnKeyUp(nChar, nRepCnt, nFlags);
		}

		void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// undo code not required, enter note handles it
			KeyUp(nChar, nRepCnt, nFlags);
			CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}

		void CChildView::KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
		{
			// undo code not required, enter not and msbput handle it
			BOOL bRepeat = nFlags&0x4000;
			if(viewMode == view_modes::pattern && pattern_view()->bEditMode)
			{
				bool success = pattern_view_.OnKeyDown(nChar, nRepCnt, nFlags);
				if (success) {
					CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
					return;
				}
			}
			else if (viewMode == view_modes::sequence && pattern_view()->bEditMode)
			{
				bool success;
				// add data
		//		success = Global::pInputHandler->EnterDataSeq(nChar,nFlags);
				success = false;
				if ( success )
				{
					CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
					return;
				}
			}
			else
			{
				pattern_view()->ChordModeOffs = 0;
			}

			// get command
			CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

			if(cmd.IsValid())
			{
				if((cmd.GetType() == CT_Immediate) ||
				(cmd.GetType() == CT_Editor && viewMode == view_modes::pattern) ) 
				{			
					if ( cmd == cdefInfoMachine) {
						if (Global::_pSong->seqBus < MAX_MACHINES)
						{
							if (Global::_pSong->_pMachine[Global::_pSong->seqBus])
							{
								CPoint point;
								point.x = Global::_pSong->_pMachine[Global::_pSong->seqBus]->_x;
								point.y = Global::_pSong->_pMachine[Global::_pSong->seqBus]->_y;
								machine_view_.ShowDialog(Global::_pSong->_pMachine[Global::_pSong->seqBus], point.x, point.y);
							}
						}
					} else
					pattern_view_.PerformCmd(cmd, bRepeat);
				}
				else if (cmd.GetType() == CT_Note && viewMode != view_modes::sequence)
				{
					if(!bRepeat) 
					{	
						const int outnote = cmd.GetNote();
						// play note
						Global::pInputHandler->PlayNote(outnote); 
					}
				}
			}
		}

		void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
		{
			KeyDown(nChar, nRepCnt, nFlags);
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
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

		void CChildView::MidiPatternNote(int outnote, int velocity)
		{
			pattern_view()->MidiPatternNote(outnote, velocity);
		}

		void CChildView::MidiPatternTweak(int command, int value)
		{
			pattern_view()->MidiPatternTweak(command, value);
		}

		void CChildView::MidiPatternTweakSlide(int command, int value)
		{
			pattern_view()->MidiPatternTweakSlide(command, value);
		}

		void CChildView::MidiPatternCommand(int command, int value)
		{
			pattern_view()->MidiPatternCommand(command, value);
		}

		void CChildView::MidiPatternMidiCommand(int command, int value)
		{
			pattern_view()->MidiPatternMidiCommand(command, value);
		}

		void CChildView::MidiPatternInstrument(int value)
		{
			pattern_view()->MidiPatternInstrument(value);
		}

		void CChildView::MousePatternTweak(int machine, int command, int value)
		{
			pattern_view()->MousePatternTweak(machine, command, value);
		}

		void CChildView::MousePatternTweakSlide(int machine, int command, int value)
		{
			pattern_view()->MousePatternTweakSlide(machine, command, value);
		}

		//////////////////////////////////////////////////////////////////////
		// Plays entire row of the pattern

		void CChildView::PlayCurrentRow(void)
		{
			pattern_view()->PlayCurrentRow();
		}

		void CChildView::PlayCurrentNote(void)
		{
			pattern_view()->PlayCurrentNote();
		}


	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

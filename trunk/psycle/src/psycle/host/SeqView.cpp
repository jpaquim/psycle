#include "seqview.hpp"

#include "Configuration.hpp"
#include "Helpers.hpp"
#include "Project.hpp"
#include "Psycle.hpp"
#include "MainFrm.hpp"
#include "Song.hpp"


namespace psycle {
	namespace host {

		BEGIN_MESSAGE_MAP(SequencerView, CDialogBar)
			ON_BN_CLICKED(IDC_INCLEN, OnInclen)
			ON_BN_CLICKED(IDC_DECLEN, OnDeclen)
		END_MESSAGE_MAP()

		SequencerView::SequencerView(CMainFrame* main_frame)
			: main_frame_(main_frame),
	          project_(0)
		{
		}

		SequencerView::~SequencerView()
		{
		}

		void SequencerView::SetProject(Project* project)
		{
			project_ = project;
		}

		void SequencerView::OnInclen() 
		{
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);
			
			if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
			{
				++_pSong->playLength;
				UpdatePlayOrder(false);
				UpdateSequencer();
			}
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnDeclen()
		{
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);
						
			if(_pSong->playLength>1)
			{
				--_pSong->playLength;
				_pSong->playOrder[_pSong->playLength]=0;
				UpdatePlayOrder(false);
				UpdateSequencer();
			}
			main_frame_->SetFocus();	
		}


		void SequencerView::UpdateSequencer(int selectedpos)
		{
			if (!project_)
				return;

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			char buf[16];

			int top = cc->GetTopIndex();
			cc->ResetContent();
			
			if (Global::pConfig->_bShowPatternNames)
			{
				for(int n=0;n< project_->song().playLength;n++)
				{
					sprintf(buf,"%.2X:%s",n, project_->song().patternName[project_->song().playOrder[n]]);
					cc->AddString(buf);
				}
			}
			else
			{
				for(int n=0;n<project_->song().playLength;n++)
				{
					sprintf(buf,"%.2X: %.2X",n,project_->song().playOrder[n]);
					cc->AddString(buf);
				}
			}
			
			cc->SelItemRange(false,0,cc->GetCount()-1);
			for (int i=0; i<MAX_SONG_POSITIONS;i++)
			{
				if ( project_->song().playOrderSel[i]) cc->SetSel(i,true);
			}
			if (selectedpos >= 0)
			{
				cc->SetSel(selectedpos);
				top = selectedpos - 0xC;
				if (top < 0) top = 0;
			}
			cc->SetTopIndex(top);
			main_frame_->StatusBarIdle();
		}


		void SequencerView::UpdatePlayOrder(bool mode)
		{
			if (!project_)
				return;

			CStatic *ll_l=(CStatic *)GetDlgItem(IDC_SEQ3);
			CListBox *pls=(CListBox*)GetDlgItem(IDC_SEQLIST);
			CStatic *pLength = (CStatic*)GetDlgItem(IDC_LENGTH);
			Song* _pSong = &project_->song();

			int ll = _pSong->playLength;

			char buffer[16];

		// Update Labels
			
			sprintf(buffer,"%.2X",ll);
			ll_l->SetWindowText(buffer);

			/*
			int songLength = 0;
			for (int i=0; i <ll; i++)
			{
				int pattern = _pSong->playOrder[i];
				// this should parse each line for ffxx commands if you want it to be truly accurate
				songLength += (_pSong->patternLines[pattern] * 60/(_pSong->BeatsPerMin() * _pSong->_LinesPerBeat));
			}

			sprintf(buffer, "%02d:%02d", songLength / 60, songLength % 60);
			*/

			// take ff and fe commands into account

			float songLength = 0;
			int bpm = _pSong->BeatsPerMin();
			int tpb = _pSong->LinesPerBeat();
			for (int i=0; i <ll; i++)
			{
				int pattern = _pSong->playOrder[i];
				// this should parse each line for ffxx commands if you want it to be truly accurate
				unsigned char* const plineOffset = _pSong->_ppattern(pattern);
				for (int l = 0; l < _pSong->patternLines[pattern]*MULTIPLY; l+=MULTIPLY)
				{
					for (int t = 0; t < _pSong->SONGTRACKS*EVENT_SIZE; t+=EVENT_SIZE)
					{
						PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
						switch (pEntry->_cmd)
						{
						case 0xFF:
							if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
							{
								bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
							}
							break;
							
						case 0xFE:
							if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
							{
								tpb=pEntry->_parameter;
							}
							break;
						}
					}
					songLength += (60.0f/(bpm * tpb));
				}
			}
			
			sprintf(buffer, "%02d:%02d", helpers::math::truncated(songLength / 60), helpers::math::truncated(songLength) % 60);
			pLength->SetWindowText(buffer);
			
			// Update sequencer line
			
			if (mode)
			{
				const int ls= main_frame_->m_wndView.pattern_view()->editPosition;
				const int le=_pSong->playOrder[ls];
				pls->DeleteString(ls);

				if (Global::pConfig->_bShowPatternNames)
					sprintf(buffer,"%.2X:%s",ls,_pSong->patternName[le]);
				else
					sprintf(buffer,"%.2X: %.2X",ls,le);
				pls->InsertString(ls,buffer);
				// Update sequencer selection	
				pls->SelItemRange(false,0,pls->GetCount()-1);
				pls->SetSel(ls,true);
				int top = ls - 0xC;
				if (top < 0) top = 0;
				pls->SetTopIndex(top);
				memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
				_pSong->playOrderSel[ls] = true;
			}
			else
			{
				int top = pls->GetTopIndex();
				pls->SelItemRange(false,0,pls->GetCount()-1);
				for (int i=0;i<MAX_SONG_POSITIONS;i++ )
				{
					if (_pSong->playOrderSel[i]) pls->SetSel(i,true);
				}
				pls->SetTopIndex(top);
			}
			
		}

	}
}

#include "seqview.hpp"

#include "Configuration.hpp"
#include "Helpers.hpp"
#include "Player.hpp"
#include "Project.hpp"
#include "Psycle.hpp"
#include "MainFrm.hpp"
#include "Song.hpp"


namespace psycle {
	namespace host {


		SequencerView::SequencerView(CMainFrame* main_frame)
			: main_frame_(main_frame),
	          project_(0),
			  seqcopybufferlength(0)
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
			if (!project_)
				return;
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
			if (!project_)
				return;
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

		void SequencerView::OnIncshort() 
		{			
			if (!project_)
				return;
			int indexes[MAX_SONG_POSITIONS];
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

			for (int i = 0; i < num; i++)
			{
				if(_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-1))
				{
					_pSong->playOrder[indexes[i]]++;
				}
			}
			UpdatePlayOrder(false);
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSelchangeSeqlist() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int maxitems=cc->GetCount();
			int const ep=cc->GetCurSel();
			if(pat_view->editPosition<0) pat_view->editPosition = 0; // DAN FIXME
			int const cpid=_pSong->playOrder[pat_view->editPosition];

			memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
			for (int c=0;c<maxitems;c++) 
			{
				if ( cc->GetSel(c) != 0) _pSong->playOrderSel[c]=true;
			}
			
			if((ep!=pat_view->editPosition))// && ( cc->GetSelCount() == 1))
			{
				if ((Global::pPlayer->_playing) && (Global::pConfig->_followSong))
				{
					bool b = Global::pPlayer->_playBlock;
					Global::pPlayer->Start(ep,0,false);
					Global::pPlayer->_playBlock = b;
				}
				pat_view->editPosition=ep;
				pat_view->prevEditPosition=ep;
				UpdatePlayOrder(false);
				
				if(cpid!=_pSong->playOrder[ep])
				{
					main_frame_->m_wndView.Repaint(draw_modes::pattern);
					if (Global::pPlayer->_playing) {
						main_frame_->m_wndView.Repaint(draw_modes::playback);
					}
				}		
			}
			main_frame_->StatusBarIdle();
			main_frame_->SetFocus();
		}

		void SequencerView::OnDblclkSeqlist() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
		//
		//	CListBox *cc=(CListBox *)m_wndSeq.GetDlgItem(IDC_SEQLIST);
		//	int const ep=_pSong->GetBlankPatternUnsed();
		//	int const sep=m_wndView.editPosition;
			
		//	if(ep!=_pSong->playOrder[sep])
		//	{
		//		_pSong->playOrder[sep]=ep;
		//		UpdatePlayOrder(true);
		//		m_wndView.Repaint(draw_modes::pattern);
		//	}
		//	m_wndView.SetFocus();
		//	
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const ep=cc->GetCurSel();
			if (Global::pPlayer->_playing)
			{
				bool b = Global::pPlayer->_playBlock;
				Global::pPlayer->Start(ep,0);
				Global::pPlayer->_playBlock = b;
			}
			else
			{
				Global::pPlayer->Start(ep,0);
			}
			pat_view->editPosition=ep;
			//Following two lines by alk to disable view change to pattern mode when 
			//double clicking on a pattern in the sequencer list
			//m_wndView.OnPatternView();
			SetFocus();
		}


		void SequencerView::OnDecshort() 
		{		
			if (!project_)
				return;
			int indexes[MAX_SONG_POSITIONS];
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);


			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

			for (int i = 0; i < num; i++)
			{
				if(_pSong->playOrder[indexes[i]]>0)
				{
					_pSong->playOrder[indexes[i]]--;
				}
			}
			UpdatePlayOrder(false);
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnInclong() 
		{			
			if (!project_)
				return;
			int indexes[MAX_SONG_POSITIONS];
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

			for (int i = 0; i < num; i++)
			{
				if(_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-16))
				{
					_pSong->playOrder[indexes[i]]+=16;			
				}
				else
				{
					_pSong->playOrder[indexes[i]]=(MAX_PATTERNS-1);
				}
			}
			UpdatePlayOrder(false);
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::OnDeclong() 
		{  
			if (!project_)
				return;
			int indexes[MAX_SONG_POSITIONS];
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);


			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

			for (int i = 0; i < num; i++)
			{
				if(_pSong->playOrder[indexes[i]]>=16)
				{
					_pSong->playOrder[indexes[i]]-=16;			
				}
				else
				{
					_pSong->playOrder[indexes[i]]=0;
				}
			}
			UpdatePlayOrder(false);
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqnew() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
			{
				PatternView* pat_view = project_->pat_view();
				Song* _pSong = &project_->song();
				pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

				++_pSong->playLength;

				pat_view->editPosition++;
				int const pop=pat_view->editPosition;
				for(int c=(_pSong->playLength-1);c>=pop;c--)
				{
					_pSong->playOrder[c]=_pSong->playOrder[c-1];
				}
				_pSong->playOrder[pat_view->editPosition]=_pSong->GetBlankPatternUnused();
				
				if ( _pSong->playOrder[pat_view->editPosition]>= MAX_PATTERNS )
				{
					_pSong->playOrder[pat_view->editPosition]=MAX_PATTERNS-1;
				}

				_pSong->AllocNewPattern(_pSong->playOrder[pat_view->editPosition],"",Global::pConfig->defaultPatLines,FALSE);

				UpdatePlayOrder(true);
				UpdateSequencer(pat_view->editPosition);

				main_frame_->m_wndView.Repaint(draw_modes::pattern);
			}
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqins() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
				Song* _pSong = &project_->song();
			if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
			{
				PatternView* pat_view = project_->pat_view();
				Song* _pSong = &project_->song();
				pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

				++_pSong->playLength;

				pat_view->editPosition++;
				int const pop=pat_view->editPosition;
				for(int c=(_pSong->playLength-1);c>=pop;c--)
				{
					_pSong->playOrder[c]=_pSong->playOrder[c-1];
				}

				UpdatePlayOrder(true);
				UpdateSequencer(pat_view->editPosition);

				main_frame_->m_wndView.Repaint(draw_modes::pattern);
			}
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqduplicate() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int selcount = cc->GetSelCount();
			if (selcount == 0) return;
			if ( _pSong->playLength+selcount >= MAX_SONG_POSITIONS)
			{
				MessageBox("Cannot clone the pattern(s). The maximum sequence length would be exceeded.","Clone Patterns");
				main_frame_->m_wndView.SetFocus();
				return;
			}			
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

			// Moves all patterns after the selection, to make space.
			int* litems = new int[selcount];
			cc->GetSelItems(selcount,litems);
			for(int i(_pSong->playLength-1) ; i >= litems[selcount-1] ;--i)
			{
				_pSong->playOrder[i+selcount]=_pSong->playOrder[i];
			}
			_pSong->playLength+=selcount;

			for(int i(0) ; i < selcount ; ++i)
			{
				int newpat = -1;
				// This for loop is in order to clone sequences like: 00 00 01 01 and avoid duplication of same patterns.
				for (int j(0); j < i; ++j)
				{
					if (_pSong->playOrder[litems[0]+j] == _pSong->playOrder[litems[0]+i])
					{
						newpat=_pSong->playOrder[litems[selcount-1]+j+1];
					}
				}
				if (newpat == -1 ) 
				{
					newpat = _pSong->GetBlankPatternUnused();
					if (newpat < MAX_PATTERNS-1)
					{
						int oldpat = _pSong->playOrder[litems[i]];
						_pSong->AllocNewPattern(newpat,_pSong->patternName[oldpat],_pSong->patternLines[oldpat],FALSE);
						memcpy(_pSong->_ppattern(newpat),_pSong->_ppattern(oldpat),MULTIPLY2);
					}
					else 
					{
						newpat=0;
					}
				}
				_pSong->playOrder[litems[selcount-1]+i+1]=newpat;
			}
			pat_view->editPosition=litems[selcount-1]+1;
			UpdatePlayOrder(true);
			UpdateSequencer(pat_view->editPosition);
			main_frame_->m_wndView.Repaint(draw_modes::pattern);

			delete [] litems; litems = 0;
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqcut() 
		{
			if (!project_)
				return;
			OnSeqcopy();
			OnSeqdelete();
		}

		void SequencerView::OnSeqdelete() 
		{
			if (!project_)
				return;
			int indexes[MAX_SONG_POSITIONS];
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,indexes);

			// our list can be in any order so we must be careful
			int smallest = indexes[0]; // we need a good place to put the cursor when we are done, above the topmost selection seems most intuitive
			for (int i=0; i < num; i++)
			{
				int c;
				for(c = indexes[i] ; c < _pSong->playLength - 1 ; ++c)
				{
					_pSong->playOrder[c]=_pSong->playOrder[c+1];
				}
				_pSong->playOrder[c]=0;
				_pSong->playLength--;
				if (_pSong->playLength <= 0)
				{
					_pSong->playLength =1;
				}
				for(int j(i + 1) ; j < num ; ++j)
				{
					if (indexes[j] > indexes[i])
					{
						indexes[j]--;
					}
				}
				if (indexes[i] < smallest)
				{
					smallest = indexes[i];
				}
			}
			pat_view->editPosition = smallest-1;

			if (pat_view->editPosition<0)
			{
				pat_view->editPosition = 0;
			}
			else if (pat_view->editPosition>=_pSong->playLength)
			{
				pat_view->editPosition=_pSong->playLength-1;
			}

			UpdatePlayOrder(true);
			UpdateSequencer(pat_view->editPosition);
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqcopy() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			seqcopybufferlength= cc->GetSelCount();
			cc->GetSelItems(MAX_SONG_POSITIONS,seqcopybuffer);

			// sort our table so we can paste it in a sensible manner later
			for (int i=0; i < seqcopybufferlength; i++)
			{
				for (int j=i+1; j < seqcopybufferlength; j++)
				{
					if (seqcopybuffer[j] < seqcopybuffer[i])
					{
						int k = seqcopybuffer[i];
						seqcopybuffer[i] = seqcopybuffer[j];
						seqcopybuffer[j] = k;
					}
				}
				// convert to actual index
				seqcopybuffer[i] = _pSong->playOrder[seqcopybuffer[i]];
			}
		}

		void SequencerView::OnSeqpaste() 
		{		
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			if (seqcopybufferlength > 0)
			{
				if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
				{					
					pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);					

					// we will do this in a loop to easily handle an error if we run out of space

					// our list can be in any order so we must be careful
					int pastedcount = 0;
					for (int i=0; i < seqcopybufferlength; i++)
					{
						if(_pSong->playLength<(MAX_SONG_POSITIONS-1))
						{
							++_pSong->playLength;

							pat_view->editPosition++;
							pastedcount++;
							int c;
							for(c = _pSong->playLength - 1 ; c >= pat_view->editPosition ; --c)
							{
								_pSong->playOrder[c]=_pSong->playOrder[c-1];
							}
							_pSong->playOrder[c+1] = seqcopybuffer[i];
						}
					}

					if (pastedcount>0)
					{
						UpdatePlayOrder(true);
						for(int i(pat_view->editPosition + 1 - pastedcount) ; i < pat_view->editPosition ; ++i)
						{
							_pSong->playOrderSel[i] = true;
						}
						UpdateSequencer(pat_view->editPosition);
						main_frame_->m_wndView.Repaint(draw_modes::pattern);
					}
				}
			}
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqclr() 
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			if (MessageBox("Do you really want to clear the sequence and pattern data?","Sequencer",MB_YESNO) == IDYES)
			{
				pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);
				
				// clear sequence
				for(int c=0;c<MAX_SONG_POSITIONS;c++)
				{
					_pSong->playOrder[c]=0;
				}
				// clear pattern data
				_pSong->DeleteAllPatterns();
				// init a pattern for #0
				_pSong->_ppattern(0);

				pat_view->editPosition=0;
				_pSong->playLength=1;
				UpdatePlayOrder(true);
				UpdateSequencer();
				main_frame_->m_wndView.Repaint(draw_modes::pattern);
			}
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::OnSeqsort()
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();				
			pat_view->AddUndoSequence(_pSong->playLength,
									  pat_view->editcur.track,
									  pat_view->editcur.line,
									  pat_view->editcur.col,
									  pat_view->editPosition);
				

			unsigned char oldtonew[MAX_PATTERNS];
			unsigned char newtoold[MAX_PATTERNS];
			memset(oldtonew,255,MAX_PATTERNS*sizeof(char));
			memset(newtoold,255,MAX_PATTERNS*sizeof(char));

			if (Global::pPlayer->_playing)
			{
				Global::pPlayer->Stop();
			}


		// Part one, Read patterns from sequence and assign them a new ordered number.
			unsigned unsigned char freep=0;
			for ( int i=0 ; i<_pSong->playLength ; i++ )
			{
				const unsigned char cp=_pSong->playOrder[i];
				if ( oldtonew[cp] == 255 ) // else, we have processed it already
				{
					oldtonew[cp]=freep;
					newtoold[freep]=cp;
					freep++;
				}
			}
		// Part one and a half. End filling the order numbers.
			for(int i(0) ; i < MAX_PATTERNS ; ++i)
			{
				if ( oldtonew[i] == 255 )
				{
					oldtonew[i] = freep;
					newtoold[freep] = i;
					freep++;
				}
			}
		// Part two. Sort Patterns. Take first "invalid" out, and start putting patterns in their place.
		//			 When we have to put the first read one back, do it and find next candidate.

			int patl; // first one is initial one, next one is temp one
			char patn[32]; // ""
			unsigned char * pData; // ""


			int idx=0;
			int idx2=0;
			for(int i(0) ; i < MAX_PATTERNS ; ++i)
			{
				if ( newtoold[i] != i ) // check if this place belongs to another pattern
				{
					pData = _pSong->ppPatternData[i];
					memcpy(&patl,&_pSong->patternLines[i],sizeof(int));
					memcpy(patn,&_pSong->patternName[i],sizeof(char)*32);

					idx = i;
					while ( newtoold[idx] != i ) // Start moving patterns while it is not the stored one.
					{
						idx2 = newtoold[idx]; // get pattern that goes here and move.

						_pSong->ppPatternData[idx] = _pSong->ppPatternData[idx2];
						memcpy(&_pSong->patternLines[idx],&_pSong->patternLines[idx2],sizeof(int));
						memcpy(&_pSong->patternName[idx],&_pSong->patternName[idx2],sizeof(char)*32);
						
						newtoold[idx]=idx; // and indicate that this pattern has been corrected.
						idx = idx2;
					}

					// Put pattern back.
					_pSong->ppPatternData[idx] = pData;
					memcpy(&_pSong->patternLines[idx],&patl,sizeof(int));
					memcpy(_pSong->patternName[idx],patn,sizeof(char)*32);

					newtoold[idx]=idx; // and indicate that this pattern has been corrected.
				}
			}
		// Part three. Update the sequence

			for(int i(0) ; i < _pSong->playLength ; ++i)
			{
				_pSong->playOrder[i]=oldtonew[_pSong->playOrder[i]];
			}

		// Part four. All the needed things.

			seqcopybufferlength = 0;
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}


		void SequencerView::OnSeqShowpattername()
		{
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			Global::pConfig->_bShowPatternNames=((CButton*)GetDlgItem(IDC_SHOWPATTERNAME))->GetCheck();
			
			//

			//trying to set the size of the sequencer bar... how to do this!?

			//CRect borders;
			//((CDialog*)m_wndSeq.GetDlgItem(AFX_IDW_DIALOGBAR))->GetWindowRect(&borders);
			//TRACE("borders.right = %i", borders.right);
			//if (Global::pConfig->_bShowPatternNames)
			//{
              // //m_wndSeq.SetBorders(borders.left, borders.top, 6, borders.bottom);
			//}
			//else
			//{
			//	//m_wndSeq.SetBorders(borders.left, borders.top, 3, borders.bottom);
			//}
			//

			UpdateSequencer();
			CListBox *pls=(CListBox*)GetDlgItem(IDC_SEQLIST);
			pls->SetSel(Global::pPlayer->_playPosition,true);

			int top = ((Global::pPlayer->_playing)?Global::pPlayer->_playPosition:pat_view->editPosition) - 0xC;
			if (top < 0) top = 0;
			pls->SetTopIndex(top);
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::UpdateSequencer(int selectedpos)
		{
			if (!project_)
				return;


			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			char buf[16];

			int top = cc->GetTopIndex();
			cc->ResetContent();
			
#ifdef use_psycore
			psy::core::PatternSequence* sequence = &project_->psy_song().patternSequence();
			psy::core::SequenceLine* line = *(sequence->begin());
			// Iterate the sequence entries and add them.
			psy::core::SequenceLine::iterator it = line->begin();
			for(int n = 0 ; it != line->end(); ++it, ++n) {
				psy::core::SequenceEntry* entry = it->second;
				if (Global::pConfig->_bShowPatternNames) {
					sprintf(buf,"%.2X:%s",n, entry->pattern()->name().c_str());
				} else {
					sprintf(buf,"%.2X: %.2X",n,entry->pattern()->id());
				}
				cc->AddString(buf);
			}	
			cc->SelItemRange(false,0,cc->GetCount()-1);
			if (selectedpos >= 0)
			{
				cc->SetSel(selectedpos);
				top = selectedpos - 0xC;
				if (top < 0) top = 0;
			}
			cc->SetTopIndex(top);
			main_frame_->StatusBarIdle();
#else

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
#endif
		}


		void SequencerView::UpdatePlayOrder(bool mode)
		{
			if (!project_)
				return;
#ifdef use_psycore
#else

			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();

			CStatic *ll_l=(CStatic *)GetDlgItem(IDC_SEQ3);
			CListBox *pls=(CListBox*)GetDlgItem(IDC_SEQLIST);
			CStatic *pLength = (CStatic*)GetDlgItem(IDC_LENGTH);			

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
				const int ls= pat_view->editPosition;
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

#endif
		}


	} // namespace host
} // namespace psycle

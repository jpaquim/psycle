// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include "seqview.hpp"

#include <psycle/helpers/math.hpp>

#include <psycle/core/player.h>
#include <psycle/core/song.h>

#include "Configuration.hpp"
#include "MainFrm.hpp"
#include "PatternView.hpp"
#include "Project.hpp"
#include "SeqDeleteCommand.hpp"

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

		SequencerView::SequencerView(CMainFrame* main_frame)
			: main_frame_(main_frame),
			  project_(0),
			  seqcopybufferlength(0),
			  selected_entry_(0),
			  selblock_play_(false) {
			seq_sel_play_line_ = new psycle::core::SequenceLine();
		}

		SequencerView::~SequencerView() {
			delete seq_sel_play_line_;
		}
		
		void SequencerView::OnSeqcut() {
			if (!project_)
				return;

			OnSeqcopy();
			project_->cmd_manager()->ExecuteCommand(new SeqDeleteCommand(this));
		}

		SequenceLine* SequencerView::ComputeSelblockLine() {
			seq_sel_play_line_->clear();
			sel_pos_map_.clear();
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
				SequenceEntry* old_entry = GetEntry(*it);				
				SequenceEntry* entry = new SequenceEntry(seq_sel_play_line_);
				entry->setPattern(old_entry->pattern());				
				seq_sel_play_line_->insert(old_entry->tickPosition(), entry);
				sel_pos_map_[entry] = *it;				
			}
			seq_sel_play_line_->removeSpaces();
			return seq_sel_play_line_;
		}

		void SequencerView::SwitchToSelBlockPlay() {
			psycle::core::Sequence& seq = project_->song().sequence();
			seq_main_play_line_ = *(seq.begin()+1);
			seq_sel_play_line_ = ComputeSelblockLine();
			*(seq.begin()+1) = seq_sel_play_line_;
			seq_sel_play_line_->SetSequence(&project_->song().sequence());
			selblock_play_= true;
		}

		void SequencerView::SwitchToNormalPlay() {
			if (selblock_play_) {
				psycle::core::Sequence& seq = project_->song().sequence();
				*(seq.begin()+1) = seq_main_play_line_;
				selblock_play_= false;
			}
		}


		SequenceEntry* SequencerView::GetEntry(int list_position) {
			std::map<int,SequenceEntry*>::iterator it;
			it = pos_map_.find(list_position);
			assert(it != pos_map_.end());
			return it->second;
		}

		void SequencerView::SetEntry(psycle::core::SequenceEntry* entry) {
			int pos = -1;
			if (!selblock_play_) {
				std::map<int,SequenceEntry*>::iterator it = pos_map_.begin();
				for ( ; it != pos_map_.end(); ++it) {
					if (it->second == entry) {
						pos = it->first;
						break;
					}
				}
			} else {
				std::map<SequenceEntry*, int>::iterator it;
				it = sel_pos_map_.find(entry);
				if ( it != sel_pos_map_.end() )
					pos = it->second;
			}
			assert(pos != -1);

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			cc->SelItemRange(false,0,cc->GetCount());
			cc->SetSel(pos,true);
			int top = pos - 0xC;
			if (top < 0) top = 0;
			cc->SetTopIndex(top);
			PatternView* pat_view = project_->pat_view();
			selected_entry_ = entry;
			pat_view->SetPattern(entry->pattern());			
			BuildSelectionList();
			SelectItems();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
			selected_entry_ = entry;
		} 

		void SequencerView::UpdateSequencer(int selectedpos) {
			if (!project_)
				return;

			BuildPositionMap();
			BuildListBox();
			if (selection_.empty() && pos_map_.size() > 0) {
				selection_.push_back(0);
				selected_entry_ = GetEntry(0);
				project_->pat_view()->SetPattern(selected_entry_->pattern());
			}
			SelectItems();

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int sel_idx = cc->GetCurSel();
			selected_entry_ = GetEntry(sel_idx);
			main_frame_->m_wndView.pattern_view()->SetPattern(selected_entry_->pattern());
		}

		void SequencerView::BuildPositionMap() {
			if (!project_)
				return;
			pos_map_.clear();
			psycle::core::Sequence* sequence = &project_->song().sequence();
			psycle::core::SequenceLine* line = *(sequence->begin()+1);	
			psycle::core::SequenceLine::iterator sit = line->begin();
			for (int pos = 0; sit != line->end(); ++sit, ++pos) {
				pos_map_[pos] = sit->second;
			}
		}

		void SequencerView::BuildSelectionList() {			
			selection_.clear();
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int const num= cc->GetSelCount();
			int* indexes = new int[num];
			cc->GetSelItems(num,indexes);
			for (int i = 0; i < num; ++i) {
			  int idx = indexes[i];
		  	  selection_.push_back(idx);
			}
			delete[] indexes;			
		}

		void SequencerView::BuildCopyList() {
			copy_list_.clear();
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
				copy_list_.push_back(GetEntry(*it)->pattern());
			}
		}

		void SequencerView::BuildListBox() {
			if (!project_)
				return;
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			char buf[16];
			int top = cc->GetTopIndex();
			cc->ResetContent();
			std::map<int,SequenceEntry*>::iterator it = pos_map_.begin();
			for(int n = 0 ; it != pos_map_.end(); ++it, ++n) {
				SequenceEntry* entry = it->second;
				if (Global::pConfig->_bShowPatternNames) {
					sprintf(buf,"%.2X:%s",n, entry->pattern()->name().c_str());
				} else {
					sprintf(buf,"%.2X: %.2X", n, entry->pattern()->id());
				}
				cc->AddString(buf);
			}
		}

		void SequencerView::SelectItems() {
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			cc->SelItemRange(false, 0, cc->GetCount() - 1);
			std::vector<int>::iterator it = selection_.begin();
			for ( ;it != selection_.end(); ++it) {
				cc->SetSel(*it, true);
			}						
		}

		void SequencerView::OnInclen() {
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			Sequence* sequence = &_pSong->sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			// if(len<(MAX_SONG_POSITIONS-1)) limit it or not ?
			{
				Pattern* pattern = sequence->FindPattern(0);
				assert(pattern);
				double beat_pos = line->tickLength();
				line->createEntry(pattern, beat_pos);				
				UpdateSequencer();				
			}
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnDeclen() {
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			if(len>1) {
				SequenceLine::reverse_iterator r = line->rbegin();
				++r;
				line->erase(r.base());
				UpdatePlayOrder(false);
				UpdateSequencer();				
			}
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnIncshort() {			
			if (!project_)
				return;

			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
			  SequenceEntry* item = pos_map_[*it];
			  int id = item->pattern()->id()+1;
			  Pattern* pattern = sequence->FindPattern(id);
			  if (pattern) {				  
				  item->setPattern(pattern);
			  } else {
				  psycle::core::Pattern* pattern = new psycle::core::Pattern();
				  pattern->setID(id);
				  sequence->Add(pattern);
				  item->setPattern(pattern);				  
			  }			  
			}			
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnDecshort() {		
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
			  SequenceEntry* item = pos_map_[*it];			
			  int id = item->pattern()->id()-1;
			  if (id >= 0) {
				Pattern* pattern = sequence->FindPattern(id);
				if (pattern) {				  
				  item->setPattern(pattern);
				} else {
				  pattern = new psycle::core::Pattern();
				  pattern->setID(id);
				  sequence->Add(pattern);
				  item->setPattern(pattern);
				}	  			
			  }
			}			
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnInclong() {			
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
			  SequenceEntry* item = pos_map_[*it];
			  int id = item->pattern()->id()+16;
			  Pattern* pattern = sequence->FindPattern(id);
			  if (pattern) {				  
				  item->setPattern(pattern);
			  } else {
				  pattern = new psycle::core::Pattern();
				  pattern->setID(id);
				  sequence->Add(pattern);
				  item->setPattern(pattern);
			  }
			}
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::OnDeclong() {  
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int len = line->size(); // Length, in patterns, of the sequence.
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it) {
			  SequenceEntry* item = pos_map_[*it];
			  int id = std::max(0, item->pattern()->id()-16);
			  if (id >= 0) {
				  Pattern* pattern = sequence->FindPattern(id);
				if (pattern) {				  
				  item->setPattern(pattern);
				} else {
				  pattern = new psycle::core::Pattern();
				  pattern->setID(id);
				  sequence->Add(pattern);
				  item->setPattern(pattern);
				}	  			
			  }
			}
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqnew() {
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);
			int id = 0;
			for ( ; sequence->FindPattern(id) !=0 ; ++id);
			psycle::core::Pattern* pattern = new psycle::core::Pattern();
			pattern->timeSignatures().clear();
			pattern->timeSignatures().push_back(psycle::core::TimeSignature(16.0));
			pattern->setID(id);
			sequence->Add(pattern);
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
			entry->setPattern(pattern);
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int sel_idx = cc->GetCurSel();
			line->insertEntryAndMoveRest(entry, GetEntry(sel_idx)->tickEndPosition());
			BuildPositionMap();
			BuildListBox();
			selection_.clear();
			selection_.push_back(sel_idx+1);
			SelectItems();
			selected_entry_ = entry;
			main_frame_->m_wndView.pattern_view()->SetPattern(entry->pattern());
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqins() {
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);			
			psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);			
			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int sel_idx = cc->GetCurSel();
			entry->setPattern(GetEntry(sel_idx)->pattern());
			line->insertEntryAndMoveRest(entry, GetEntry(sel_idx)->tickEndPosition());
			BuildPositionMap();
			BuildListBox();
			selection_.clear();
			selection_.push_back(sel_idx+1);
			SelectItems();
			main_frame_->m_wndView.pattern_view()->SetPattern(entry->pattern());
			selected_entry_ = entry;
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
		}


		void SequencerView::OnSeqduplicate() {
			if (!project_)
				return;
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);			
			std::sort(selection_.begin(), selection_.end());
			if (selection_.size() == 0)
				return;
			int max = selection_.back();
			int ins = max;
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it ) {
				int id = 0;
				for ( ; sequence->FindPattern(id) !=0 ; ++id);				
				int sel_idx = *it;
				psycle::core::Pattern* src_pattern = GetEntry(sel_idx)->pattern();
				psycle::core::Pattern* pattern = new psycle::core::Pattern(*src_pattern);
				pattern->setID(id);
				sequence->Add(pattern);
				psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
				entry->setPattern(pattern);			
				line->insertEntryAndMoveRest(entry, GetEntry(max)->tickEndPosition());
				BuildPositionMap();
				++max;
			}
			BuildPositionMap();
			BuildListBox();
			selection_.clear();
			selection_.push_back(ins+1);
			SelectItems();
			selected_entry_ = GetEntry(ins+1);
			main_frame_->m_wndView.pattern_view()->SetPattern(selected_entry_->pattern());
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqdelete() {
			if (!project_)
				return;
	
			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			if (GetEntry(cc->GetCurSel())->pattern()->id() == 0 && line->size() == 1)
				return;

			int min = selection_.front();
			std::vector<int>::iterator it = selection_.begin();
			for ( ; it != selection_.end(); ++it ) {
				if (*it < min)
					min = *it;
				psycle::core::SequenceEntry* entry = GetEntry(*it);
				line->removeEntry(entry);
			}
			line->removeSpaces();
			if ( line->empty() ) {
				Pattern* pattern = sequence->FindPattern(0);
				assert(pattern);
				double beat_pos = line->tickLength();
				line->createEntry(pattern, 0);
				selection_.clear();
				selection_.push_back(0);
			} else {
				selection_.clear();
				selection_.push_back(std::max(0,min-1));
			}

			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnSeqcopy() {
			if (!project_)
				return;

			BuildCopyList();
		}

		void SequencerView::OnSeqpaste() {		
			if (!project_)
				return;

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			int ins = cc->GetCurSel();
			int max = ins;

			PatternView* pat_view = project_->pat_view();
			Sequence* sequence = &project_->song().sequence();
			SequenceLine* line = *(sequence->begin()+1);

			std::vector<psycle::core::Pattern*>::iterator it = copy_list_.begin();
			for ( ; it != copy_list_.end(); ++it ) {					
				psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
				entry->setPattern(*it);			
				line->insertEntryAndMoveRest(entry, GetEntry(max)->tickEndPosition());
				BuildPositionMap();
				++max;
			}
			UpdateSequencer();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::OnSeqclr() {
			if (!project_)
				return;

			PatternView* pat_view = project_->pat_view();
			Song* _pSong = &project_->song();
			if (MessageBox("Do you really want to clear the sequence and pattern data?","Sequencer",MB_YESNO) == IDYES)
			{
				_pSong->sequence().removeAll();
				Pattern* pattr = new Pattern();
				pattr->setID(0);
				_pSong->sequence().Add(pattr);
				SequenceLine* line = _pSong->sequence().createNewLine();
				line->createEntry(pattr,0);
				selection_.clear();
				selection_.push_back(0);
				UpdateSequencer();
				selected_entry_ = GetEntry(0);
				pat_view->SetPattern(selected_entry_->pattern());
				project_->cmd_manager()->Clear(); // no undo available
				main_frame_->m_wndView.Repaint(draw_modes::pattern);
			}
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::OnSeqsort() {
			// todo

			/*SequenceLine* line = *_pSong->sequence().begin()+1;
			SequenceLine::iterator ite = line->begin();
			for(int i(0) ; ite != line->end(); ++ite, ++i)
			{
				if ( newtoold[i] != i ) // check if this place belongs to another pattern
				{
					ite->second->pattern()->set_name_as_index(newtoold[i]);
				}
			}*/
		}

		void SequencerView::OnSelchangeSeqlist() {
			if (!project_)
				return;			

			CListBox *cc=(CListBox *)GetDlgItem(IDC_SEQLIST);
			PatternView* pat_view = project_->pat_view();
			selected_entry_ = GetEntry(cc->GetCurSel());
			pat_view->SetPattern(selected_entry_->pattern());			
			BuildSelectionList();
			SelectItems();
			main_frame_->m_wndView.Repaint(draw_modes::pattern);
			main_frame_->StatusBarIdle();
			main_frame_->m_wndView.SetFocus();
		}

		void SequencerView::OnDblclkSeqlist() {
		}


/*	TODO
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

			if (Global::pPlayer->playing())
			{
				Global::pPlayer->stop();
			}


		// Part one, Read patterns from sequence and assign them a new ordered number.
			unsigned char freep=0;
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
*/


		void SequencerView::OnSeqShowpattername() {
			if (!project_)
				return;
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
			main_frame_->m_wndView.SetFocus();			
		}

		void SequencerView::UpdatePlayOrder(bool mode) {
			if (!project_)
				return;

			CStatic *ll_l=(CStatic *)GetDlgItem(IDC_SEQ3);
			CListBox *pls=(CListBox*)GetDlgItem(IDC_SEQLIST);
			CStatic *pLength = (CStatic*)GetDlgItem(IDC_LENGTH);			

			char buffer[16];
			Song* _pSong = &project_->song();


			Sequence& sequence = _pSong->sequence();
			SequenceLine* line = *(sequence.begin()+1);
			int ll = line->size();

			// Update Labels
			
			sprintf(buffer,"%.2X",ll);
			ll_l->SetWindowText(buffer);

			float songLength = 0;
			songLength = line->tickLength()/_pSong->BeatsPerMin();		
			sprintf(buffer, "%02d:%02d", static_cast<int>(songLength / 60), static_cast<int>(songLength) % 60);
			pLength->SetWindowText(buffer);
			// Update sequencer line
			
			if (mode)
			{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				//todo
#else
				PatternView* pat_view = project_->pat_view();
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
#endif
			}
			else {
				int top = pls->GetTopIndex();
				std::vector<int>::iterator sel_it = selection_.begin();
				for ( ; sel_it != selection_.end(); ++sel_it) {
					pls->SetSel(*sel_it,true);
				}
				pls->SetTopIndex(top);
			}
		}

		void SequencerView::IncCurPattern() {
			OnIncshort();
		}

		void SequencerView::DecCurPattern() {
			OnDecshort();
		}

		void SequencerView::IncPosition(bool bRepeat) {
		//	case cdefPlaySkipAhead:
			Song* song_ = &project_->song();
			PatternView* pat_view = project_->pat_view();
			if (Global::pPlayer->playing() && Global::pConfig->_followSong)
			{
				PlayerTimeInfo& tinfo = Player::singleton().timeInfo();
				if (tinfo.playBeatPos() < song_->sequence().tickLength() ) {
					///todo: Add a way to skip forward one pattern, not just an arbitrary size
					Player::singleton().skip(16.0);
				}
				else {
					Player::singleton().skipTo(0.0);
				}
			} else {
				std::vector<int>::iterator it = selection_.begin();
				if ( it != selection_.end() ) {
					int sel_idx = *it;
					std::map<int,SequenceEntry*>::iterator pit = pos_map_.find(sel_idx+1);
					if (pit != pos_map_.end()) {
						selected_entry_ = GetEntry(sel_idx+1);
						pat_view->SetPattern(selected_entry_->pattern());	
					} else {
						Sequence* sequence = &project_->song().sequence();
						SequenceLine* line = *(sequence->begin()+1);						
						psycle::core::Pattern* pattern = new psycle::core::Pattern();
						pattern->setID((*it)+1);
						sequence->Add(pattern);
						psycle::core::SequenceEntry* entry = new psycle::core::SequenceEntry(line);
						entry->setPattern(pattern);
						line->insertEntryAndMoveRest(entry, GetEntry(sel_idx)->tickEndPosition());
					}
					selection_.clear();
					selection_.push_back(sel_idx+1);
					UpdateSequencer();
					main_frame_->m_wndView.Repaint(draw_modes::pattern);
					main_frame_->m_wndView.SetFocus();
				}
			}
		}


		void SequencerView::DecPosition() {
			if (Global::pPlayer->playing() && Global::pConfig->_followSong)
			{				
				PlayerTimeInfo& tinfo = Player::singleton().timeInfo();
				if (tinfo.playBeatPos() > 0 ) {
					///todo: Add a way to skip back one pattern, not just an arbitrary size
					Player::singleton().skip(-16.0);
				}
				else {
					Player::singleton().skipTo(0.0);
				}
			} else {
				std::vector<int>::iterator it = selection_.begin();
				if ( it != selection_.end() ) {
					int sel_idx = *it;
					if ( sel_idx > 0 ) {
						std::map<int,SequenceEntry*>::iterator pit = pos_map_.find(sel_idx-1);
						if (pit != pos_map_.end()) {	
							selected_entry_ = GetEntry(sel_idx-1);
							project_->pat_view()->SetPattern(selected_entry_->pattern());	
						}
						selection_.clear();
						selection_.push_back(sel_idx-1);
						UpdateSequencer();
						main_frame_->m_wndView.Repaint(draw_modes::pattern);
						main_frame_->m_wndView.SetFocus();
					}
				}
			}
		}


	}	// namespace host
}	// namespace psycle

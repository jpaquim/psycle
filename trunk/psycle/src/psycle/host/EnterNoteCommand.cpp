#include "EnterNoteCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		EnterNoteCommand::EnterNoteCommand(PatternView* pat_view, int note) 
			: pat_view_(pat_view),
			  note_(note) {
		}
		
		EnterNoteCommand::~EnterNoteCommand() {
		}

		void EnterNoteCommand::Execute() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// three steps
			// 1. Undo data store
			// 2. Execute
			// 3. Redo data store

			// 1. Undo
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnCursor();
			if ( it == pat_view_->pattern()->end() ) {
				prev_has_ev_ = false;
				prev_pos_ = pat_view_->editcur.line / static_cast<double>(pat_view_->project()->beat_zoom());
			} else {
				prev_ev_ = it->second;
				prev_pos_ = it->first;
				prev_has_ev_ = true;
			}
			// 2. Execute Command
			pat_view_->EnterNote(note_);
			// 3. Redo
			it = pat_view_->GetEventOnPos(prev_pos_);
			if ( it == pat_view_->pattern()->end() ) {
				next_pos_ = prev_pos_;
				next_has_ev_ = false;
			} else {
				next_ev_ = it->second;
				next_pos_ = it->first;
				next_has_ev_ = true;
			}
#endif
		}

		void EnterNoteCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnPos(prev_pos_);
			if (it != pat_view_->pattern()->end()) {
				pat_view_->pattern()->erase(it);
			}
			if (prev_has_ev_) {
				pat_view_->pattern()->insert(prev_pos_, prev_ev_);
			}
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void EnterNoteCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnPos(next_pos_);
			if (it != pat_view_->pattern()->end()) {
				pat_view_->pattern()->erase(it);
			}
			if (next_has_ev_) {
				pat_view_->pattern()->insert(next_pos_, next_ev_);
			}
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

	}	// namespace host
}	// namespace psycle
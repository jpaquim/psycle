#include "EnterDataCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		EnterDataCommand::EnterDataCommand(PatternView* pat_view, unsigned int n_char,
										   unsigned int n_flags) 
			: pat_view_(pat_view),
			  n_char_(n_char),
			  n_flags_(n_flags) {
		}
		
		EnterDataCommand::~EnterDataCommand() {
		}

		void EnterDataCommand::Execute() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// three steps
			// 1. Undo data store
			// 2. Execute
			// 3. Redo data store

			// 1. Undo data store
			prev_track_ = pat_view_->editcur.track;
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnCursor();
			if ( it == pat_view_->pattern()->end() ) {
				prev_has_ev_ = false;
				prev_pos_ = pat_view_->editcur.line / static_cast<double>(pat_view_->project()->beat_zoom());
			} else {
				prev_ev_ = it->second;
				prev_pos_ = it->first;
				prev_has_ev_ = true;
			}
#endif
			// 2. Execute Command
			pat_view_->EnterData(n_char_, n_flags_);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// 3. Redo data store
			next_track_ = pat_view_->editcur.track;
			it = pat_view_->GetEventOnPos(prev_pos_, next_track_);
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

		void EnterDataCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnPos(prev_pos_, prev_track_);
			if (it != pat_view_->pattern()->end()) {
				pat_view_->pattern()->erase(it);
			}
			if (prev_has_ev_) {
				pat_view_->pattern()->insert(prev_pos_, prev_ev_);
			}
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void EnterDataCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern::iterator it = pat_view_->GetEventOnPos(next_pos_, next_track_);
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
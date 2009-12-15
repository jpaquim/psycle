#include "EnterNoteCommand.hpp"
#include "PatternView.hpp"

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
			prev_pattern_.Clear();
			prev_pattern_ = *pat_view_->pattern();
			pat_view_->EnterNote(note_);
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void EnterNoteCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void EnterNoteCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

	}	// namespace host
}	// namespace psycle
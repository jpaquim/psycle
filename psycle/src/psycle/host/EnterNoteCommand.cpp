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
			prev_pattern_.Clear();
			prev_pattern_ = *pat_view_->pattern();
			pat_view_->EnterNote(note_);
			next_pattern_ = *pat_view_->pattern();
		}

		void EnterNoteCommand::Undo() {
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
		}

		void EnterNoteCommand::Redo() {
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
		}

	}	// namespace host
}	// namespace psycle
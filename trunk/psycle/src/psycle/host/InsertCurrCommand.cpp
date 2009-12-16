#include "InsertCurrCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		InsertCurrCommand::InsertCurrCommand(PatternView* pat_view) 
			: pat_view_(pat_view) {
		}
		
		InsertCurrCommand::~InsertCurrCommand() {
		}

		void InsertCurrCommand::Execute() {
			// Undo store
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			prev_pattern_ = *pat_view_->pattern();
#endif
			// Execute Command
			pat_view_->InsertCurr();
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// Redo store
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void InsertCurrCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void InsertCurrCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
		}
#endif


	}	// namespace host
}	// namespace psycle

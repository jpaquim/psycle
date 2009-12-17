#include "PatTransposeCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		PatTransposeCommand::PatTransposeCommand(PatternView* pat_view, int trp) 
			: pat_view_(pat_view),
			  trp_(trp) {
		}
		
		PatTransposeCommand::~PatTransposeCommand() {
		}

		void PatTransposeCommand::Execute() {
			// Undo store
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			prev_pattern_ = *pat_view_->pattern();
#endif
			// Execute Command
			pat_view_->patTranspose(trp_);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// Redo store
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void PatTransposeCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void PatTransposeCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
		}
#endif


	}	// namespace host
}	// namespace psycle

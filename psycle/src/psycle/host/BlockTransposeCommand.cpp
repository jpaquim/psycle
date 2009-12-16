#include "BlockTransposeCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		BlockTransposeCommand::BlockTransposeCommand(PatternView* pat_view, int trp)
			: pat_view_(pat_view),
			  trp_(trp) {
		}
		
		BlockTransposeCommand::~BlockTransposeCommand() {
		}

		void BlockTransposeCommand::Execute() {
			// Undo store
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			prev_pattern_ = *pat_view_->pattern();
#endif
			// Execute Command
			pat_view_->BlockTranspose(trp_);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// Redo store
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void BlockTransposeCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void BlockTransposeCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
		}
#endif


	}	// namespace host
}	// namespace psycle

#include "PasteBlockCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		PasteBlockCommand::PasteBlockCommand(PatternView* pat_view,
											 int tx,
											 int lx,
											 bool mix) 
			: pat_view_(pat_view),
			  tx_(tx),
			  lx_(lx),
			  mix_(mix)			
		{
		}
		
		PasteBlockCommand::~PasteBlockCommand() {
		}

		void PasteBlockCommand::Execute() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			prev_pattern_ = *pat_view_->pattern();
#endif
			pat_view_->PasteBlock(tx_, lx_, mix_);
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void PasteBlockCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void PasteBlockCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}



	}	// namespace host
}	// namespace psycle

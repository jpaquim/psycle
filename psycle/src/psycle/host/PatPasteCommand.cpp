#include "PatPasteCommand.hpp"
#include "PatternView.hpp"

namespace psycle {
	namespace host {
		
		PatPasteCommand::PatPasteCommand(PatternView* pat_view, bool mix) 
			: PatHelperCommand(pat_view),
			  mix_(mix) {
		}
		
		void PatPasteCommand::Execute() {
			PatHelperCommand::PrepareUndoStorage();
			// Execute Command
			if (mix_)
				pat_view()->patMixPaste();
			else
				pat_view()->patPaste();
			PatHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

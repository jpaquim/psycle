#include "PatTransposeCommand.hpp"
#include "PatternView.hpp"

namespace psycle {
	namespace host {
		
		PatTransposeCommand::PatTransposeCommand(PatternView* pat_view, int trp) 
			: PatHelperCommand(pat_view),
			  trp_(trp) {
		}
		
		void PatTransposeCommand::Execute() {
			PatHelperCommand::PrepareUndoStorage();
			// Execute Command
			pat_view()->patTranspose(trp_);
			PatHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

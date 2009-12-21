#include "ChangeGenCommand.hpp"
#include "PatternView.hpp"

namespace psycle {
	namespace host {
		
		ChangeGenCommand::ChangeGenCommand(PatternView* pat_view, int gen) 
			: PatHelperCommand(pat_view),
			  gen_(gen) {
		}
		
		void ChangeGenCommand::Execute() {
			PatHelperCommand::PrepareUndoStorage();
			// Execute Command
			pat_view()->BlockGenChange(gen_);
			PatHelperCommand::PrepareRedoStorage();
		}
		
	}	// namespace host
}	// namespace psycle

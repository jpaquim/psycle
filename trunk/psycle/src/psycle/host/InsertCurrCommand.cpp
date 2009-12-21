#include "InsertCurrCommand.hpp"
#include "PatternView.hpp"

namespace psycle {
	namespace host {
		
		InsertCurrCommand::InsertCurrCommand(PatternView* pat_view) 
			: PatHelperCommand(pat_view) {
		}
		
		void InsertCurrCommand::Execute() {
			PatHelperCommand::PrepareUndoStorage();
			// Execute Command
			pat_view()->InsertCurr();
			PatHelperCommand::PrepareRedoStorage();
		}
		
	}	// namespace host
}	// namespace psycle

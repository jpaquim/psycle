#include "SeqIncShortCommand.hpp"
#include "SeqView.hpp"

namespace psycle {
	namespace host {
		
		SeqIncShortCommand::SeqIncShortCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}
		
		void SeqIncShortCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute
			seq_view()->OnIncshort();
			SeqHelperCommand::PrepareRedoStorage();
		}


	}	// namespace host
}	// namespace psycle

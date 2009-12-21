#include "SeqIncLenCommand.hpp"
#include "SeqView.hpp"

namespace psycle {
	namespace host {
		
		SeqIncLenCommand::SeqIncLenCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}
		
		void SeqIncLenCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute
			seq_view()->OnInclen();
			SeqHelperCommand::PrepareRedoStorage();
		}


	}	// namespace host
}	// namespace psycle

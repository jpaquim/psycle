#include "SeqDeleteCommand.hpp"
#include "SeqView.hpp"

namespace psycle { 
	namespace host {
	
		SeqDeleteCommand::SeqDeleteCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}

		void SeqDeleteCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute Command
			seq_view()->OnSeqdelete();
			SeqHelperCommand::PrepareRedoStorage();
		}


	}	// namespace host
}	// namespace psycle

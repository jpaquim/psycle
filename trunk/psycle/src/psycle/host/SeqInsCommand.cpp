#include "SeqInsCommand.hpp"
#include "SeqView.hpp"

namespace psycle { 
	namespace host {		

		SeqInsCommand::SeqInsCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}

		void SeqInsCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute Command
			seq_view()->OnSeqins();
			SeqHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

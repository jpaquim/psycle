#include "SeqSortCommand.hpp"
#include "SeqView.hpp"

namespace psycle { 
	namespace host {		

		SeqSortCommand::SeqSortCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}

		void SeqSortCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute Command
			seq_view()->OnSeqdelete();
			SeqHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

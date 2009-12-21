#include "SeqPasteCommand.hpp"
#include "SeqView.hpp"

namespace psycle { 
	namespace host {		

		SeqPasteCommand::SeqPasteCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}

		void SeqPasteCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute Command
			seq_view()->OnSeqdelete();
			SeqHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

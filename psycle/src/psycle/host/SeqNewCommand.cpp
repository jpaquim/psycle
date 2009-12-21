#include "SeqNewCommand.hpp"
#include "SeqView.hpp"

namespace psycle { 
	namespace host {		

		SeqNewCommand::SeqNewCommand(SequencerView* seq_view) 
			: SeqHelperCommand(seq_view) {
		}

		void SeqNewCommand::Execute() {
			SeqHelperCommand::PrepareUndoStorage();
			// Execute Command
			seq_view()->OnSeqnew();
			SeqHelperCommand::PrepareRedoStorage();
		}

	}	// namespace host
}	// namespace psycle

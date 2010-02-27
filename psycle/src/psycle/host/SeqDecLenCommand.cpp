#include "SeqDecLenCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {
		
SeqDecLenCommand::SeqDecLenCommand(SequencerView* seq_view) 
	: SeqHelperCommand(seq_view) {
}

void SeqDecLenCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute
	seq_view()->OnDeclen();
	SeqHelperCommand::PrepareRedoStorage();
}

}}

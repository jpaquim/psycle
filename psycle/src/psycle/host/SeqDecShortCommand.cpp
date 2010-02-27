#include "SeqDecShortCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {
		
SeqDecShortCommand::SeqDecShortCommand(SequencerView* seq_view) 
	: SeqHelperCommand(seq_view) {
}

void SeqDecShortCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute
	seq_view()->OnDecshort();
	SeqHelperCommand::PrepareRedoStorage();
}

}}

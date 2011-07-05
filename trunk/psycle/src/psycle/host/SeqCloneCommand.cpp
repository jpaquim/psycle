#include <psycle/host/detail/project.private.hpp>
#include "SeqCloneCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {
		
SeqCloneCommand::SeqCloneCommand(SequencerView* seq_view) 
	: SeqHelperCommand(seq_view) {
}

void SeqCloneCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute
	seq_view()->OnSeqduplicate();
	SeqHelperCommand::PrepareRedoStorage();
}

}}

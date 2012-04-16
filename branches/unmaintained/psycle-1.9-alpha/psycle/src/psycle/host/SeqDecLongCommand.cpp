#include <psycle/host/detail/project.private.hpp>
#include "SeqDecLongCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {
		
SeqDecLongCommand::SeqDecLongCommand(SequencerView* seq_view) 
	: SeqHelperCommand(seq_view) {
}

void SeqDecLongCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute
	seq_view()->OnDeclong();
	SeqHelperCommand::PrepareRedoStorage();
}

}}
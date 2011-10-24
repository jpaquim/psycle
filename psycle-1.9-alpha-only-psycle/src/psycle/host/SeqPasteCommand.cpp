#include <psycle/host/detail/project.private.hpp>
#include "SeqPasteCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {		

SeqPasteCommand::SeqPasteCommand(SequencerView* seq_view) 
: SeqHelperCommand(seq_view) {}

void SeqPasteCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute Command
	seq_view()->OnSeqpaste();
	SeqHelperCommand::PrepareRedoStorage();
}

}}

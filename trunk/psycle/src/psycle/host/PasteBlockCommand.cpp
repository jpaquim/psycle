#include <psycle/host/detail/project.private.hpp>
#include "PasteBlockCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
PasteBlockCommand::PasteBlockCommand(
	PatternView* pat_view,
	int tx,
	int lx,
	bool mix
) :
	PatHelperCommand(pat_view),
	tx_(tx),
	lx_(lx),
	mix_(mix)			
{}
		
void PasteBlockCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute
	pat_view()->PasteBlock(tx_, lx_, mix_);
	PatHelperCommand::PrepareRedoStorage();
}

}}

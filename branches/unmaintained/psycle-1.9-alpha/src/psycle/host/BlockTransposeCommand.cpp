#include <psycle/host/detail/project.private.hpp>
#include "BlockTransposeCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
BlockTransposeCommand::BlockTransposeCommand(PatternView* pat_view, int trp)
:
	PatHelperCommand(pat_view),
	trp_(trp)
{}

void BlockTransposeCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute Command
	pat_view()->BlockTranspose(trp_);
	PatHelperCommand::PrepareRedoStorage();
}

}}

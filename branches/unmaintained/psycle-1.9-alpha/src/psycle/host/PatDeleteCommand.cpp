#include <psycle/host/detail/project.private.hpp>
#include "PatDeleteCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
PatDeleteCommand::PatDeleteCommand(PatternView* pat_view) 
:
	PatHelperCommand(pat_view)
{}
		
void PatDeleteCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute Command
	pat_view()->patDelete();
	PatHelperCommand::PrepareRedoStorage();
}

}}

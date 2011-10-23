#include <psycle/host/detail/project.private.hpp>
#include "DeleteCurrCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
DeleteCurrCommand::DeleteCurrCommand(PatternView* pat_view) 
:
	PatHelperCommand(pat_view)
{}
		
void DeleteCurrCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute
	pat_view()->DeleteCurr();
	PatHelperCommand::PrepareRedoStorage();
}

}}

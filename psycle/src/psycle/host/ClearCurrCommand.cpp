#include <psycle/host/detail/project.private.hpp>
#include "ClearCurrCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
ClearCurrCommand::ClearCurrCommand(PatternView* pat_view) 
:
	PatHelperCommand(pat_view)
{}
		
void ClearCurrCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute
	pat_view()->ClearCurr();
	PatHelperCommand::PrepareRedoStorage();
}

}}

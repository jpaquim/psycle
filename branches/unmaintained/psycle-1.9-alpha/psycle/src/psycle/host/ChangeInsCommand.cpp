#include <psycle/host/detail/project.private.hpp>
#include "ChangeInsCommand.hpp"
#include "PatternView.hpp"

namespace psycle { namespace host {
		
ChangeInsCommand::ChangeInsCommand(PatternView* pat_view, int ins) 
:
	PatHelperCommand(pat_view),
	ins_(ins)
{}
		
void ChangeInsCommand::Execute() {
	PatHelperCommand::PrepareUndoStorage();
	// Execute Command
	pat_view()->BlockInsChange(ins_);
	PatHelperCommand::PrepareRedoStorage();
}
		
}}

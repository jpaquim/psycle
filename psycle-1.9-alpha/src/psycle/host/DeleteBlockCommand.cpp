#include <psycle/host/detail/project.private.hpp>
#include "DeleteBlockCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle { namespace host {
		
DeleteBlockCommand::DeleteBlockCommand(PatternView* pat_view) 
:
	pat_view_(pat_view)
{}
		
void DeleteBlockCommand::Execute() {
	prev_pattern_ = *pat_view_->pattern();
	pat_view_->DeleteBlock();
	next_pattern_ = *pat_view_->pattern();
}

void DeleteBlockCommand::Undo() {
	pat_view_->pattern()->Clear();
	pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
	pat_view_->pattern()->insert(prev_pattern_, 0);
	pat_view_->Repaint(PatternView::draw_modes::pattern);
}

void DeleteBlockCommand::Redo() {
	pat_view_->pattern()->Clear();
	pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
	pat_view_->pattern()->insert(next_pattern_, 0);
	pat_view_->Repaint(PatternView::draw_modes::pattern);
}

}}

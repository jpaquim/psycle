#include <psycle/host/detail/project.private.hpp>
#include "PatHelperCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle { namespace host {
		
void PatHelperCommand::PrepareUndoStorage() {
	prev_pattern_ = *pat_view_->pattern();
}

void PatHelperCommand::PrepareRedoStorage() {
	// Redo store
	next_pattern_ = *pat_view_->pattern();
}

void PatHelperCommand::Undo() {
	pat_view_->pattern()->Clear();
	pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
	pat_view_->pattern()->insert(prev_pattern_, 0);
	pat_view_->Repaint(PatternView::draw_modes::pattern);
}

void PatHelperCommand::Redo() {
	pat_view_->pattern()->Clear();
	pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
	pat_view_->pattern()->insert(next_pattern_, 0);
	pat_view_->Repaint(PatternView::draw_modes::pattern);
}

}}

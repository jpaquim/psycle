// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Command.hpp"
#include <psycle/core/pattern.h>

namespace psycle { namespace host {

/// This class offers help to prepare undo/redo storage for patternview commands
/// It prepares the undo doing a copy of the pattern
/// Operations, that could be more memory efficient implemented,
/// should use their own undo/redo prepare
class PatHelperCommand : public CommandUndoable {
	public:
		PatHelperCommand(class PatternView* pat_view) : pat_view_(pat_view) {}

		virtual void Undo();
		virtual void Redo();

	protected:
		void PrepareUndoStorage();
		void PrepareRedoStorage();
		PatternView* pat_view() { return pat_view_; }

	private:
		PatternView* pat_view_;
		Pattern prev_pattern_;
		Pattern next_pattern_;
};

}}

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include "Command.hpp"

// This class offers help to prepare undo/redo storage for SeqView commands
// It prepares the un(re)do doing a copy of the SequenceLine
// Operations, that could be more memory efficient implemented,
// should use their own undo/redo prepare

#include <psycle/core/sequence.h>

namespace psycle {
	namespace host {

		class SeqHelperCommand : public CommandUndoable {
		public:
			SeqHelperCommand(class SequencerView* pat_view);
			~SeqHelperCommand();

			virtual void Undo();
			virtual void Redo();

		protected:
			void PrepareUndoStorage();
			void PrepareRedoStorage();
			SequencerView* seq_view() { return seq_view_; }

		private:
			SequencerView* seq_view_;
			psycle::core::SequenceLine prev_line_;
			psycle::core::SequenceLine next_line_;

		};

	}	// namespace host
}	// namespace psycle

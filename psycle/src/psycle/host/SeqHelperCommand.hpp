#pragma once
#include "Command.hpp"
#include "configuration_options.hpp"

// This class offers help to prepare undo/redo storage for SeqView commands
// It prepares the undo doing a copy of the SequenceLine
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
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::SequenceLine prev_line_;
			psycle::core::SequenceLine next_line_;
#endif
		};

	}	// namespace host
}	// namespace psycle
#pragma once
#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

// This class offers help to prepare undo/redo storage for patternview commands
// It prepares the undo doing a copy of the pattern
// Operations, that could be more memory efficient implemented,
// should use their own undo/redo prepare

namespace psycle {
	namespace host {

		class PatHelperCommand : public CommandUndoable {
		public:
			PatHelperCommand(class PatternView* pat_view);
			~PatHelperCommand();

			virtual void Undo();
			virtual void Redo();

		protected:

			void PrepareUndoStorage();
			void PrepareRedoStorage();
			PatternView* pat_view() { return pat_view_; }

		private:
			PatternView* pat_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
			psycle::core::Pattern next_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle
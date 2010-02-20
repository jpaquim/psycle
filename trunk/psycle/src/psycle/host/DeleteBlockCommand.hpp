#pragma once
#include "Command.hpp"
#include <psycle/core/pattern.h>


namespace psycle {
	namespace host {

		class DeleteBlockCommand : public CommandUndoable {
		public:
			DeleteBlockCommand(class PatternView* pat_view);
			~DeleteBlockCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			PatternView* pat_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
			psycle::core::Pattern next_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle
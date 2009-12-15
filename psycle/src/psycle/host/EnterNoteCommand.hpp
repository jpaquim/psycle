#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class EnterNoteCommand : public CommandUndoable {
		public:
			EnterNoteCommand(class PatternView* pat_view, int note);
			~EnterNoteCommand();

			virtual void Execute();
			virtual void Undo();

		private:
			int note_;
			PatternView* pat_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle
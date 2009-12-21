#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class ClearCurrCommand : public CommandUndoable {
		public:
			ClearCurrCommand(class PatternView* pat_view);
			~ClearCurrCommand();

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
#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class PatPasteCommand : public CommandUndoable {
		public:
			PatPasteCommand(class PatternView* pat_view, bool mix = false);
			~PatPasteCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			PatternView* pat_view_;
			bool mix_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
			psycle::core::Pattern next_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle
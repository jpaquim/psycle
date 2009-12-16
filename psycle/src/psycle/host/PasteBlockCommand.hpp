#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class PasteBlockCommand : public CommandUndoable {
		public:
			PasteBlockCommand(class PatternView* pat_view, int tx, int lx, bool mix);
			~PasteBlockCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			PatternView* pat_view_;
			int tx_, lx_;
			bool mix_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
			psycle::core::Pattern next_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle
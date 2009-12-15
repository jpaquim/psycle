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
			virtual void Redo();

		private:
			int note_;
			PatternView* pat_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::PatternEvent prev_ev_;
			double prev_pos_;
			bool prev_has_ev_;
			psycle::core::PatternEvent next_ev_;
			double next_pos_;
			bool next_has_ev_;
#endif
		};

	}	// namespace host
}	// namespace psycle
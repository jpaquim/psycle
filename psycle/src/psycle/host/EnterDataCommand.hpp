#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class EnterDataCommand : public CommandUndoable {
		public:
			EnterDataCommand(class PatternView* pat_view, unsigned int n_char,
							 unsigned int n_flags);
			~EnterDataCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			unsigned int n_char_;
			unsigned int n_flags_;
			PatternView* pat_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::PatternEvent prev_ev_;
			double prev_pos_;
			int prev_track_;
			bool prev_has_ev_;
			psycle::core::PatternEvent next_ev_;
			double next_pos_;
			int next_track_;
			bool next_has_ev_;
#endif
		};

	}	// namespace host
}	// namespace psycle
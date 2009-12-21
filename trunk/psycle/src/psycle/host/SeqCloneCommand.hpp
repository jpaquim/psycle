#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/patternsequence.h>
#endif

namespace psycle {
	namespace host {

		class SeqCloneCommand : public CommandUndoable {
		public:
			SeqCloneCommand(class SequencerView* pat_view);
			~SeqCloneCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			SequencerView* seq_view_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::SequenceLine prev_line_;
			psycle::core::SequenceLine next_line_;
#endif
		};

	}	// namespace host
}	// namespace psycle
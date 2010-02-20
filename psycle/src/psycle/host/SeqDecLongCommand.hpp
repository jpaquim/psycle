#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqDecLongCommand : public SeqHelperCommand {
		public:
			SeqDecLongCommand(class SequencerView* pat_view);
			~SeqDecLongCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
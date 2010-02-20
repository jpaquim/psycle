#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqCloneCommand : public SeqHelperCommand {
		public:
			SeqCloneCommand(class SequencerView* pat_view);
			~SeqCloneCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
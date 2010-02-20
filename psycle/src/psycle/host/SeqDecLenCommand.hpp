#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqDecLenCommand : public SeqHelperCommand {
		public:
			SeqDecLenCommand(class SequencerView* pat_view);
			~SeqDecLenCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqInsCommand : public SeqHelperCommand {
		public:
			SeqInsCommand(class SequencerView* pat_view);
			~SeqInsCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
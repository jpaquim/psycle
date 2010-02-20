#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqIncLongCommand : public SeqHelperCommand {
		public:
			SeqIncLongCommand(class SequencerView* pat_view);
			~SeqIncLongCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
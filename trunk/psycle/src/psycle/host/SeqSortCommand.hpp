#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqSortCommand : public SeqHelperCommand {
		public:
			SeqSortCommand(class SequencerView* pat_view);
			~SeqSortCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
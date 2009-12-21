#include "Command.hpp"
#pragma once
#include "configuration_options.hpp"
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqDecShortCommand : public SeqHelperCommand {
		public:
			SeqDecShortCommand(class SequencerView* pat_view);
			~SeqDecShortCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
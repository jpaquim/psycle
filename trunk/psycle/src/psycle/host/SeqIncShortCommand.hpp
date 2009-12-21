#include "Command.hpp"
#pragma once
#include "configuration_options.hpp"
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqIncShortCommand : public SeqHelperCommand {
		public:
			SeqIncShortCommand(class SequencerView* pat_view);
			~SeqIncShortCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
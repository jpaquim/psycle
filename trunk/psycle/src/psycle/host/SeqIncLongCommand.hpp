#include "Command.hpp"
#pragma once
#include "configuration_options.hpp"
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
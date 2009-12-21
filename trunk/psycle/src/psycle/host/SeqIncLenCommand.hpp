#include "Command.hpp"
#pragma once
#include "configuration_options.hpp"
#include "SeqHelperCommand.hpp"

namespace psycle {
	namespace host {

		class SeqIncLenCommand : public SeqHelperCommand {
		public:
			SeqIncLenCommand(class SequencerView* pat_view);
			~SeqIncLenCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
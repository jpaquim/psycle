#pragma once
#include "SeqHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class SeqNewCommand : public SeqHelperCommand {
		public:
			SeqNewCommand(class SequencerView* pat_view);
			~SeqNewCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
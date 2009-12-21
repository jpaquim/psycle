#pragma once
#include "SeqHelperCommand.hpp"
#include "configuration_options.hpp"

namespace psycle {
	namespace host {

		class SeqPasteCommand : public SeqHelperCommand {
		public:
			SeqPasteCommand(class SequencerView* pat_view);
			~SeqPasteCommand() {}

			virtual void Execute();
		};

	}	// namespace host
}	// namespace psycle
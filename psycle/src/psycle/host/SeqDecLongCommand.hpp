#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqDecLongCommand : public SeqHelperCommand {
	public:
		SeqDecLongCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

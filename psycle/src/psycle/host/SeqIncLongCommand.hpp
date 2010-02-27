#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqIncLongCommand : public SeqHelperCommand {
	public:
		SeqIncLongCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

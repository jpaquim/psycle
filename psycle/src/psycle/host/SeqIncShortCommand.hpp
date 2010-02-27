#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqIncShortCommand : public SeqHelperCommand {
	public:
		SeqIncShortCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

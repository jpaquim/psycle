#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqIncLenCommand : public SeqHelperCommand {
	public:
		SeqIncLenCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

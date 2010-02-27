#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqDeleteCommand : public SeqHelperCommand {
	public:
		SeqDeleteCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

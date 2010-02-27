#pragma once
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqNewCommand : public SeqHelperCommand {
	public:
		SeqNewCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

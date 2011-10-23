#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqDecShortCommand : public SeqHelperCommand {
	public:
		SeqDecShortCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

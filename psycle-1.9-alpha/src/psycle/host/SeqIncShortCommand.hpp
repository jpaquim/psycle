#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqIncShortCommand : public SeqHelperCommand {
	public:
		SeqIncShortCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

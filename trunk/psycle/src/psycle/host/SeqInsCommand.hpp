#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqInsCommand : public SeqHelperCommand {
	public:
		SeqInsCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

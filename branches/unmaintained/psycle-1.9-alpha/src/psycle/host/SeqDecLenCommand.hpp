#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqDecLenCommand : public SeqHelperCommand {
	public:
		SeqDecLenCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

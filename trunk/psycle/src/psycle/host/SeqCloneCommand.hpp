#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqCloneCommand : public SeqHelperCommand {
	public:
		SeqCloneCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

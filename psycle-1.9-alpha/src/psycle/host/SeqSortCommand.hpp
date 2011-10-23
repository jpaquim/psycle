#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqSortCommand : public SeqHelperCommand {
	public:
		SeqSortCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqNewCommand : public SeqHelperCommand {
	public:
		SeqNewCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

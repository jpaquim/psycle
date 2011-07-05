#pragma once
#include <psycle/host/detail/project.hpp>
#include "SeqHelperCommand.hpp"

namespace psycle { namespace host {

class SeqPasteCommand : public SeqHelperCommand {
	public:
		SeqPasteCommand(class SequencerView* pat_view);
		virtual void Execute();
};

}}

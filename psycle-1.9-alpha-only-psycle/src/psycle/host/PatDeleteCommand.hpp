#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class PatDeleteCommand : public PatHelperCommand {
	public:
		PatDeleteCommand(class PatternView* pat_view);
		virtual void Execute();
};

}}

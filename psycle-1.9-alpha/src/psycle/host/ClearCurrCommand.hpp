#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class ClearCurrCommand : public PatHelperCommand {
	public:
		ClearCurrCommand(class PatternView* pat_view);
		virtual void Execute();
};

}}

#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class InsertCurrCommand : public PatHelperCommand {
	public:
		InsertCurrCommand(class PatternView* pat_view);
		virtual void Execute();
};

}}

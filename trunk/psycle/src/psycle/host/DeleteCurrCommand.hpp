#pragma once
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class DeleteCurrCommand : public PatHelperCommand {
	public:
		DeleteCurrCommand(class PatternView* pat_view);
		virtual void Execute();
};

}}

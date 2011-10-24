#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class ChangeInsCommand : public PatHelperCommand {
	public:
		ChangeInsCommand(class PatternView* pat_view, int ins);
		virtual void Execute();
	private:
		int ins_;
};

}}

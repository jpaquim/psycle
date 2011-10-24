#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class ChangeGenCommand : public PatHelperCommand {
	public:
		ChangeGenCommand(class PatternView* pat_view, int gen);
		virtual void Execute();
	private:
		int gen_;
};

}}

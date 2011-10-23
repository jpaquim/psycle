#pragma once
#include <psycle/host/detail/project.hpp>
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class PatTransposeCommand : public PatHelperCommand {
	public:
		PatTransposeCommand(class PatternView* pat_view, int trp);
		virtual void Execute();
	private:
		int trp_;
};

}}

#pragma once
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class BlockTransposeCommand : public PatHelperCommand {
	public:
		BlockTransposeCommand(class PatternView* pat_view, int trp);
		virtual void Execute();
	private:
		int trp_;
};

}}

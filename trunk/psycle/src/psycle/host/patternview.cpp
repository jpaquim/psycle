#include "patternview.hpp"
#include "MainFrm.hpp"

namespace psycle {
	namespace host {

		PatternView::PatternView(CChildView* parent, CMainFrame* main, Song* song)						
			:  parent_(parent),
			   main_(main),
			   song_(song)
		{
		}

		PatternView::~PatternView()
		{
		}

		void PatternView::Draw(CDC *devc, const CRgn& rgn)
		{
		}

	}  // namespace host
}  // namespace psycle

#pragma once


namespace psycle {
	namespace host {

		class PatternView {
		public:
			PatternView(class CChildView* parent, class CMainFrame* main, class Song* song);
			~PatternView();

			void Draw(CDC *devc, const CRgn& rgn);

			CChildView* child_view() { return parent_; }
			CMainFrame* main() { return main_; }
			Song* song() { return song_; }

		private:
			CChildView* parent_;
			CMainFrame* main_;			
			Song* song_;
		};

	}
}


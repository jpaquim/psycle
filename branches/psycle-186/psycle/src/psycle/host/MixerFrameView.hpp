///\file
///\brief interface file for psycle::host::CNativeGui.
#pragma once
#include "Psycle.hpp"
#include "NativeGui.hpp"
#include "PsycleConfig.hpp"
namespace psycle {
namespace host {

		class Machine;
		class Mixer;

		/// Native Knob-based UI for psycle plugins and non-GUI VSTs
		class MixerFrameView : public CNativeGui
		{
		enum
		{
			rowlabels=0,
			send1,
			sendmax=send1+MAX_CONNECTIONS,
			mix = sendmax,
			gain,
			pan,
			slider,
			solo,
			mute,
			dryonly,
			wetonly
		};
		enum
		{
			collabels=0,
			colmaster,
			chan1,
			chanmax=chan1+MAX_CONNECTIONS,
			return1=chanmax,
			returnmax=return1+MAX_CONNECTIONS
		};
		public:
			MixerFrameView(Machine* effect, CChildView* view);
			virtual ~MixerFrameView();
		// Operations
			virtual bool GetViewSize(CRect& rect);

		protected:
			inline Mixer& mixer(){ return *reinterpret_cast<Mixer*>(_pMachine); }
			virtual void SelectMachine(Machine* pMachine);
			inline Machine& machine(){ return *_pMachine; }
			virtual int ConvertXYtoParam(int x, int y);
			bool UpdateSendsandChans();
			void GenerateBackground(CDC &bufferDC, CDC &SliderbackDC, CDC& SliderKnobDC, CDC &VuOnDC, CDC& VuOffDC);
			int GetColumn(int x,int &xoffset);
			int GetRow(int x,int y,int &yoffset);
			int GetParamFromPos(int col,int row);
			bool GetRouteState(int ret,int send);
			
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			DECLARE_MESSAGE_MAP()
		
			// graphics
			bool updateBuffer;

			CBitmap *bmpDC;

			std::string sendNames[MAX_CONNECTIONS];
			// used to know if they have changed since last paint.
			int numSends;
			// used to know if they have changed since last paint.
			int numChans;
			int _swapstart;
			int _swapend;
			bool isslider;
			bool refreshheaders;


		};

	}   // namespace host
}   // namespace psycle

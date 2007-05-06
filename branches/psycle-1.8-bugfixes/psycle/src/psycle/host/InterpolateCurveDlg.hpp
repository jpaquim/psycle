///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include "constants.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	class CInterpolateCurve : public CDialog
	{
		public:
			CInterpolateCurve(int startsel, int endsel, CWnd* pParent = 0);
			~CInterpolateCurve();
			
		// Dialog Data
			enum { IDD = IDD_INTERPOLATE_CURVE };
			CEdit m_Pos;
			CEdit m_Value;
			CComboBox m_CurveType;
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		protected:
			virtual BOOL OnInitDialog();
			afx_msg void OnOk();
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnSelendokCurveType();
			afx_msg void OnEnChangeVal();
			afx_msg void OnEnChangePos();
			DECLARE_MESSAGE_MAP()
		private:
			float HermiteCurveInterpolate(int kf0, int kf1, int kf2, int kf3, int curposition, int maxposition, float tangmult, bool interpolation);
			RECT GetGPointRect(int i);
			void AdjustPointToView(CPoint&point);
			void AdjustRectToView(RECT&rect);
			int GetPointFromX(LONG x);

			int startIndex;
			int numLines;
			RECT grapharea;
			int	xoffset;
			float xscale;
			float yscale;
			int selectedGPoint;
			bool bDragging;
		public:
			typedef struct keyframesstruct {
				int value;
				int curvetype;
			};
			keyframesstruct *kf;
			afx_msg void OnEnKillfocusPos();
			afx_msg void OnEnKillfocusVal();
	};

	NAMESPACE__END
NAMESPACE__END

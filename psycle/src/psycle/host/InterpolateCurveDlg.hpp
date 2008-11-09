///\file
///\brief interface file for psycle::host::CNewMachine.
#pragma once
#include "Constants.hpp"
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
#include "afxwin.h" // ??
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

	class CInterpolateCurve : public CDialog
	{
		public:
			CInterpolateCurve(int startsel, int endsel, int _linesperbeat, CWnd* pParent = 0);
			~CInterpolateCurve();
			
			void AssignInitialValues(int* values,int commandtype);
			
		// Dialog Data
			enum { IDD = IDD_INTERPOLATE_CURVE };
			CButton m_checktwk;
			CComboBox m_combotwk;
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
			afx_msg void OnEnKillfocusPos();
			afx_msg void OnEnKillfocusVal();
			afx_msg void OnBnClickedChecktwk();
			DECLARE_MESSAGE_MAP()
		private:
			void AdjustPointToView(CPoint&point);
			void AdjustRectToView(RECT&rect);
			RECT GetGPointRect(int i);
			int GetPointFromX(LONG x);
			void GetNextkfvalue(int &startpos);
			void FillReturnValues();
			void SetPosText(int i);
			void SetValText(int i);
			float HermiteCurveInterpolate(int kf0, int kf1, int kf2, int kf3, int curposition, int maxposition, float tangmult, bool interpolation);

			int startIndex;
			int numLines;
			int linesperbeat;
			RECT grapharea;
			int	xoffset;
			float xscale;
			float yscale;
			int selectedGPoint;
			bool bDragging;
			typedef struct keyframesstruct {
				int value;
				int curvetype;
			};
			keyframesstruct *kf;
		public:
			int *kfresult;
			int kftwk;
	};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

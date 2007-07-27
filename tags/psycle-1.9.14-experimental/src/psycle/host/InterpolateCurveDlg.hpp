///\file
///\brief interface file for psycle::host::CNewMachine.
#include <psycle/engine/constants.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

	class CInterpolateCurve : public CDialog
		{
		public:
			CInterpolateCurve(CWnd* pParent = 0);
			~CInterpolateCurve();
			
		// Dialog Data
			//{{AFX_DATA(CInterpolateCurve)
			enum { IDD = IDD_INTERPOLATE_CURVE };
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CInterpolateCurve)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CInterpolateCurve)
			virtual BOOL OnInitDialog();
			afx_msg void OnOk();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:

		public:
			
			// list of graph points
			
			bool bInterpolate;
			//static InterpolationPoint * CInterpolateCurve::point[MAX_LINES];
			

			//TO BE FIXED//////
			
			int kfvalue[MAX_LINES]; // Keyframe value
			int kfposition[MAX_LINES]; // Row position at which it is found
			int kfcurvetype[MAX_LINES];
			
			int keyframecount;


			CEdit m_Value;
			CComboBox m_CurveType;
			afx_msg void OnPaint();

			RECT grapharea;

			float xscale;
			float yscale;
			int SelStartLine;
			int SelEndLine;
			int SelectedGPoint;
			RECT GetGPointRect(int i);

			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			bool bDragging;
			CPoint oldpoint;
			RECT olddrawnrect;
			int oldpos;
			float HermiteCurveInterpolate(int kf0, int kf1, int kf2, int kf3, int curposition, int maxposition, float tangmult, bool interpolation);
			afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnSelendokCurveType();
		};

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

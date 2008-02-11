/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include "XMInstrument.hpp"
#include "resources/resources.hpp"
#include <afxwin.h>
#include <afxcmn.h>
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
PSYCLE__MFC__NAMESPACE__BEGIN(host)

class XMSampler;
	
class XMSamplerUIInst : public CPropertyPage
{
public:
	class CEnvelopeEditor : public CStatic
		{
		public:
			// constant
		static const int POINT_SIZE = 6;///< Envelope Point size, in pixels.
			
		CEnvelopeEditor();
		virtual ~CEnvelopeEditor();
		
			void Initialize(XMSampler * const pSampler,XMInstrument::Envelope * const pEnvelope);
		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
		afx_msg void OnMouseMove( UINT nFlags, CPoint point );
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnPopAddPoint();
		afx_msg void OnPopSustainStart();
		afx_msg void OnPopSustainEnd();
		afx_msg void OnPopLoopStart();
		afx_msg void OnPopLoopEnd();
		afx_msg void OnPopRemovePoint();
		afx_msg void OnPopRemoveSustain();
		afx_msg void OnPopRemoveLoop();
		afx_msg void OnPopRemoveEnvelope();

	protected:

			/**  */
		const int GetEnvelopePointIndexAtPoint(const int x,const int y)
			{
			std::size_t const _points = m_pEnvelope->NumOfPoints();
			for(unsigned int i = 0;i < _points ;i++)
				{
					CPoint _pt_env;
				_pt_env.y = (int)((float)m_WindowHeight * (1.0f - m_pEnvelope->GetValue(i)));
				_pt_env.x = (int)(m_Zoom * (float)m_pEnvelope->GetTime(i));
					if(((_pt_env.x - POINT_SIZE / 2) <= x) & ((_pt_env.x + POINT_SIZE / 2) >= x) &
						((_pt_env.y - POINT_SIZE / 2) <= y) & ((_pt_env.y + POINT_SIZE / 2) >= y))
					{
					return i;
					}
				}

			return _points; // return == _points -> Point not found.
			}

		XMInstrument::Envelope* m_pEnvelope;
		XMSampler * m_pXMSampler;
		bool m_bInitialized;
			float m_Zoom;///< Zoom
		int m_WindowHeight;
		int m_WindowWidth;
			
			bool m_bPointEditing;///< EnvelopePoint 
		 int m_EditPoint;///< ***** Envelope Point Index
			int m_EditPointX;///< Envelope Point
			int m_EditPointY;///< Envelope Point

		CPen _line_pen;
		CPen _gridpen;
		CPen _gridpen1;
//		CBrush brush;
		CBrush _point_brush;
		};
		
	class CSampleAssignEditor : public CStatic
		{
		friend class XMSamplerUIInst;
		public:
		enum TNoteKey
			{
			NaturalKey=0,
			SharpKey
			};
		CSampleAssignEditor();
		virtual ~CSampleAssignEditor();

		void Initialize(XMSampler * const pSampler,XMInstrument * const pInstrument,CWnd *pParent);
		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

	protected:
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
		afx_msg void OnMouseMove( UINT nFlags, CPoint point );
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

		int m_naturalkey_width;
		int m_naturalkey_height;
		int m_sharpkey_width;
		int m_sharpkey_height;
		int m_octave_width;
		static const int m_NaturalKeysPerOctave = 7;
		static const int m_SharpKeysPerOctave = 5;
		static const int m_KeysPerOctave = 12;
		static const int m_SharpKey_Xpos[m_SharpKeysPerOctave];
		static const TNoteKey m_NoteAssign[m_KeysPerOctave];
		static const int m_noteAssignindex[m_KeysPerOctave];

		int GetKeyIndexAtPoint(const int x,const int y,CRect& keyRect);

	protected:
		XMSampler *m_pSampler;
		XMInstrument *m_pInst;
		CBitmap m_NaturalKey;
		CBitmap m_SharpKey;
		CBitmap m_BackKey;
		bool	m_bInitialized;
		int		m_Octave;

		int m_FocusKeyIndex;///< 
		CRect m_FocusKeyRect;///<
		CScrollBar m_scBar;
			
			};





DECLARE_DYNAMIC(XMSamplerUIInst)

public:
		XMSamplerUIInst();
		virtual ~XMSamplerUIInst();

		/// Dialog ID
		enum { IDD = IDD_XMSAMPLERUIINST };
		
protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV

		DECLARE_MESSAGE_MAP()

public:
		void pMachine(XMSampler * const p){m_pMachine = p;};
		XMSampler * const pMachine(){return m_pMachine;};

	afx_msg BOOL OnSetActive(void);
	afx_msg void OnNMCustomdrawVolCutoffPan(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSwing1Glide(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawFadeoutRes(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSwing2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawNotemodnote(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawNoteMod(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnNMCustomdrawADSRBase(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawADSRMod(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawADSRAttack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawADSRDecay(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawADSRSustain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawADSRRelease(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnLbnSelchangeInstrumentlist();
	afx_msg void OnEnChangeInsName();
	afx_msg void OnCbnSelendokFiltertype();
	afx_msg void OnBnClickedEnvcheck();
	afx_msg void OnCbnSelendokInsNnacombo();
	afx_msg void OnCbnSelendokInsDctcombo();
	afx_msg void OnCbnSelendokInsDcacombo();
	afx_msg void OnBnClickedLoadins();
	afx_msg void OnBnClickedSaveins();
	afx_msg void OnBnClickedDupeins();
	afx_msg void OnBnClickedDeleteins();
	afx_msg void OnBnClickedInsTgeneral();
	afx_msg void OnBnClickedInsTamp();
	afx_msg void OnBnClickedInsTpan();
	afx_msg void OnBnClickedInsTfilter();
	afx_msg void OnBnClickedInsTpitch();
	afx_msg void OnBnClickedEnvadsr();
	afx_msg void OnBnClickedEnvfreeform();

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		
private:

	void SetInstrumentData(const int instno);
	void SetNewNoteAction(const int nna,const int dct,const int dca);
	void AssignAmplitudeValues(XMInstrument& inst);
	void AssignPanningValues(XMInstrument& inst);
	void AssignFilterValues(XMInstrument& inst);
	void AssignPitchValues(XMInstrument& inst);
		

		XMSampler *m_pMachine;
		bool m_bInitialized;
	
protected:
	enum TabPage
		{
		general=0,
		amplitude,
		panning,
		filter,
		pitch
		};

	CListBox m_InstrumentList;
	CComboBox m_FilterType;
	CSliderCtrl m_SlVolCutoffPan;
	CButton m_cutoffPan;
	CSliderCtrl m_SlSwing1Glide;
	CButton m_Ressonance;
	CSliderCtrl m_SlFadeoutRes;
	CSliderCtrl m_SlSwing2;
	CSliderCtrl m_SlNoteModNote;
	CSliderCtrl m_SlNoteMod;

	CButton m_EnvEnabled;
//	EnvelopeEditor m_EnvelopeEditor;
	CEnvelopeEditor m_EnvelopeEditor;
	CSliderCtrl m_SlADSRBase;
	CSliderCtrl m_SlADSRMod;
	CSliderCtrl m_SlADSRAttack;
	CSliderCtrl m_SlADSRDecay;
	CSliderCtrl m_SlADSRSustain;
	CSliderCtrl m_SlADSRRelease;

	CEdit m_InstrumentName;
	CComboBox m_NNA;
	CComboBox m_DCT;
	CComboBox m_DCA;
//	SampleAssignEditor m_SampleAssignEditor;
	CSampleAssignEditor m_SampleAssign;

};

PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "XMInstrument.hpp"

namespace psycle { namespace host {

class XMSampler;

class CEnvelopeEditor : public CStatic
	{
	public:
		// constant
		static const int POINT_SIZE = 6;///< Envelope Point size, in pixels.

		CEnvelopeEditor();
		virtual ~CEnvelopeEditor();

		void Initialize(XMInstrument::Envelope& pEnvelope);
		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

		XMInstrument::Envelope& envelope() { return *m_pEnvelope; }
		bool freeform() { return m_bFreeform; }
		void freeform(bool freeform) { m_bFreeform=freeform; }
		bool negative() { return m_bnegative; }
		void negative(bool negative) { m_bnegative=negative; }
		int editPoint() { return m_EditPoint; }

	protected:
		DECLARE_MESSAGE_MAP()
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
		inline const int GetEnvelopePointIndexAtPoint(const int x,const int y)
		{
			unsigned int const _points = m_pEnvelope->NumOfPoints();
			for(unsigned int i = 0;i < _points ;i++)
			{
				CPoint _pt_env;
				_pt_env.y = (m_bnegative) 
					? static_cast<int>((float)m_WindowHeight * (1.0f - (1.0f+m_pEnvelope->GetValue(i))*0.5f))
					: static_cast<int>((float)m_WindowHeight * (1.0f - m_pEnvelope->GetValue(i)));
				_pt_env.x = static_cast<int>(m_Zoom * (float)m_pEnvelope->GetTime(i));

				if(((_pt_env.x - POINT_SIZE / 2) <= x) & ((_pt_env.x + POINT_SIZE / 2) >= x) &
					((_pt_env.y - POINT_SIZE / 2) <= y) & ((_pt_env.y + POINT_SIZE / 2) >= y))
				{
					return i;
				}
			}

			return _points; // return == _points -> Point not found.
		}

		XMInstrument::Envelope* m_pEnvelope;
		bool m_bInitialized;
		bool m_bFreeform;
		bool m_bnegative;
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
		CBrush _point_brush;
	};
}}
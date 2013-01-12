#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {
	
class XMInstrument;

class CSampleAssignEditor : public CStatic
{
		friend class XMSamplerUIInst;
	public:
		enum TNoteKey
		{
			NaturalKey=0,
			SharpKey
		};

		static const int m_NaturalKeysPerOctave;
		static const int m_SharpKeysPerOctave;
		static const int m_KeysPerOctave;
		static const int m_SharpKey_Xpos[];
		static const TNoteKey m_NoteAssign[];
		static const int m_noteAssignindex[];
	
	public:
		CSampleAssignEditor();
		virtual ~CSampleAssignEditor();

		void Initialize(XMInstrument& pInstrument);
		inline int Octave() { return m_Octave; }
		inline void Octave(int octave) { m_Octave = octave; }

	protected:
		virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
		afx_msg void OnMouseMove( UINT nFlags, CPoint point );

	protected:
		int GetKeyIndexAtPoint(const int x,const int y,CRect& keyRect);

		XMInstrument *m_pInst;
		int m_naturalkey_width;
		int m_naturalkey_height;
		int m_sharpkey_width;
		int m_sharpkey_height;
		int m_octave_width;
		bool	m_bInitialized;
		int		m_Octave;
		int m_FocusKeyIndex;///< 
		CRect m_FocusKeyRect;///<

		CBitmap m_NaturalKey;
		CBitmap m_SharpKey;
		CBitmap m_BackKey;

	};
}}
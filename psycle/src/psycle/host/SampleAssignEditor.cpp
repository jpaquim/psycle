#include <psycle/host/detail/project.private.hpp>
#include "SampleAssignEditor.hpp"
#include <psycle/host/XMInstrument.hpp>

namespace psycle { namespace host {

//////////////////////////////////////////////////////////////////////////////
// SampleAssignEditor ------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
const int CSampleAssignEditor::m_NaturalKeysPerOctave = 7;
const int CSampleAssignEditor::m_SharpKeysPerOctave = 5;
const int CSampleAssignEditor::m_KeysPerOctave = 12;

//static const unsigned int CSampleAssignEditor::m_Sharpkey_Xpos[5]= {27,96,190,246,304};
//const unsigned int CSampleAssignEditor::m_Sharpkey_Xpos[]= {16,55,110,148,185};
const int CSampleAssignEditor::m_SharpKey_Xpos[]= {15,44,92,121,150};
const CSampleAssignEditor::TNoteKey CSampleAssignEditor::m_NoteAssign[]=
	{CSampleAssignEditor::NaturalKey,CSampleAssignEditor::SharpKey,CSampleAssignEditor::NaturalKey,CSampleAssignEditor::SharpKey,CSampleAssignEditor::NaturalKey,
	CSampleAssignEditor::NaturalKey,CSampleAssignEditor::SharpKey,CSampleAssignEditor::NaturalKey,CSampleAssignEditor::SharpKey,CSampleAssignEditor::NaturalKey,CSampleAssignEditor::SharpKey,CSampleAssignEditor::NaturalKey};
const int CSampleAssignEditor::m_noteAssignindex[m_KeysPerOctave] = {0,0,1,1,2,3,2,4,3,5,4,6};


CSampleAssignEditor::CSampleAssignEditor()
: m_bInitialized(false)
, m_Octave(3)
{
	m_NaturalKey.LoadBitmap(IDB_KEYS_NORMAL);
	m_SharpKey.LoadBitmap(IDB_KEYS_SHARP);
	m_BackKey.LoadBitmap(IDB_KEYS_BACK);

	BITMAP _bmp, _bmp2;
	m_NaturalKey.GetBitmap(&_bmp);
	m_SharpKey.GetBitmap(&_bmp2);
	m_naturalkey_width = _bmp.bmWidth;
	m_naturalkey_height = _bmp.bmHeight;
	m_sharpkey_width = _bmp2.bmWidth;
	m_sharpkey_height = _bmp2.bmHeight;

	m_octave_width = m_naturalkey_width * m_NaturalKeysPerOctave;
}
CSampleAssignEditor::~CSampleAssignEditor()
{
	m_NaturalKey.DeleteObject();
	m_SharpKey.DeleteObject();
	m_BackKey.DeleteObject();
}

BEGIN_MESSAGE_MAP(CSampleAssignEditor, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()
/*
	ON_WM_CONTEXTMENU()
*/
	
void CSampleAssignEditor::Initialize(XMInstrument & pInstrument)
{
	m_pInst = &pInstrument;
	m_bInitialized=true;
	Invalidate();
}
	
void CSampleAssignEditor::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if(m_bInitialized){
		if (lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
		{
			const CString _Key_name[m_KeysPerOctave] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
			const CString _NaturalKey_name[m_NaturalKeysPerOctave] = {"C","D","E","F","G","A","B"};
			const int _NaturalKey_index[m_NaturalKeysPerOctave] = {0,2,4,5,7,9,11};
//			const CString _SharpKey_name[m_SharpKeysPerOctave] = {"C#","D#","F#","G#","A#"};
			const int _SharpKey_index[m_SharpKeysPerOctave] = {1,3,6,8,10};
//			const unsigned int _end = 2*m_NaturalKeysPerOctave;//show two octaves.
//			const unsigned int _ends = 2*m_SharpKeysPerOctave;//show two octaves.


			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			CRect _rect;
			GetClientRect(&_rect);
			dc.FillSolidRect(&_rect,RGB(0,0,0));
			dc.SetBkMode(TRANSPARENT);
	

			CDC memDC;
			memDC.CreateCompatibleDC(&dc);
			CBitmap* oldbmp = memDC.SelectObject(&m_BackKey);
			dc.SetTextColor(RGB(255,255,255));

			CString _tmp_str;
			int _index = 0,_octave = 0;

			//Draw top background image.
			for(int i = 0;i < _rect.Width() && m_Octave+_octave<10;i+=26)
			{
				dc.BitBlt(i,0, 	26,20, 	&memDC, 0,0,	SRCCOPY);
				_tmp_str.Format("%s%d",_NaturalKey_name[_index],_octave+m_Octave);
				if (m_FocusKeyRect.left>=i && m_FocusKeyRect.left<i+26 && m_FocusKeyRect.left!=m_FocusKeyRect.right){
					dc.SetTextColor(RGB(255,255,0));
					dc.TextOut(i+7,4,_tmp_str);
					dc.SetTextColor(RGB(255,255,255));
				}
				else { dc.TextOut(i+7,4,_tmp_str); }
				
				_index++;
				if(_index == m_NaturalKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			BITMAP _bmp, _bmps;
			m_NaturalKey.GetBitmap(&_bmp);
			m_SharpKey.GetBitmap(&_bmps);

			memDC.SelectObject(&m_NaturalKey);
			dc.SetTextColor(RGB(128,96,32));

			_index = 0,_octave = 0;
			for(int i = 0;i*_bmp.bmWidth < _rect.Width() && m_Octave+_octave<10;i++)
			{
				dc.BitBlt(i*_bmp.bmWidth,20, 
					_bmp.bmWidth,_bmp.bmHeight, &memDC, 0,0,	SRCCOPY);
				_tmp_str.Format("%s-%d"
					,_Key_name[m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).first%12]
					,m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).first/12);
				dc.TextOut(7+i*_bmp.bmWidth,68,_tmp_str);
				int _sample = m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).second;
				if ( _sample == 255 ) _tmp_str="--";
				else _tmp_str.Format("%02X",m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_NaturalKey_index[_index]).second);
				dc.TextOut(7+i*_bmp.bmWidth,80,_tmp_str);
				_index++;
				if(_index == m_NaturalKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			_index = 0,_octave = 0;
			memDC.SelectObject(&m_SharpKey);
			for(int i = 0;m_SharpKey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave < _rect.Width() && m_Octave+_octave<10;i++)
			{
				dc.BitBlt(m_SharpKey_Xpos[_index]+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,20, 
				_bmps.bmWidth,	_bmps.bmHeight, 	&memDC,		0,0,	SRCCOPY);
				_tmp_str.Format("%s%d"
					,_Key_name[m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).first%12]
					,m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).first/12);
				dc.TextOut(m_SharpKey_Xpos[_index]-10+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,24,_tmp_str);
				
				int _sample=m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).second;
				if ( _sample == 255 ) _tmp_str="--";
				else  _tmp_str.Format("%02X",m_pInst->NoteToSample((m_Octave+_octave)*m_KeysPerOctave+_SharpKey_index[_index]).second);
				dc.TextOut(m_SharpKey_Xpos[_index]-10+ _bmp.bmWidth*m_NaturalKeysPerOctave*_octave,36,_tmp_str);
				_index++;
				if(_index == m_SharpKeysPerOctave){
					_index = 0;
					_octave++;
				}
			}

			//
			memDC.SelectObject(oldbmp);
			dc.Detach();
		}
	}
}

int CSampleAssignEditor::GetKeyIndexAtPoint(const int x,const int y,CRect& keyRect)
{
	if ( y < 20 || y > 20+m_naturalkey_height ) return notecommands::empty;

	//Get the X position in natural key notes.
	int notenatural= ((x/m_octave_width)*m_NaturalKeysPerOctave);
	int indexnote = ((x%m_octave_width)/m_naturalkey_width);
	notenatural+=indexnote;

	keyRect.top = 20;
	keyRect.bottom = 20+m_naturalkey_height;
	keyRect.left = (notenatural)*m_naturalkey_width;
	keyRect.right = keyRect.left+m_naturalkey_width;

	//Adapt the index note to a 12note range instead of 7note.
	int note = ((x/m_octave_width)*m_KeysPerOctave);
	int cnt=0;
	while(m_noteAssignindex[cnt]!= indexnote) cnt++;
	indexnote=cnt;
	note += cnt;

	if ( y > 20+m_sharpkey_height ) 
	{
		return note+(m_Octave*m_KeysPerOctave);
	}


	//If the code reaches here, we have to check if it is a sharp key or a natural one.

	//Check previous sharp note
	if (indexnote > 0 && m_NoteAssign[indexnote-1]==SharpKey)
	{
		const int _xpos = m_SharpKey_Xpos[m_noteAssignindex[indexnote-1]] + (note / m_KeysPerOctave) * m_octave_width;
		if(x >= _xpos && x <= (_xpos + m_sharpkey_width))
		{
			keyRect.bottom = m_sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + m_sharpkey_width;
			return note-1+(m_Octave*m_KeysPerOctave);
			}
		}
	//Check next sharp note
	if ( indexnote+1<m_KeysPerOctave && m_NoteAssign[indexnote+1]==SharpKey)
	{
		const int _xpos = m_SharpKey_Xpos[m_noteAssignindex[indexnote+1]] + (note / m_KeysPerOctave) * m_octave_width;
		if(x >= _xpos && x <= (_xpos + m_sharpkey_width))
		{
			keyRect.bottom = m_sharpkey_height;
			keyRect.left = _xpos;
			keyRect.right = _xpos + m_sharpkey_width;
			return note+1+(m_Octave*m_KeysPerOctave);
		}
	}
	//Not a valid sharp note. Return the already found note.
	return note+(m_Octave*m_KeysPerOctave);
}
void CSampleAssignEditor::OnMouseMove( UINT nFlags, CPoint point )
{
	int tmp = m_FocusKeyIndex;
	m_FocusKeyIndex=GetKeyIndexAtPoint(point.x,point.y,m_FocusKeyRect);
	if  ( tmp != m_FocusKeyIndex) Invalidate();
}
void CSampleAssignEditor::OnLButtonDown( UINT nFlags, CPoint point )
{
MessageBox("hola");
}
void CSampleAssignEditor::OnLButtonUp( UINT nFlags, CPoint point )
{
MessageBox("hola");
}

}}
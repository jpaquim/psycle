///\file
///\brief implementation file for psycle::host::CInterpolateCurve.
#include <project.private.hpp>
#include "psycle.hpp"
#include "InterpolateCurveDlg.hpp"
#include ".\interpolatecurvedlg.hpp"

NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

		CInterpolateCurve::CInterpolateCurve(int startsel, int endsel,CWnd* pParent)
			: CDialog(CInterpolateCurve::IDD, pParent)
			, startIndex(startsel)
			, numLines(endsel-startsel+1)
			, selectedGPoint(-1)
			, bDragging(false)
		{
			kf = new keyframesstruct[numLines];
		}

		CInterpolateCurve::~CInterpolateCurve()
		{
		}

		void CInterpolateCurve::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			DDX_Control(pDX, IDC_POS, m_Pos);
			DDX_Control(pDX, IDC_VAL, m_Value);
			DDX_Control(pDX, IDC_CURVE_TYPE, m_CurveType);
		}

		BEGIN_MESSAGE_MAP(CInterpolateCurve, CDialog)
			ON_BN_CLICKED(IDOK, OnOk)
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONUP()
			ON_WM_MOUSEMOVE()
			ON_WM_RBUTTONDOWN()
			ON_CBN_SELENDOK(IDC_CURVE_TYPE, OnSelendokCurveType)
			ON_EN_CHANGE(IDC_VAL, OnEnChangeVal)
			ON_EN_CHANGE(IDC_POS, OnEnChangePos)
			ON_EN_KILLFOCUS(IDC_POS, OnEnKillfocusPos)
			ON_EN_KILLFOCUS(IDC_VAL, OnEnKillfocusVal)
		END_MESSAGE_MAP()
		
		BOOL CInterpolateCurve::OnInitDialog()
		{
			CDialog::OnInitDialog();

			//set combo box items
			m_CurveType.AddString ("Linear");
			m_CurveType.AddString ("Hermite Curve");
			m_CurveType.SetCurSel (1);

			//set default keyframe values
			for (int i = 0;i < numLines;i++)
			{
				kf[i].value = -1;
				kf[i].curvetype = 1;
			}
			
			//determine scaling factor
			GetClientRect(&grapharea);
			AdjustRectToView(grapharea);
			xscale = (grapharea.right - grapharea.left) / (numLines-1);
			xoffset = ((grapharea.right - grapharea.left) - (xscale*(numLines-1)))/2;
			yscale = (grapharea.bottom - grapharea.top) / float(65535); ///total height divided by max param value.
			grapharea.left+=xoffset;grapharea.right-=xoffset;
			
			return true;			
		}
		
		void CInterpolateCurve::OnOk()
		{
			///\todo: fill all the kf values.
			OnOK();		
		}

		void CInterpolateCurve::AdjustPointToView(CPoint&point)
		{
			point.y-=20; point.x-=10+xoffset;
		}
		void CInterpolateCurve::AdjustRectToView(RECT&rect)
		{
			rect.top += 20; rect.left += 10; rect.right -= 10; rect.bottom -= 100;
		}
		int CInterpolateCurve::GetPointFromX(LONG x)
		{
			int pos = -1;

			//determine x position
			float remainder = ((x * 1000) % (int(xscale * 1000)))/1000;  //remainder to reasonable accuracy
			if (remainder < (3 + int(xscale / 2 + 0.5)))
			{
				pos = int(x / xscale); //round down to neareset position on left of point
			}
			///\todo: what's the meaning of this "else if"?
			else if (remainder > ( - int(xscale / 2 + 0.5) - 6))
			{
				pos = int(x / xscale)+1;// round up to nearest position on right of point
			}
			return pos;
		}

		RECT CInterpolateCurve::GetGPointRect(int i)
		{
			RECT pointrect;
			int x = grapharea.left + int(xscale * i);
			int y = grapharea.top + int(yscale * (65535 - kf[i].value));
			pointrect.left = x - 3; pointrect.right = x + 3;
			pointrect.top = y - 3;  pointrect.bottom = y + 3;
			return pointrect;
		}

		float CInterpolateCurve::HermiteCurveInterpolate(int kf0, int kf1, int kf2, int kf3, int curposition, int maxposition, float tangmult, bool interpolation)
		{
			if ( interpolation == true )
			{
				float s = (float)curposition/(float)maxposition;
				float pws3 = pow(s, 3);
				float pws2 = pow(s, 2);
				float pws23 = 3 * pws2;
				float h1 = (2 * pws3) - pws23 + 1;
				float h2 = (-2 * pws3) + pws23;
				float h3 = pws3 - (2 * pws2) + s;
				float h4 = pws3 - pws2;

				float t1 = tangmult * (kf2-kf0);
				float t2 = tangmult * (kf3-kf1);

				return (h1 * kf1 + h2 * kf2 + h3 * t1 + h4 * t2);
			}
			else
			{
				return kf1;
			}
		}

		void CInterpolateCurve::OnPaint()
		{
			CPaintDC dc(this); // device context for painting

			RECT tmprect = grapharea;
			//set an extra border.
			tmprect.left -= xoffset; tmprect.right += xoffset;
			tmprect.top -= 10; tmprect.bottom += 10;
			dc.FillSolidRect (&tmprect, 0x00FFFFFF);
			dc.Rectangle (&tmprect);
			//draw gridlines
			//vertical
			for (int h = 0; h < numLines;h++)
			{
				dc.FillSolidRect (int(h * xscale) + grapharea.left,grapharea.top, 1,grapharea.bottom - grapharea.top, 0x00DDDDDD);
			}
			//horizontal
			for (int j = 0; j < 17; j++)  //line every 0x00001000
			{
				dc.FillSolidRect (grapharea.left, grapharea.top + int(j * yscale * 4096), grapharea.right - grapharea.left, 1, 0x00DDDDDD);
			}
			// Draw keyframe points.
			for (int i = 0;i < numLines; i++)
			{
				if (kf[i].value >= 0)
				{
					RECT pointrect = GetGPointRect(i);
					dc.FillSolidRect (&pointrect, 0);
				}
			}
			//draw lines between points
			for (int i = 0; i < numLines; i++)
			{
				int x = grapharea.left + int(xscale * i);
				int y = grapharea.top + int(yscale * (65535 - kf[i].value));
				
				if (kf[i].value >= 0)
				{
					dc.MoveTo (x, y);
					for (int h = i + 1; h <= numLines; h++)  //find next occupied pos
					{
						if (kf[h].value >= 0)
						{
							int x2 = grapharea.left + int(xscale * h);
							int y2 = grapharea.top + int(yscale * (65535 - kf[h].value));

							switch (kf[i].curvetype)
							{
							case 0:
								dc.LineTo (x2, y2);
								break;
							case 1:
								int max = x2 - x; 
								int kf0=0, kf1, kf2, kf3=0;
								kf1 = y; kf2 = y2;
								int curvept;
								
								// find kf3;
								bool bPt3Found = false;
								int k = h+1;
								while (k < numLines)
								{
									if (kf[k].value >= 0)
									{
										//found next point
										kf3 = grapharea.top + int(yscale * (65535 - kf[k].value));
										k = numLines;//force loop to end
										bPt3Found = true;
									}
									k++;
								}
								if (bPt3Found == false)
								{
									//at second last keyframe, so kf3 = kf2;
									kf3 = kf2;
								}

								//find k0
								bool bPt0Found = false;
								int l = i-1;
								while (l >=0)
								{
									if (kf[l].value >= 0)
									{
										kf0 = grapharea.top + int(yscale * (65535 - kf[l].value));
										l = -1; //force loop to end
										bPt0Found = true;
									}
									l--;
								}
								if (bPt0Found == false)
								{
									kf0 = kf1;
								}

								for (int g = 0;g < max; g++)
								{
									curvept = HermiteCurveInterpolate(kf0,kf1,kf2,kf3,g,max, 0, true);
									dc.LineTo (g + x, curvept);
								}

								break;
							}
							h = numLines ; //make loop end prematurely.
						}
					}
				}
			} 
			

			//draw selected item
			if (selectedGPoint > -1)
			{
				RECT highlightrect = GetGPointRect(selectedGPoint);
				dc.FillSolidRect (&highlightrect, 0x000000FF);
			}
		}

		void CInterpolateCurve::OnLButtonDown(UINT nFlags, CPoint point)
		{
			AdjustPointToView(point);
			if ((point.x >= -3) && (point.x <= 3+(grapharea.right-grapharea.left)) && (point.y >= 0 ) && (point.y <= (grapharea.bottom-grapharea.top) ))
			{			
				int pos = GetPointFromX(point.x);
				if (pos >= 0)
				{	
                    kf[pos].value = 65535 - int((point.y)/yscale);
					selectedGPoint = pos;
					
					char val[5];
					sprintf (val, "%d", pos+startIndex);
					m_Pos.SetWindowText (val);
					sprintf (val, "%04X", kf[pos].value);
					m_Value.SetWindowText (val);

					switch (kf[pos].curvetype)
					{
					case 0: 
						m_CurveType.SetCurSel (0); break;
					case 1:
						m_CurveType.SetCurSel (1); break;
					}

					RECT temp;  temp.left = grapharea.left - 4; temp.right = grapharea.right + 4; 
					temp.bottom = grapharea.bottom + 4; temp.top = grapharea.top - 4;
					InvalidateRect(&temp, 0);

				}
			}
			bDragging=true;
			SetCapture();
			CDialog::OnLButtonDown(nFlags, point);
		}

		void CInterpolateCurve::OnLButtonUp(UINT nFlags, CPoint point)
		{
			AdjustPointToView(point);
			bDragging = false;
			ReleaseCapture();

			CDialog::OnLButtonUp(nFlags, point);
		}

		void CInterpolateCurve::OnMouseMove(UINT nFlags, CPoint point)
		{
//			AdjustPointToView(point); //adjust is already done inside  OnLButtonDown
			if (bDragging)
			{
				OnLButtonDown(nFlags,point);
			}
			CDialog::OnMouseMove(nFlags, point);
		}

		void CInterpolateCurve::OnRButtonDown(UINT nFlags, CPoint point)
		{
			AdjustPointToView(point);
			int pos = GetPointFromX(point.x);

			if (pos >= 0)
				kf[pos].value = -1;

			if (pos == selectedGPoint)
			{
				selectedGPoint = -1;
			}

			RECT temp;  temp.left = grapharea.left - 4; temp.right = grapharea.right + 4; 
			temp.bottom = grapharea.bottom + 4; temp.top = grapharea.top - 4;
			InvalidateRect(&temp, 0);
			CDialog::OnRButtonDown(nFlags, point);
		}	

		void CInterpolateCurve::OnSelendokCurveType()
		{
			if (selectedGPoint >=0)
			{
				switch (m_CurveType.GetCurSel ())
				{
				case 0:
					kf[selectedGPoint].curvetype = 0; break;
				case 1:
					kf[selectedGPoint].curvetype = 1; break;
				}
				Invalidate();
			}
		}
		void CInterpolateCurve::OnEnChangeVal()
		{

		}
		void CInterpolateCurve::OnEnChangePos()
		{
		}

		void CInterpolateCurve::OnEnKillfocusPos()
		{
			CString text;
			m_Pos.GetWindowText(text);
			int pos=atoi(text);

			if ( pos >=startIndex && pos <startIndex+numLines)
			{
				if ( kf[pos].value == -1)
				{
					CString text2;
					m_Value.GetWindowText(text2);
					int value=atoi(text2);
					if ( value >0 && value < 65536)
					{
						kf[pos].value = value;
					}
					else 
					{
						kf[pos].value= 0;
						std::string text2;
						text2 =  kf[pos].value;
						m_Value.SetWindowText(text2.c_str());
					}
				}
				else
				{
					std::string text2;
					text2 =  kf[pos].value;
					m_Value.SetWindowText(text2.c_str());
				}
				selectedGPoint = pos;
				RECT temp;  temp.left = grapharea.left - 4; temp.right = grapharea.right + 4; 
				temp.bottom = grapharea.bottom + 4; temp.top = grapharea.top - 4;
				InvalidateRect(&temp, 0);
			}

		}

		void CInterpolateCurve::OnEnKillfocusVal()
		{
			if (selectedGPoint >=0)
			{
				CString text;
				m_Value.GetWindowText(text);
				int value=atoi(text);
				if ( value >=0 && value < 65536)
				{
					kf[selectedGPoint].value = value;
					RECT temp;  temp.left = grapharea.left - 4; temp.right = grapharea.right + 4; 
					temp.bottom = grapharea.bottom + 4; temp.top = grapharea.top - 4;
					InvalidateRect(&temp, 0);
				}
				else
				{
					std::string text2;
					text2 =  kf[selectedGPoint].value;
					m_Value.SetWindowText(text2.c_str());
				}
			}
		}



/*		void CInterpolateCurve::FillKeyFrame()
		{
			int kf0=-1, kf1=-1, kf2=-1, kf3=-1;

			int i=0;
			while (i < numLines && kf[i].value <0) i++;
			if ( i >= numLines ) break;

			for (; i < numLines;)
			{
				




				for (int h = i + 1; h <= numLines; h++)  //find next occupied pos
				{
					if (kf[h].value >= 0)
					{

				if ( i == 0)
				{

				}
				
				if ( kf[i].value < 0)
				{
					kf[i].value = 0;
				}
				else
				{
					for (int h = i + 1; h <= numLines; h++)  //find next occupied pos
					{
						if (kf[h].value >= 0)
						{
							int x2 = grapharea.left + int(xscale * h);
							int y2 = grapharea.top + int(yscale * (65535 - kf[h].value));

							switch (kf[i].curvetype)
							{
							case 0:
								dc.LineTo (x2, y2);
								break;
							case 1:
								int max = x2 - x; 
								int kf0=0, kf1, kf2, kf3=0;
								kf1 = y; kf2 = y2;
								int curvept;

								// find kf3;
								bool bPt3Found = false;
								int k = h+1;
								while (k < numLines)
								{
									if (kf[k].value >= 0)
									{
										//found next point
										kf3 = grapharea.top + int(yscale * (65535 - kf[k].value));
										k = numLines;//force loop to end
										bPt3Found = true;
									}
									k++;
								}
								if (bPt3Found == false)
								{
									//at second last keyframe, so kf3 = kf2;
									kf3 = kf2;
								}

								//find k0
								bool bPt0Found = false;
								int l = i-1;
								while (l >=0)
								{
									if (kf[l].value >= 0)
									{
										kf0 = grapharea.top + int(yscale * (65535 - kf[l].value));
										l = -1; //force loop to end
										bPt0Found = true;
									}
									l--;
								}
								if (bPt0Found == false)
								{
									kf0 = kf1;
								}

								for (int g = 0;g < max; g++)
								{
									curvept = HermiteCurveInterpolate(kf0,kf1,kf2,kf3,g,max, 0, true);
									dc.LineTo (g + x, curvept);
								}

								break;
							}
							h = numLines ; //make loop end prematurely.
		}

*/


NAMESPACE__END
NAMESPACE__END


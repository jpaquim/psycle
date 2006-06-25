///\file
///\brief implementation file for psycle::host::CInterpolateCurve.
#include <packageneric/pre-compiled.private.hpp>
#include "psycle.hpp"
#include "InterpolateCurveDlg.hpp"

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		CInterpolateCurve::CInterpolateCurve(CWnd* pParent)
			: CDialog(CInterpolateCurve::IDD, pParent)
		{
			/////////////
		}

		CInterpolateCurve::~CInterpolateCurve()
		{
		}

		void CInterpolateCurve::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CInterpolateCurve)

			//}}AFX_DATA_MAP

			DDX_Control(pDX, IDC_VAL, m_Value);
			DDX_Control(pDX, IDC_CURVE_TYPE, m_CurveType);
		}

		BEGIN_MESSAGE_MAP(CInterpolateCurve, CDialog)
			//{{AFX_MSG_MAP(CInterpolateCurve)
			ON_BN_CLICKED(IDOK, OnOk)
			//}}AFX_MSG_MAP
			
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_LBUTTONUP()
			ON_WM_MOUSEMOVE()
			ON_WM_RBUTTONDOWN()
			ON_CBN_SELENDOK(IDC_CURVE_TYPE, OnSelendokCurveType)
		END_MESSAGE_MAP()
		
		BOOL CInterpolateCurve::OnInitDialog()
		{
			
			//MessageBox ("asdfasdf","asdf");
			CDialog::OnInitDialog();

			bInterpolate = false;

			//set combo box items
			m_CurveType.AddString ("Linear");
			m_CurveType.AddString ("Hermite Curve");
			m_CurveType.SetCurSel (1);

			

			//set default curve type
			for (int i = 0;i < MAX_LINES;i++)
			{
				kfcurvetype[i] = 1;
			}
			
			//determine scaling factor
			GetWindowRect(&grapharea);			
			ScreenToClient(&grapharea);
			grapharea.top += 40; grapharea.left += 20; grapharea.right -= 20; grapharea.bottom -= 100;
			xscale = (grapharea.right - grapharea.left) / (SelEndLine - SelStartLine); 
			yscale = (grapharea.bottom - grapharea.top) / float(65535); ///total width divided by max param value.
			
			SelectedGPoint = 0;

			return true;			
		}
		
		void CInterpolateCurve::OnOk()
		{
			bInterpolate = true;
			OnOK();		
		}

		void CInterpolateCurve::OnPaint()
		{
			CPaintDC dc(this); // device context for painting
			RECT bgrect;  bgrect.left = grapharea.left - 4; bgrect.top = grapharea.top - 4;
			bgrect.right = grapharea.right + 4;  bgrect.bottom = grapharea.bottom + 4;

			dc.FillSolidRect (&bgrect, 0x00FFFFFF);
			dc.Rectangle (&bgrect);
			//draw gridlines
			//vertical
			for (int h = 0; h <= (SelEndLine - SelStartLine);h++)
			{
				//dc.SetDCPenColor (0x00CCCCCC) //not sure why this doesn't work, so i've used 
												// the fillsolidrect method instead
				dc.FillSolidRect (int(h * xscale + 0.5) + grapharea.left,grapharea.top - 3, 1,grapharea.bottom - grapharea.top + 6, 0x00DDDDDD);
			}
			//horizontal
			for (int j = 0; j < 16; j++)  //line every 0x00001000
			{
				dc.FillSolidRect (grapharea.left - 3, grapharea.top - 3 + int(j * yscale * 4096 + 0.5), grapharea.right - grapharea.left + 6, 1, 0x00DDDDDD);
			}

			 
			for (int i = 0;i < MAX_LINES; i++)
			{
				if (kfvalue[i] >= 0)
				{
					RECT pointrect = GetGPointRect(i);
	
					//char buff[100]; sprintf(buff, "%d", pointrect.left);MessageBox (buff, buff);
					
					dc.FillSolidRect (&pointrect, 0);
				}
			}
			//draw lines between points
			for (int i = SelStartLine; i < SelEndLine; i++)
			{
				int x = grapharea.left + int(xscale * (i - SelStartLine) + 0.5);
				int y = grapharea.top + int(yscale * (65535 - kfvalue[i]) + 0.5);
				
				if (kfvalue[i] >= 0)
				{
					for (int h = i + 1; h <= SelEndLine; h++)  //find next occupied pos
					{
						if (kfvalue[h] >= 0)
						{
							//char b[10]; sprintf (b, "%d", h); MessageBox (b, "kfvalue[h]");
							int x2 = grapharea.left + int(xscale * (h - SelStartLine) + 0.5);
							int y2 = grapharea.top + int(yscale * (65535 - kfvalue[h]) + 0.5);

							switch (kfcurvetype[i])
							{
							case 0:
								dc.MoveTo (x, y);
								dc.LineTo (x2, y2);
								break;
							case 1:
								int max = x2 - x; 
								int kf0, kf1, kf2, kf3;
								kf1 = y; kf2 = y2;
								int curvept;
								
								// find kf3;
								bool bPt3Found = false;
								int k = h;
								while (k < MAX_LINES)
								{
									if (kfvalue[k] >= 0)
									{
										//found next point
										kf3 = grapharea.top + int(yscale * (65535 - kfvalue[k]) + 0.5);
										k = MAX_LINES;//force loop to end
										bPt3Found = true;
									}
									k++;
								}
								if (bPt3Found = false)
								{
									//at second last keyframe, so kf3 = kf2;
									kf3 = kf2;
								}

								//find k0
								bool bPt0Found = false;
								int l = i;
								while (l >=0)
								{
									if (kfvalue[l] >= 0)
									{
										kf0 = grapharea.top + int(yscale * (65535 - kfvalue[l]) + 0.5);
										l = -1; //force loop to end
										bPt0Found = true;
									}
									l--;
								}
								if (bPt0Found = false)
								{
									kf0 = kf1;
								}


								//char bu[4]; sprintf (bu, "%d", kf0); MessageBox (bu, bu);
								for (int g = 0;g < max; g++)
								{
									curvept = HermiteCurveInterpolate(kf1,kf1,kf2,kf3,g,max, 0, true);
									dc.LineTo (g + x, curvept);
								}

								break;
							}
							h = MAX_LINES ; //make loop end prematurely.
						}
					}
				}
			} 
			

			//draw selected item
			if (SelectedGPoint > -1)
			{
				RECT highlightrect = GetGPointRect(SelectedGPoint);
				dc.FillSolidRect (&highlightrect, 0x000000FF);
			}
		}

		void CInterpolateCurve::OnLButtonDown(UINT nFlags, CPoint point)
		{
			if ((point.x >= grapharea.left - 3) && (point.x <= grapharea.right + 3 ) && (point.y >= grapharea.top ) && (point.y <= grapharea.bottom ))
			{			
				
				//check if user has clicked on a point

				bool bPointAllowed = false;
				bool bPointFound = false;
				int pos = 0;

				CClientDC dc(this);

				//determine x position
				float remainder = (((point.x - 17) * 1000) % (int(xscale * 1000)))/1000;  //remainder to reasonable accuracy

				//char buff[100]; sprintf(buff, "%e", remainder);MessageBox (buff, buff);

				if (remainder < (3 + int(xscale / 2 + 0.5)))
				{
					pos = int((point.x - 17) / xscale) + SelStartLine; //round down to neareset position on left of point
					bPointAllowed = true;
				}
				else if (remainder > ( - int(xscale / 2 + 0.5) - 6))
				{
					pos = int((point.x - 17) / xscale) + SelStartLine;// round up to nearest position on right of point
					bPointAllowed = true;
				}

				RECT temprect = GetGPointRect (pos);


				
				if (bPointAllowed)
				{	
					//char b2[100]; sprintf (b2, "%d", SelectedGPoint); MessageBox (b2, "selectedpoint");

					//temprect.left -=1; temprect.right +=1; temprect.top -=1; temprect.bottom +=1;
					if (PtInRect (& temprect, point))
					{
						SelectedGPoint = pos;
					}
					else
					{
                        kfvalue[pos] = 65535 - int((point.y - 20)/yscale + 0.5);
						SelectedGPoint = pos;
					}
					

					char val[4]; sprintf (val, "%04X", kfvalue[pos]);
					m_Value.SetWindowText (val);

					switch (kfcurvetype[pos])
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

			CDialog::OnLButtonDown(nFlags, point);
		}

		RECT CInterpolateCurve::GetGPointRect(int i)
		{
			RECT pointrect;
			int x, y;
			x = grapharea.left + int(xscale * (i - SelStartLine) + 0.5);
			y = grapharea.top + int(yscale * (65535 - kfvalue[i]) + 0.5);
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

		void CInterpolateCurve::OnLButtonUp(UINT nFlags, CPoint point)
		{
			bDragging = false;

			CDialog::OnLButtonUp(nFlags, point);
		}

		void CInterpolateCurve::OnMouseMove(UINT nFlags, CPoint point)
		{
			CDialog::OnMouseMove(nFlags, point);
		}

		void CInterpolateCurve::OnRButtonDown(UINT nFlags, CPoint point)
		{

			int pt = int((point.x - 20) / xscale) + SelStartLine;
			int allowedpt = -1;
			//char b2[100]; sprintf (b2, "%d", pt); MessageBox (b2, "kfpos");
			RECT t1 = GetGPointRect (pt);  RECT t2 = GetGPointRect(pt + 1);
			if (PtInRect( &t1, point))
				allowedpt = pt + 0;
			else if (PtInRect(&t2, point))
				allowedpt = pt + 1;

			if (allowedpt >= 0)
				kfvalue[allowedpt] = -1;

			if (allowedpt == SelectedGPoint)
			{
				SelectedGPoint = -1;
			}


			RECT temp;  temp.left = grapharea.left - 4; temp.right = grapharea.right + 4; 
			temp.bottom = grapharea.bottom + 4; temp.top = grapharea.top - 4;
			InvalidateRect(&temp, 0);
			CDialog::OnRButtonDown(nFlags, point);
		}	

		void CInterpolateCurve::OnSelendokCurveType()
		{
			if (SelectedGPoint >=0)
			{
				switch (m_CurveType.GetCurSel ())
				{
				case 0:
					kfcurvetype[SelectedGPoint] = 0; break;
				case 1:
					kfcurvetype[SelectedGPoint] = 1; break;
				}
				Invalidate();
			}
		}

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END


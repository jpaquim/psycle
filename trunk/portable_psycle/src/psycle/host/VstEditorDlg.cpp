#include "stdafx.h"
#include "Psycle.h"
#include "VstEditorDlg.h"
#include "FrameMachine.h"
#include "VstGui.h"
#include "Vst\AEffEditor.h"
#include "PresetsDlg.h"
#include "inputhandler.h"
#include "MainFrm.h"
///\file
///\brief implementation file for psycle::host::CVstEditorDlg.
namespace psycle
{
	namespace host
	{
		#define VST_PARAMETRIC_WIDTH 228
		#define VST_PARAMETRIC_HEIGHT 338
		#define VST_PARAM_PANE 1
		#define VST_UI_PANE 0

		extern CPsycleApp theApp;

		IMPLEMENT_DYNCREATE(CVstEditorDlg, CFrameWnd)

		CVstEditorDlg::CVstEditorDlg()
		{
			editorgui = false;
			creatingwindow = false;
			_pMachine = NULL;
		}

		CVstEditorDlg::~CVstEditorDlg()
		{
			if (_editorActive != NULL) 
			{
				*_editorActive = false;
			}
		}

		BEGIN_MESSAGE_MAP(CVstEditorDlg, CFrameWnd)
			//{{AFX_MSG_MAP(CVstEditorDlg)
			ON_COMMAND(ID_PARAMETERS_RANDOMPARAMETERS, OnParametersRandomparameters)
			ON_COMMAND(ID_PARAMETERS_SHOWPRESET, OnParametersShowpreset)
			ON_WM_SETFOCUS()
			ON_WM_CLOSE()
			ON_WM_SIZE()
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CVstEditorDlg::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
		{
			SIZE size;
			int width=500;
			int height=200;

			creatingwindow = true;
			_splitter.CreateStatic(this, 1, 2);
			try
			{
				editorgui = _pMachine->proxy().flags() & effFlagsHasEditor;
			}
			catch(const std::exception &)
			{
				editorgui = false;
			}
			if(editorgui)
			{
				SIZE size={200,100};
				_splitter.CreateView(0, VST_UI_PANE, RUNTIME_CLASS(CVstGui), size, pContext);
				pGui = _splitter.GetPane(0, VST_UI_PANE);
				((CVstGui*)pGui)->_pMachine= _pMachine;
				((CVstGui*)pGui)->proxy = &_pMachine->proxy();
				try
				{
					_pMachine->proxy().dispatcher(effEditOpen, 0, 0, pGui->m_hWnd);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				ERect * er;
				try
				{
					_pMachine->proxy().dispatcher(effEditGetRect, 0, 0, &er);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				width = er->right - er->left;
				height = er->bottom - er->top;

				_splitter.SetColumnInfo(VST_UI_PANE,width,width);
				
				((CVstGui*)pGui)->DoTheInit();
			}
			else
			{
				int numParameters;
				try
				{
					numParameters = _pMachine->proxy().numParams();
				}
				catch(const std::exception &)
				{
					numParameters = 0;
				}
				int ncol = 1;
				while ( (numParameters/ncol)*28 > ncol*134) ncol++;
				int parspercol = numParameters/ncol;
				if (parspercol>24)	
				{
					parspercol=24;
					ncol=numParameters/24;
					if (ncol*24 != numParameters)
					{
						ncol++;
					}
				}
				if ( parspercol*ncol < numParameters) parspercol++;
				size.cx=ncol*134;
				size.cy=parspercol*28;
				height=size.cy;
				width=size.cx;

				_splitter.CreateView(0, VST_UI_PANE, RUNTIME_CLASS(CFrameMachine), size, pContext);
				pGui = (CFrameMachine*)_splitter.GetPane(0, VST_UI_PANE);
				((CFrameMachine*)pGui)->wndView=wndView;
				((CFrameMachine*)pGui)->MachineIndex=MachineIndex;
				((CFrameMachine*)pGui)->Generate();
				((CFrameMachine*)pGui)->SelectMachine(_pMachine);
				((CFrameMachine*)pGui)->_pActive = 0;
			}
			size.cx = VST_PARAMETRIC_WIDTH;
			size.cy = VST_PARAMETRIC_HEIGHT;
			width += size.cx;
			if (height < size.cy)
			{
				height = size.cy;
			}
			width += GetSystemMetrics(SM_CXEDGE)*3;
			height += 9+GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE);

			_splitter.CreateView(0, VST_PARAM_PANE, RUNTIME_CLASS(CDefaultVstGui), size, pContext);
			pParamGui= (CDefaultVstGui*)_splitter.GetPane(0,VST_PARAM_PANE);
			pParamGui->_pMachine = _pMachine;
			pParamGui->mainView=pGui;
			pParamGui->childView=wndView;
			pParamGui->MachineIndex=MachineIndex;
			pParamGui->Init();

			creatingwindow = false;

			CWnd *dsk = GetDesktopWindow();
			CRect rClient;
			dsk->GetClientRect(&rClient);

			MoveWindow(rClient.Width()/2-(width/2), rClient.Height()/2-(height/2), width, height, true);	

		//	SetWindowPos(this, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

			pGui->SetFocus();

			return CFrameWnd::OnCreateClient(lpcs, pContext);
		}

		void CVstEditorDlg::OnParametersRandomparameters() 
		{
			// Randomize controls
			int numParameters;
			try
			{
				numParameters = reinterpret_cast<vst::plugin *>(_pMachine)->proxy().numParams();
			}
			catch(const std::exception &)
			{
				numParameters = 0;
			}
			catch(...) // reinterpret_cast sucks
			{
				numParameters = 0;
			}
			for(int c(0); c < numParameters ; ++c)
			{
				const float randsem(static_cast<float>(rand() * 0.000030517578125f));
				try
				{
					reinterpret_cast<vst::plugin *>(_pMachine)->proxy().setParameter(c, randsem);
				}
				catch(const std::exception &)
				{
					// o_O`
				}
				catch(...) // reinterpret_cast sucks
				{
					// o_O`
				}
			}
			pGui->Invalidate(false);
		}

		void CVstEditorDlg::OnParametersShowpreset() 
		{
			CPresetsDlg dlg;
			dlg._pMachine=_pMachine;
			dlg.DoModal();
			pParamGui->UpdateOne();
			if (!editorgui) pGui->Invalidate(false);
			pGui->SetFocus();
		}

		void CVstEditorDlg::OnSetFocus(CWnd* pOldWnd) 
		{
			CFrameWnd::OnSetFocus(pOldWnd);
			
			pGui->SetFocus();
		}

		void CVstEditorDlg::Refresh(int par,float value)
		{
			if ( par == -1 ) pParamGui->UpdateOne();
			else pParamGui->UpdateNew(par,value);
			if (!editorgui) pGui->Invalidate(false);
		}

		void CVstEditorDlg::Resize(int w,int h)
		{
			if (!creatingwindow)
			{
				int nw, nh;
				nw = w + VST_PARAMETRIC_WIDTH + GetSystemMetrics(SM_CXEDGE)*3;
				nh = h + VST_PARAMETRIC_HEIGHT + 9+GetSystemMetrics(SM_CYCAPTION) +
					GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYEDGE);

				SetWindowPos(NULL, 0, 0, nw, nh, SWP_NOMOVE | SWP_NOZORDER);
				
				_splitter.SetColumnInfo(VST_UI_PANE,w,w);
			}
		}

		void CVstEditorDlg::OnClose() 
		{
			_pMachine->editorWnd=NULL;
			
			CFrameWnd::OnClose();
			((CMainFrame *)theApp.m_pMainWnd)->m_wndView.SetFocus();
		}
	}
}

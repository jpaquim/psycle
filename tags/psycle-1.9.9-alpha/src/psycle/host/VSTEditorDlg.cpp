///\file
///\brief implementation file for psycle::host::CVstEditorDlg.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/VstEditorDlg.hpp>
#include <psycle/host/FrameMachine.hpp>
#include <psycle/host/VstGui.hpp>
#include <psycle/host/PresetsDlg.hpp>
//#include <psycle/host/inputhandler.hpp>
#include <psycle/host/MainFrm.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
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
			int width, height;
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
				try
				{
					ERect * er;
					_pMachine->proxy().dispatcher(effEditGetRect, 0, 0, &er);
					width = er->right - er->left;
					height = er->bottom - er->top;
				}
				catch(const std::exception &)
				{
					// o_O`
					width = 500;
					height = 200;
				}

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
				while ( (numParameters/ncol)*K_YSIZE > ncol*W_ROWWIDTH) ncol++;
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
				size.cx=ncol*W_ROWWIDTH;
				size.cy=parspercol*K_YSIZE;
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
			// \todo : Sizes for this case are wrong. The length of the last column is shortened.
			size.cx = VST_PARAMETRIC_WIDTH;
			size.cy = VST_PARAMETRIC_HEIGHT;
			width += size.cx;
			if (height < size.cy)
			{
				height = size.cy;
			}
			width += GetSystemMetrics(SM_CXEDGE)*4;
			height += 9+GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + (GetSystemMetrics(SM_CYEDGE)*2);

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

			MoveWindow((rClient.Width()-width)/2, (rClient.Height()-height)/2, width, height, true);	

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
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

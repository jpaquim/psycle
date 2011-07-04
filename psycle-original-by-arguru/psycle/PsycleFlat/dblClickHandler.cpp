//////////////////////////////////////////////////////////////////////
// Double Click Handler

void CChildView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
int tmac=-1;
int mmod;

switch(viewMode)
{
case 0: // User is in machine view mode

	tmac=GetMachine(point);

	if(tmac!=-1)
	{
	mmod=SONG->machine[tmac]->machineMode;

// Master -------------------------------------------------------------

	if(mmod==0)
	{
			CMasterDlg m_wndMaster;
			m_wndMaster.machineRef=SONG->machine[tmac];
			m_wndMaster.DoModal();
	}

// Psychosc ----------------------------------------------------------

	if(mmod==1)
	{
		gearPsychosc tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}
// Distortion --------------------------------------------------------

  	if(mmod==2)
	{
		CGearDistort tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}
// Tracker -----------------------------------------------------------

  	if(mmod==3)
	{
		CGearTracker tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}

// Delay -------------------------------------------------------------

  	if(mmod==4)
	{
		CGearDelay tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.SPT=&SONG->SamplesPerTick;
		tmacEditor.DoModal();
	}

// Filter -------------------------------------------------------------

  	if(mmod==5)
	{
		CGearfilter tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}

// Gainer -------------------------------------------------------------

  	if(mmod==6)
	{
		CGearGainer tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}

// Flanger ------------------------------------------------------------

  	if(mmod==7)
	{
		CGearFlanger tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.DoModal();
	}

// Plugin ------------------------------------------------------------

  	if(mmod==8)
	{
		pParentMain->ShowMachineGui(tmac);
	}

// VST2 Inst/Eff -----------------------------------------------------

  	if(mmod==9)
	{
		CGearVst tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.songRef=SONG;
		tmacEditor.DoModal();
	}

	if(mmod==10)
	{
		CGearVst tmacEditor;
		tmacEditor.machineRef=SONG->machine[tmac];
		tmacEditor.songRef=SONG;
		tmacEditor.DoModal();
	}

	}// some tmac...
	else
	{
		// Show new machine dialog
		CNewMachine dlg;
		dlg.songRef=SONG;
		dlg.xLoc=point.x;
		dlg.yLoc=point.y;
		dlg.Outputdll=-1;

		if(dlg.DoModal()==IDOK)
		{
			// Stop driver to handle possible conflicts
			// between threads.

			pAudioDriver->Enable(false);

			if(!SONG->CreateMachine(dlg.Outputmachine,dlg.xLoc,dlg.yLoc,-1,dlg.Outputdll))
			{
			MessageBox("Error!","Machine Creation Failed",MB_OK);
			}
			else if(dlg.OutBus)
			{
			SONG->seqBus=SONG->GetFreeBus();
			SONG->busMachine[SONG->seqBus]=lbc;
			UpdateSBrowseDlg();
			}

			// Restarting the driver...
			pParentMain->UpdateEnvInfo();
			pAudioDriver->Enable(true);
		}
	}

		updateMode=0;
		Invalidate(false);
break;

}//<-- End DOUBLELCLICKPRESING/VIEWMODE switch statement

}

#include "mainwindow.h"
#include "bitmaps.h"

enum {
  ID_Quit = 1,
  ID_About,
  ID_New,
  ID_Open,
  ID_Save,
  ID_Undo,
  ID_Redo
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
EVT_MENU(ID_Quit, MainWindow::quit)
EVT_MENU(ID_About, MainWindow::about)
EVT_MENU(ID_New, MainWindow::newSong)
EVT_MENU(ID_Open, MainWindow::open)
EVT_MENU(ID_Save, MainWindow::save)
EVT_MENU(ID_Undo, MainWindow::undo)
EVT_MENU(ID_Redo, MainWindow::redo)
END_EVENT_TABLE()

MainWindow::MainWindow()
  : wxFrame(NULL,-1,wxT("Hello World"), wxPoint(50,50), wxSize(450,340))
{
  createMenus();
  createStatusBar();
}

void MainWindow::createMenus() {
  wxMenuBar* menuBar = new wxMenuBar();

  wxMenu* fileMenu = new wxMenu();
  fileMenu->Append(ID_New, wxT("&New"))->SetBitmap(*Bitmaps::_new);
  fileMenu->Append(ID_Open, wxT("&Open..."))->SetBitmap(*Bitmaps::open);
  fileMenu->Append(ID_Save, wxT("&Save..."))->SetBitmap(*Bitmaps::save);
  fileMenu->AppendSeparator();
  fileMenu->Append(ID_Quit, wxT("E&xit"));
  menuBar->Append(fileMenu, wxT("&File"));

  wxMenu* editMenu = new wxMenu();
  editMenu->Append(ID_Undo, wxT("&Undo"));
  editMenu->Append(ID_Redo, wxT("&Redo"));
  menuBar->Append(editMenu, wxT("&Edit"));

  wxMenu* viewMenu = new wxMenu();
  menuBar->Append(viewMenu, wxT("&View"));
  wxMenu* configMenu = new wxMenu();
  menuBar->Append(configMenu, wxT("&Configuration"));
  wxMenu* performMenu = new wxMenu();
  menuBar->Append(performMenu, wxT("&Perform"));
  wxMenu* communityMenu = new wxMenu();
  menuBar->Append(communityMenu, wxT("&Community"));
  //menuBar->AppendSeparator();
  wxMenu* helpMenu = new wxMenu();
  menuBar->Append(helpMenu, wxT("&Help"));
  
  SetMenuBar(menuBar);
}
	 
void MainWindow::createStatusBar() {
  CreateStatusBar();
  SetStatusText(wxT("Ready"));
}

void MainWindow::createToolBars() {
  wxToolBar* tools = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER,-1);
  tools->SetToolBitmapSize(wxSize(16,16));
  assert(Bitmaps::_new);
  tools->AddTool(ID_New,*Bitmaps::_new,wxT("New"));
  tools->AddTool(ID_Open,*Bitmaps::open,wxT("Open"));
  tools->AddTool(ID_Save,*Bitmaps::save,wxT("Save"));
  //tools->AddSeparator();
  tools->Realize();
}

void MainWindow::newSong(wxCommandEvent& event) {
}

void MainWindow::open(wxCommandEvent& event) {
}

void MainWindow::save(wxCommandEvent& event) {
  wxString fileName = wxFileSelector(wxT("Choose a file name"),
				     wxT("."),
				     wxT(""),
				     wxT(".html"),
				     wxT("HTML (*.html *.htm)"));

  // ...
  
  SetStatusText(wxString::Format(wxT("Saved %s"),fileName.c_str()));
}	 

void MainWindow::quit(wxCommandEvent& event) {
  Close(true);
}

void MainWindow::undo(wxCommandEvent& event) {
}

void MainWindow::redo(wxCommandEvent& event) {
}

void MainWindow::about(wxCommandEvent& event) {
  wxMessageBox(wxT("It makes music and stuff."),
	       wxT("About wxpsycle"),
	       wxOK | wxICON_INFORMATION, this);
}

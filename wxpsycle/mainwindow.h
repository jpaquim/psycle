#include <wx/wx.h>

class MainWindow: public wxFrame {
 public:
  MainWindow();
 private:

  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  

  void quit(wxCommandEvent& event);
  void about(wxCommandEvent& event);
  void newSong(wxCommandEvent& event);
  void open(wxCommandEvent& event);
  void save(wxCommandEvent& event);
  void undo(wxCommandEvent& event);
  void redo(wxCommandEvent& event);

  DECLARE_EVENT_TABLE()
};

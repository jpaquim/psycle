// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.private.hpp>
#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <boost/signal.hpp>

#include "Scintilla.h"
#include "SciLexer.h"

namespace psycle {
namespace host  {

class CScintilla : public CWnd {
 public:
  CScintilla() : find_flags_(0), is_modified_(false), has_file_(false) {}
  bool Create(CWnd* pParentWnd, UINT nID) {
    if (!CreateEx(0, 
        _T("scintilla"),
         "", 
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN
        , CRect(0, 0, 0, 0),
        pParentWnd,
        nID,
        0)) {
      TRACE0("Failed to create scintilla window\n");				
      return false;
    }
      
    fn = (int (__cdecl *)(void *,int,int,int))
         SendMessage(SCI_GETDIRECTFUNCTION, 0, 0);
    ptr = (void *)SendMessage(SCI_GETDIRECTPOINTER, 0, 0);
      return true;
  }

  template<class T, class T1>
  int f(int sci_cmd, T lparam, T1 wparam) {     
    return fn(ptr, sci_cmd, (WPARAM) lparam, (LPARAM) wparam);
  }
    
  void AddText(const std::string& text) {       
    f(SCI_ADDTEXT, text.size(), text.c_str());
  }

  void FindText(const std::string& text, int cpmin, int cpmax, int& pos, int& cpselstart, int& cpselend) const {
    TextToFind txt;
    txt.chrg.cpMin = cpmin;      
    txt.chrg.cpMax = cpmax;
    txt.lpstrText = text.c_str();
    pos = const_cast<CScintilla*>(this)->f(SCI_FINDTEXT, find_flags_, &txt);
    cpselstart = txt.chrgText.cpMin;
    cpselend = txt.chrgText.cpMax;
  }

  void GotoLine(int pos) { f(SCI_GOTOLINE, pos, 0); }
  int length() const {
    return const_cast<CScintilla*>(this)->f(SCI_GETLENGTH, 0, 0);
  }
  int selectionstart() const {
    return const_cast<CScintilla*>(this)->f(SCI_GETSELECTIONSTART, 0, 0);     
  }
  int selectionend() const {
    return const_cast<CScintilla*>(this)->f(SCI_GETSELECTIONEND, 0, 0);     
  }
  void SetSel(int cpmin, int cpmax) { f(SCI_SETSEL, cpmin, cpmax); }
  bool has_selection() const { return selectionstart() != selectionend(); }
 
  void set_find_match_case(bool on) {     
   if (on) {
      find_flags_ = find_flags_ | SCFIND_MATCHCASE;
    } else {
      find_flags_ = find_flags_ & ~SCFIND_MATCHCASE;
    }
  }

  void set_find_whole_word(bool on) {
    if (on) {
      find_flags_ = find_flags_ | SCFIND_WHOLEWORD;
    } else {
      find_flags_ = find_flags_ & ~SCFIND_WHOLEWORD;
    }
  }

  void LoadFile(const std::string& filename) {
    using namespace std;    
    #if __cplusplus >= 201103L
      ifstream file (filename, ios::in|ios::binary|ios::ate);
    #else
      ifstream file (filename.c_str(), ios::in|ios::binary|ios::ate);
    #endif
    if (file.is_open()) {
      is_modified_ = true;
      f(SCI_CANCEL, 0, 0);
      f(SCI_SETUNDOCOLLECTION, 0, 0);      
      file.seekg(0, ios::end);
      size_t size = file.tellg();
      char *contents = new char [size];
      file.seekg (0, ios::beg);
      file.read (contents, size);
      file.close();
      f(SCI_ADDTEXT, size, contents);      
      f(SCI_SETUNDOCOLLECTION, true, 0);
      f(SCI_EMPTYUNDOBUFFER, 0, 0);      
      f(SCI_SETSAVEPOINT, 0, 0);      
      f(SCI_GOTOPOS, 0, 0);      
      delete [] contents;
      fname_ = filename;
      is_modified_ = false;
      has_file_ = true;
    } else {
      throw std::runtime_error("File Not Open Error!");
    }
  }

  void SaveFile(const std::string& filename) {
    //Get the length of the document
    int nDocLength = f(SCI_GETLENGTH, 0, 0);
    //Write the data in blocks to disk
    CFile file;
    BOOL res = file.Open(_T(filename.c_str()), CFile::modeCreate | CFile::modeReadWrite);
    if (res) {
      is_modified_ = true;
      for (int i=0; i<nDocLength; i += 4095) //4095 because data will be returned NULL terminated
      {
        int nGrabSize = nDocLength - i;
        if (nGrabSize > 4095)
          nGrabSize = 4095;
        //Get the data from the control
        TextRange tr;
        tr.chrg.cpMin = i;
        tr.chrg.cpMax = i + nGrabSize;
        char Buffer[4096];
        tr.lpstrText = Buffer;
        f(SCI_GETTEXTRANGE, 0, &tr);
        //Write it to disk
        file.Write(Buffer, nGrabSize);
      }
    file.Close();
    has_file_ = true;
    fname_ = filename;
    is_modified_ = false;
    } else {
      throw std::runtime_error("File Save Error");
    }
  }  
  bool has_file() const { return has_file_; }
  const std::string& filename() const { return fname_; }
  bool is_modified() const { return is_modified_; }
  virtual void OnModified() { 
    if (!is_modified_) {
      OnFirstModified();
    }
    is_modified_ = true;
  }

  virtual void OnFirstModified() {}

  boost::signal<void ()> modified;
            
 protected:
  DECLARE_DYNAMIC(CScintilla)     
  int (*fn)(void*,int,int,int);
  void * ptr;
   
  BOOL OnModified(NMHDR *,LRESULT *) {
    modified();
    return false;
  }

  void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  }

  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  }

  virtual BOOL PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message==WM_KEYDOWN ) {
      int fordummies = 0;
      // return 1;
    } 
    return CWnd::PreTranslateMessage(pMsg);
  }

  
  DECLARE_MESSAGE_MAP(); 

 private:         
  DWORD find_flags_;
  std::string fname_;
  bool is_modified_, has_file_;
};

} // namespace host
} // namespace psycle
// code from http://www.drdobbs.com/windows/building-mfc-dialogs-at-runtime/184405811
//:  Implementation of the Dialog_item class.

#include <psycle/host/detail/project.hpp>
#include "Dialog_item.h"
#include "Dyn_dialog.h"   // for enum on file type


// -------------------- Function Dialog_item ------------------------
//Dialog_item::Dialog_item(const Dialog_item& r) { } Not used.


// -------------------- Function Dialog_item ------------------------
Dialog_item::Dialog_item() :
   d_controltype(e_static_text), d_data_type(e_no_data),
   d_caption(_T("")), d_value(0)
{
   d_item_template.style = WS_VISIBLE | WS_CHILD;
   d_item_template.dwExtendedStyle = 0;
   d_item_template.id = 0;
   Set_position_and_size(0, 0, 10, 10);  // completes DLGITEMTEMPLATE

   d_max.i = d_min.i = 0;

   return;
}


// -------------------- Function Define_static_text -----------------
void Dialog_item::Define_static_text(WORD id, int left, int top,
                                     int width, int height,
                                     const char* caption,
                                     unsigned int text_alignment)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_item_template.style |= WS_GROUP | text_alignment;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   return;
}


// -------------------- Function Define_combo_or_list_box -----------
void Dialog_item::Define_combo_or_list_box(bool is_combo, WORD id,
                                           int left, int top,
                                           int width, int height,
                                           int* index,
                                           int first_string_index,
                                           int last_string_index,
                                           bool sort)
{
   d_data_type = e_int;
   d_value = static_cast<void*>(index);

   Set_combo_list_box_common_parameters(is_combo, id, left, top,
                                        width, height,
                                        first_string_index,
                                        last_string_index, sort);
   return;
}


// -------------------- Function Define_combo_or_list_box -----------
void Dialog_item::Define_combo_or_list_box(bool is_combo, WORD id,
                                           int left, int top,
                                           int width, int height,
                                           CString* str,
                                           int first_string_index,
                                           int last_string_index,
                                           bool sort)
{
   d_data_type = e_cstring;
   d_value = static_cast<void*>(str);

   Set_combo_list_box_common_parameters(is_combo, id, left, top,
                                        width, height,
                                        first_string_index,
                                        last_string_index, sort);
   return;
}


// --------------- Function Set_combo_list_box_common_parameters ----
void Dialog_item::Set_combo_list_box_common_parameters(bool is_combo,
                                              WORD id, int left,
                                              int top, int width,
                                              int height,
                                              int first_string_index,
                                              int last_string_index,
                                              bool sort)
{
   if (is_combo)
      {
      d_controltype = e_combobox;

      d_item_template.style |= WS_TABSTOP | WS_VSCROLL | WS_BORDER
                                  | CBS_DROPDOWNLIST;
      if (sort)
         {
         d_item_template.style |= CBS_SORT;
         }
      }
   else
      {
      d_controltype = e_listbox;

      d_item_template.style |= WS_TABSTOP | WS_VSCROLL | WS_BORDER;
      if (sort)
         {
         d_item_template.style |= LBS_SORT;
         }
      }

   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   d_min.i = first_string_index;
   d_max.i =  last_string_index;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      CString* value,
                                      int n_char_limit)
{
   d_data_type = e_cstring;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_max.i = d_min.i = n_char_limit;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      int* value, int min, int max)
{
   d_data_type = e_int;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.i = min;
   d_max.i = max;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      double* value, double min,
                                      double max)
{
   d_data_type = e_double;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.d = min;
   d_max.d = max;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      float* value, float min,
                                      float max)
{
   d_data_type = e_float;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.f = min;
   d_max.f = max;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      unsigned int* value,
                                      unsigned int min,
                                      unsigned int max)
{
   d_data_type = e_unsigned_int;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.ui = min;
   d_max.ui = max;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      BYTE* value, BYTE min,
                                      BYTE max)
{
   d_data_type = e_byte;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.by = min;
   d_max.by = max;

   return;
}


// -------------------- Function Define_edit_control ----------------
void Dialog_item::Define_edit_control(WORD id, int left, int top,
                                      int width, int height,
                                      short* value, short min,
                                      short max)
{
   d_data_type = e_short;

   Set_edit_control_common_parameters(id, left, top, width, height);

   d_value = static_cast<void*>(value);

   d_min.si = min;
   d_max.si = max;

   return;
}


// -------------------- Function Set_edit_control_common_parameters -
void Dialog_item::Set_edit_control_common_parameters(WORD id,
                                                     int left,
                                                     int top,
                                                     int width,
                                                     int height)
{
   d_controltype = e_editcontrol;

   d_item_template.style |= WS_TABSTOP | ES_AUTOHSCROLL | WS_BORDER
                              | ES_LEFT;
   d_item_template.id = id;

   Set_position_and_size(left, top, width, height);

   return;
}


// -------------------- Function Define_pushbutton ------------------
void Dialog_item::Define_pushbutton(WORD id, int left, int top,
                                    int width, int height,
                                    const char* caption)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_controltype = e_button;

   d_item_template.style |= WS_TABSTOP | BS_PUSHBUTTON;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   return;
}


// -------------------- Function Define_checkbox --------------------
void Dialog_item::Define_checkbox(WORD id, int left, int top,
                                  int width, int height,
                                  const char* caption, bool* value)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_controltype = e_button;
   d_data_type   = e_bool;

   d_item_template.style |= WS_TABSTOP | BS_AUTOCHECKBOX;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   d_value = static_cast<void*>(value);

   d_max.i = d_min.i = 0;

   return;
}


// -------------------- Function Define_first_radio_button ----------
void Dialog_item::Define_first_radio_button(WORD id, int left,
                                            int top, int width,
                                            int height,
                                            const char* caption,
                                            int* value)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_controltype = e_button;
   d_data_type = e_int;

   d_item_template.style |= WS_TABSTOP | WS_GROUP
                              | BS_AUTORADIOBUTTON;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   d_value = static_cast<int*>(value);

   d_max.i = d_min.i = 0;

   return;
}


// -------------------- Function Define_radio_button ----------------
void Dialog_item::Define_radio_button(WORD id, int left, int top,
                                      int width, int height,
                                      const char* caption)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_controltype = e_button;

   d_item_template.style |= WS_TABSTOP | BS_AUTORADIOBUTTON;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   return;
}


// -------------------- Function Define_group_box -------------------
void Dialog_item::Define_group_box(WORD id, int left, int top,
                                   int width, int height,
                                   const char* caption)
{
   if (caption)
      {
      d_caption = _T(caption);
      }

   d_controltype = e_button;

   d_item_template.style |= BS_GROUPBOX | WS_GROUP;
   d_item_template.id = id;
   Set_position_and_size(left, top, width, height);

   return;
}


// -------------------- Function Set_position_and_size --------------
void Dialog_item::Set_position_and_size(int left, int top, int width,
                                        int height)
{
   d_item_template.x  = static_cast<short>(left);
   d_item_template.y  = static_cast<short>(top);
   d_item_template.cx = static_cast<short>(width);
   d_item_template.cy = static_cast<short>(height);

   return;
}


// -------------------- Function operator= --------------------------
const Dialog_item& Dialog_item::operator=(const Dialog_item& r)
{
   if (this != &r)
      {
      d_caption       = r.d_caption;
      d_controltype   = r.d_controltype;
      d_data_type     = r.d_data_type;
      d_item_template = r.d_item_template;
      d_value         = r.d_value;

      memcpy(&d_min, &r.d_min, sizeof(uval));
      memcpy(&d_max, &r.d_max, sizeof(uval));
      }

   return *this;
}


// -------------------- Function Get_control_type -------------------
Dialog_item::controltype Dialog_item::Get_control_type() const
{
   return d_controltype;
}


// -------------------- Function Is_string_edit_control -------------
bool Dialog_item::Is_string_edit_control() const
{
   return ((d_controltype == e_editcontrol) &&
           (d_data_type == e_cstring)) ? true : false;
}


// -------------------- Function Get_caption ------------------------
const CString& Dialog_item::Get_caption() const
{
   return d_caption;
}


// -------------------- Function Add_strings_to_box -----------------
void Dialog_item::Add_strings_to_box(const CStringList& strings,
                                     POSITION& pos, CWnd* box) const
{
   CComboBox* combo_box = 0;
   CListBox*  list_box  = 0;

   if (d_controltype == e_combobox)
      {
      combo_box = static_cast<CComboBox*>(box);
      }
   else
      {
      list_box = static_cast<CListBox*>(box);
      }

   int i = d_min.i;
   const int last_index = d_max.i;

   while (i <= last_index)
      {
      if (d_controltype == e_combobox)
         {
         combo_box->AddString(strings.GetNext(pos));
         }
      else
         {
         list_box->AddString(strings.GetNext(pos));
         }
      ++i;
      }

   int value = 0;

   if (d_data_type == e_int)
      {
      value = *(static_cast<int*>(d_value));

      if ((value < -1) || (value > (last_index - d_min.i)))
         {
         value = -1;
         }
      }
   else                             // the variable is a CString
      {
      if (d_controltype == e_combobox)
         {
         int i = combo_box->FindStringExact(-1,
                                  *(static_cast<CString*>(d_value)));
         if (i != CB_ERR)
            {
            value = i;
            }
         }
      else
         {
         int i = list_box->FindStringExact(-1,
                                 *(static_cast<CString*>(d_value)));
         if (i != LB_ERR)
            {
            value = i;
            }
         }
      }

   if (d_controltype == e_combobox)
      {
      combo_box->SetCurSel(value);
      }
   else
      {
      list_box->SetCurSel(value);
      }

   return;
}


// -------------------- Function Add_style_parameters ---------------
void Dialog_item::Add_style_parameters(unsigned int extra_style)
{
   d_item_template.style |= extra_style;

   return;
}


// -------------------- Function Remove_style_parameters ------------
void Dialog_item::Remove_style_parameters(unsigned int remove_style)
{
   d_item_template.style &= ~remove_style;

   return;
}


// -------------------- Function Button_control_data_exchange -------
void Dialog_item::Button_control_data_exchange(CDataExchange* pDX)
{
   const int idc = d_item_template.id;

   switch (d_data_type)
      {
      case e_bool:
         {
         bool* value = static_cast<bool*>(d_value);
         BOOL temp_val = *value;

         DDX_Check(pDX, idc, temp_val);

         if (pDX->m_bSaveAndValidate)
            {
            *value = (temp_val) ? true : false;
            }
         }
         break;

      case e_int:
         DDX_Radio(pDX, idc, *(static_cast<int*>(d_value)));
         break;

      default:
         break;
      }

   return;
}


// -------------------- Function Box_data_exchange ------------------
void Dialog_item::Box_data_exchange(CDataExchange* pDX, CWnd* box)
{
   CComboBox* combo_box = 0;
   CListBox*  list_box  = 0;

   if (d_controltype == e_combobox)
      {
      combo_box = static_cast<CComboBox*>(box);
      }
   else
      {
      list_box = static_cast<CListBox*>(box);
      }

   DDX_Control(pDX, d_item_template.id, *box);

   if (pDX->m_bSaveAndValidate)
      {
      int index = (d_controltype == e_combobox) ?
                      combo_box->GetCurSel() : list_box->GetCurSel();
      if (index < 0)   // Force a valid value if
         {             // no selection was made.
         index = 0;
         }

      if (d_data_type == e_int)
         {
         *(static_cast<int*>(d_value)) = index;
         }
      else
         {
         if (d_controltype == e_combobox)
            {
            combo_box->GetLBText(index,
                                (*(static_cast<CString*>(d_value))));
            }
         else
            {
            list_box->GetText(index,
                                (*(static_cast<CString*>(d_value))));
            }
         }
      }

   return;
}


// -------------------- Function Edit_control_data_exchange ---------
void Dialog_item::Edit_control_data_exchange(CDataExchange* pDX)
{
   const int idc = d_item_template.id;

   switch (d_data_type)
      {
      case e_cstring:
         {
         CString* str = static_cast<CString*>(d_value);
         DDX_Text(pDX, idc, *str);
         const int length_limit = d_max.i;
         if (length_limit > 0)
            {
            DDV_MaxChars(pDX, *str, length_limit);
            }
         }
         break;

      case Dialog_item::e_int:
         {
         int* value = static_cast<int*>(d_value);
         DDX_Text(pDX, idc, *value);

         const int min = d_min.i;
         const int max = d_max.i;

         if (min != max)
            {
            DDV_MinMaxInt(pDX, *value, min, max);
            }
         }
         break;

      case Dialog_item::e_float:
         {
         float* value = static_cast<float*>(d_value);
         DDX_Text(pDX, idc, *value);

         const float min = d_min.f;
         const float max = d_max.f;

         if (min != max)
            {
            DDV_MinMaxFloat(pDX, *value, min, max);
            }
         }
         break;

      case Dialog_item::e_double:
         {
         double* value = static_cast<double*>(d_value);
         DDX_Text(pDX, idc, *value);

         const double min = d_min.d;
         const double max = d_max.d;

         if (min != max)
            {
            DDV_MinMaxDouble(pDX, *value, min, max);
            }
         }
         break;

      case Dialog_item::e_unsigned_int:
         {
         unsigned int* value =
                         static_cast<unsigned int*>(d_value);
         DDX_Text(pDX, idc, *value);

         const unsigned int min = d_min.ui;
         const unsigned int max = d_max.ui;

         if (min != max)
            {
            DDV_MinMaxUInt(pDX, *value, min, max);
            }
         }
         break;

      case Dialog_item::e_byte:
         {
         BYTE* value = static_cast<BYTE*>(d_value);
         DDX_Text(pDX, idc, *value);

         const BYTE min = d_min.by;
         const BYTE max = d_max.by;

         if (min != max)
            {
            DDV_MinMaxByte(pDX, *value, min, max);
            }
         }
         break;

      case Dialog_item::e_short:
         {
         short* value = static_cast<short*>(d_value);
         DDX_Text(pDX, idc, *value);

         const short min = d_min.si;
         const short max = d_max.si;

         if (min != max)
            {
            DDV_MinMaxInt(pDX, *value, min, max);
            }
         }
         break;

      default:   // do nothing
         break;
      }

   return;
}


// -------------------- Function Write_to_template_buffer -----------
unsigned char* Dialog_item::Write_to_template_buffer(
                                             unsigned char* pc) const
{
   // This code is based on code in MSDN's dlgtempl example.

   pc = (BYTE*)(((DWORD)pc + 3) & ~3);  // The pointer must be DWORD
                                        // aligned.
   memcpy(pc, (const void *)&d_item_template,
          sizeof(DLGITEMTEMPLATE));
   pc += sizeof(DLGITEMTEMPLATE);

   *(WORD*)pc = 0xFFFF; // Flag that next word will be a control type
   pc += sizeof(WORD);

   *(WORD*)pc = static_cast<WORD>(d_controltype); // Atom value for
   pc += sizeof(WORD);                            // control

   // Transfer the caption even if it's an empty string.

   pc = Write_string_to_buffer(d_caption, pc);

   if (pc)
      {
      *(WORD*)pc = 0;      // Number of bytes of data for control.
      pc += sizeof(WORD);
      }

   return pc;
}


// -------------------- Function Write_string_to_buffer -------------
// Function to make a UNICODE version of 'str' and copy it to 'pc'.
// 'pc' is assumed to point to enough space to hold the written
// string.
// RETURN: 'pc' incremented by the number of chars written to 'pc'.
//         If the call to operator new fails, 0 is returned.
// ------------------------------------------------------------------
//static
unsigned char* Dialog_item::Write_string_to_buffer(const char* str,
                                                   unsigned char* pc)
{
   const int n_chars = strlen(str) + 1;

   WCHAR* caption = 0;

   try
      {
      caption = new WCHAR[n_chars + 4];  // Some extra, to avoid
      }                                  // allocating 0.

   catch (CMemoryException* e)
      {
      e->Delete();
      pc = 0;
      }

   if (pc)
      {
      int n = MultiByteToWideChar(CP_ACP, 0, str, -1, caption,
                                  n_chars) * sizeof(WCHAR);
                               // Convert char count to byte count.
      memcpy(pc, caption, n);
      delete []caption;        // was bug in MSDN code (missing []).
      pc += n;
      }

   return pc;
}


// -------------------- Function File_or_directory_select -----------
bool Dialog_item::File_or_directory_select(CWnd* window,
                                    int dialog_type,
                                    const CString& dialog_title,
                                    const CString& file_filter,
                                    const CString& default_extension)
{
   bool okay = false;

   CString* p_str = static_cast<CString*>(d_value);

   if (dialog_type != Dynamic_dialog::e_select_directory)  // select
      {                                                    // file?
      const BOOL read_file = (dialog_type) ? FALSE : TRUE;

      const char* filter = 0;
      if (file_filter.GetLength() > 1)
         {
         filter = (file_filter);
         }

      const char* extension = 0;
      if (default_extension.GetLength() > 1)
         {
         extension = (default_extension);
         }

      BOOL mode = (read_file) ?
                             OFN_FILEMUSTEXIST : OFN_OVERWRITEPROMPT;
      mode |= OFN_HIDEREADONLY;

      CFileDialog fdialog(read_file, extension, (*p_str), mode,
                          filter);
      fdialog.m_ofn.lpstrTitle = dialog_title;

      if (fdialog.DoModal() == IDOK)
         {
         okay = true;
         *p_str = fdialog.GetPathName();
         }
      }
   else                                   // Selecting a directory.
      {
      char directory_name[_MAX_PATH + 4];

      okay = Select_directory(window, dialog_title, directory_name);

      if (okay)
         {
         *p_str = directory_name;

         okay = Find_fullpathname(*p_str, directory_name);

         if (okay)
            {
            // Check the directory_name really is a directory.

            const DWORD attrib = ::GetFileAttributes(directory_name);
            if ((attrib != 0xFFFFFFFF) &&
                (attrib & FILE_ATTRIBUTE_DIRECTORY))
               {
               *p_str = directory_name;  // Save in member variable.
               }
            else
               {
               okay = false;
               ::MessageBox(NULL, directory_name,
                            "Not a valid directory!",
                            MB_OK | MB_ICONINFORMATION);
               }
            }
         }
      }

   return okay;
}


// -------------------- Function Select_directory -------------------
// This code is based on the example in "Querying the User for a
// Directory under Win95 or NT4" by Brett Robichaud,
// Windows Developer's Journal, Nov. 1997, pg. 51-53.
// ------------------------------------------------------------------
//static
bool Dialog_item::Select_directory(CWnd* window,
                                   const char* dialog_title,
                                   char* directory_name)
{
   bool okay = false;

   LPMALLOC pMalloc;

   if (::SHGetMalloc(&pMalloc) == NOERROR)  // get shell's default
      {                                     // allocator.
      BROWSEINFO bi;
      bi.pidlRoot = NULL;
      bi.lpfn = NULL;
      bi.lParam = 0;
      bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
      bi.hwndOwner = window->GetSafeHwnd();
      bi.pszDisplayName = directory_name;
      bi.lpszTitle = dialog_title;

      LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
      if (pidl != NULL)
         {
         if (::SHGetPathFromIDList(pidl, directory_name))
            {
            okay = true;
            }
         pMalloc->Free(pidl);
         }
      pMalloc->Release();  // release the shell's allocator
      }

   return okay;
}


// -------------------- Function Find_fullpathname ------------------
//static
bool Dialog_item::Find_fullpathname(const char* relative_path,
                                    char* full_path)
{
   const bool okay = (::_fullpath(full_path, relative_path,
                                  _MAX_PATH) != NULL) ? true : false;
   if (!okay)
      {
      ::MessageBox(NULL, relative_path, "Not a valid relative path",
                   MB_OK | MB_ICONINFORMATION);
      }
   else
      {
      // For consistency, always ensure that the directory has a
      // trailing '\'.

      const char backslash = '\\';

      char* append_here =
                   full_path + static_cast<int>(::strlen(full_path));
      const char last_char = *(append_here - 1);

      if (last_char != backslash)
         {
         if (last_char == '/')
            {
            append_here--;         // will replace '/' with backslash
            }
         *append_here++ = backslash;
         *append_here = '\0';
         }
      }

   return okay;
}

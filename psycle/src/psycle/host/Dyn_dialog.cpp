// modified code from http://www.drdobbs.com/windows/building-mfc-dialogs-at-runtime/184405811
//
//:  Implementation of the Dynamic_dialog class.

#include <psycle/host/detail/project.hpp>
#include "Dyn_dialog.h"


// -------------------- Function Dynamic_dialog ---------------------
// ------------------------------------------------------------------
Dynamic_dialog::Dynamic_dialog(const char* title, int width,
                               int height, int position_x,
                               int position_y) :
   d_n_combo_boxes(0), d_combo_boxes(0), d_n_list_boxes(0),
   d_list_boxes(0), d_n_browse_used(0), d_font_name("MS Sans Serif"),
   d_font_size(8), d_first_time(true), d_template_buffer(0)
{
   // The combo boxes, list boxes and associated strings are members
   // of this class, rather than in Dialog_item, for several reasons:
   // 0. There is no defined operator=() for CComboBox or CListBox.
   //    This means that the Dialog_item::operator=() would be
   //    incomplete. So, the CComboBox and CListBox instances could
   //    only have their strings added once all the items have been
   //    defined, which is when all uses of Dialog_item::operator=()
   //    are complete.
   // 1. It is wasteful to have one CComboBox and one CListBox member
   //    in each instance of Dialog_item since relatively few
   //    instances are actually combo or list boxes.
   // 2. It is also wasteful to have a CList<CString> instance in
   //    each Dialog_item, so it makes sense to keep the strings with
   //    the CComboBoxes and CListBoxes.

   d_title = (title) ? title : "Dialog";

   // Initialize the DLGTEMPLATE structure.

   Set_dialog_style(WS_CAPTION | WS_VISIBLE | WS_DLGFRAME | WS_POPUP
                          | DS_SETFONT | WS_SYSMENU); // DS_MODALFRAME
   d_dialog_template.dwExtendedStyle = 0;
   d_dialog_template.cdit = 0;
   Set_dialog_position(position_x, position_y);
   Set_dialog_size(width, height);

   return;
}


// -------------------- Function ~Dynamic_dialog --------------------
//virtual
Dynamic_dialog::~Dynamic_dialog()
{
   if (d_template_buffer)
      {
      delete []d_template_buffer;
      }

   return;
}

// -------------------- Message Map ---------------------------------

BEGIN_MESSAGE_MAP(Dynamic_dialog, CDialog)
   ON_BN_CLICKED(Dynamic_dialog::e_first_browse_idc,     OnBrowse0)
   ON_BN_CLICKED(Dynamic_dialog::e_first_browse_idc + 1, OnBrowse1)
   ON_BN_CLICKED(Dynamic_dialog::e_first_browse_idc + 2, OnBrowse2)
   ON_BN_CLICKED(Dynamic_dialog::e_first_browse_idc + 3, OnBrowse3)
END_MESSAGE_MAP()


// -------------------- OnBrowse? functions -------------------------

void Dynamic_dialog::OnBrowse0() { Browse(0); }
void Dynamic_dialog::OnBrowse1() { Browse(1); }
void Dynamic_dialog::OnBrowse2() { Browse(2); }
void Dynamic_dialog::OnBrowse3() { Browse(3); }


// -------------------- Function Browse -----------------------------
// ------------------------------------------------------------------
void Dynamic_dialog::Browse(int button)
{
   // Get current values into member variables - so that any other
   // changes made by the user before clicking the Browse button
   // aren't lost by the UpdateData(FALSE) call.

   UpdateData(TRUE);

   // Each d_item_index element is the index of the associated
   // edit control, and hence CString variable.

   if (d_items[d_item_index[button]].
                File_or_directory_select(this, d_browse_type[button],
                                         d_browse_titles[button],
                                         d_file_filters[button],
                                         d_file_extension[button]))
      {
      UpdateData(FALSE);  // update values displayed in dialog box
      }

   return;
}


// -------------------- Function Set_dialog_position ----------------
void Dynamic_dialog::Set_dialog_position(int x_position,
                                         int y_position)
{
   d_dialog_template.x = static_cast<short>(x_position);
   d_dialog_template.y = static_cast<short>(y_position);

   return;
}


// -------------------- Function Set_dialog_size --------------------
void Dynamic_dialog::Set_dialog_size(int width, int height)
{
   d_dialog_template.cx = static_cast<short>(width);
   d_dialog_template.cy = static_cast<short>(height);

   return;
}


// -------------------- Function Set_dialog_style -------------------
void Dynamic_dialog::Set_dialog_style(unsigned int dialog_style)
{
   d_dialog_template.style = dialog_style;

   return;
}


// -------------------- Function Set_dialog_font --------------------
void Dynamic_dialog::Set_dialog_font(const char* font_name, int size)
{
   d_font_name = font_name;
   d_font_size = static_cast<WORD>(size);

   return;
}


// -------------------- Function Set_dialog_title -------------------
void Dynamic_dialog::Set_dialog_title(const char* title)
{
   d_title = title;

   return;
}


// -------------------- Function Add_to_last_items_styles -----------
void Dynamic_dialog::Add_to_last_items_styles(
                                            unsigned int extra_style)
{
   const unsigned int n_items = d_items.GetSize();

   if (n_items)  // There is an item to modify?
      {
      d_items[n_items - 1].Add_style_parameters(extra_style);
      }

   return;
}


// -------------------- Function Remove_from_last_items_styles ------
void Dynamic_dialog::Remove_from_last_items_styles(
                                           unsigned int remove_style)
{
   const unsigned int n_items = d_items.GetSize();

   if (n_items)  // There is an item to modify?
      {
      d_items[n_items - 1].Remove_style_parameters(remove_style);
      }

   return;
}


// -------------------- Function Add_item ---------------------------
// Function to add the new item to the d_items CArray.
// RETURN: The supplied id for the item, or 0 if the item cannot be
//         added.
// ------------------------------------------------------------------
WORD Dynamic_dialog::Add_item(WORD id, Dialog_item& item)
{
   try
      {
      d_items.Add(item);
      }

   catch (CMemoryException* e)
      {
      id = 0;
      e->Delete();
      }

   return id;
}


// -------------------- Function Add_static_text --------------------
unsigned int Dynamic_dialog::Add_static_text(int left, int top,
                                          int width, int height,
                                          const char* text,
                                          e_text_alignment alignment,
                                          WORD id)
{
   // ss_align elements must correspond to the e_text_alignment
   // enumerations.

   static const unsigned int ss_align[] =
      { SS_LEFT, SS_RIGHT, SS_CENTER, SS_LEFT | SS_CENTERIMAGE,
        SS_RIGHT | SS_CENTERIMAGE, SS_CENTER | SS_CENTERIMAGE };

   id = Assign_id(id);

   Dialog_item item;
   item.Define_static_text(id, left, top, width, height, text,
                           ss_align[alignment]);

   return Add_item(id, item);
}


// -------------------- Function Add_prompt_and_unit_strings --------
// Function to add the prompt and unit strings to the dialog for edit
// controls, list boxes and combo boxes. The prompt and unit strings
// appear to the left and right of the main control, respectively,
// aligned with the bottom of the control. (For list boxes, the
// strings are placed just down vertically from the top of the
// control.)
// ------------------------------------------------------------------
bool Dynamic_dialog::Add_prompt_and_unit_strings(
                                              const char* prompt_str,
                                              int prompt_left,
                                              const char* unit_str,
                                              int edit_control_left,
                                              int top, int width,
                                              int height)
{
   bool okay = true;

   // Align the text with the bottom of the edit control. Text is
   // 8 dialog units high, according to the Prosise book.

   height -= 8;
   if (height > 0)
      {
      top += height;
      }

   if ((prompt_str) &&
       (!Add_static_text(prompt_left, top, edit_control_left, 10,
                         prompt_str)))
      {
      okay = false;
      }

   if ((okay) && (unit_str))
      {
      // Text chars are 4 dialog units wide. Pad by one character.
      const int unit_width = (1 + strlen(unit_str)) << 2;

      edit_control_left += width + 3;  // left edge for text.

      if (!Add_static_text(edit_control_left, top, unit_width, 10,
                           unit_str))
         {
         okay = false;
         }
      }

   return okay;
}


// -------------------- Function Add_edit_control -------------------
unsigned int Dynamic_dialog::Add_string_edit_control(const char* default_text,
                                                     int left, int top,int width, int height,                                                     
                                                     int max_length,
                                                     unsigned int extra_styles,
                                                     WORD id)
{  
    id = Assign_id(id);
    CString* str = default_text ? new CString(default_text) : new CString("");
    value_strings[id] = str;
    Dialog_item item;    
    item.Define_edit_control(id, left, top, width, height, str, max_length);
    item.Add_style_parameters(extra_styles);
    id = Add_item(id, item);
    return id;
}


// -------------------- Function Add_pushbutton ---------------------
unsigned int Dynamic_dialog::Add_pushbutton(const char* label,
                                            WORD id, int left,
                                            int top, int width,
                                            int height)
{
   id = Assign_id(id);

   Dialog_item item;
   item.Define_pushbutton(id, left, top, width, height, label);

   return Add_item(id, item);
}


// -------------------- Function Add_OK_button ----------------------
unsigned int Dynamic_dialog::Add_OK_button(int left, int top,
                                           int width, int height)
{
   unsigned int id = Add_pushbutton("OK", IDOK, left, top, width,
                                    height);
   if (id)   // IDOK is 1.
      {
      Add_to_last_items_styles(BS_DEFPUSHBUTTON);
      }

   return id;
}


// -------------------- Function Add_Cancel_button ------------------
unsigned int Dynamic_dialog::Add_Cancel_button(int left, int top,
                                               int width, int height)
{
   // IDCANCEL is 2.
   return Add_pushbutton("Cancel", IDCANCEL, left, top, width,
                         height);
}


// -------------------- Function Add_Browse_button ------------------
unsigned int Dynamic_dialog::Add_Browse_button(
                                       e_browse_type dialog_type,
                                       const char* file_dialog_title,
                                       const char* file_filter,
                                       const char* default_extension,
                                       int left, int top, int width,
                                       int height, const char* label)
{
   unsigned int ret_val = 0;

   unsigned int i = d_items.GetSize();

   // Browsing only makes sense if the previous item has
   // a CString variable.

   if (i)    // There is a previous item?
      {
      --i;   // now i is the index of Dialog_item which should have
             // a CString.

      if (!d_items[i].Is_string_edit_control())
         {
         AfxMessageBox("Previous control is not a string edit"
                       " control.", MB_OK | MB_ICONWARNING);
         }
      else
         {
         if (d_n_browse_used >= e_n_browse_max)
            {
            AfxMessageBox("No more Browse buttons allowed.",
                          MB_OK | MB_ICONWARNING);
            }
         else             // Haven't used up all the buttons?
            {
            d_browse_type[d_n_browse_used] = dialog_type;
            if (file_dialog_title)
               {
               d_browse_titles[d_n_browse_used] = file_dialog_title;
               }
            d_file_filters[d_n_browse_used] = (file_filter) ?
                                                   file_filter : " ";
            d_file_extension[d_n_browse_used] = (default_extension) ?
                                             default_extension : " ";

            d_item_index[d_n_browse_used] = i;

            if (!label)
               {
               label = "Browse...";
               }

            const WORD id = static_cast<WORD>(e_first_browse_idc +
                                              d_n_browse_used);

            ret_val = Add_pushbutton(label, id, left, top, width,
                                     height);
            if (ret_val)
               {
               ++d_n_browse_used;
               }
            }
         }
      }

   return ret_val;
}


// -------------------- Function Add_checkbox -----------------------
unsigned int Dynamic_dialog::Add_checkbox(int left, int top,
                                          int width, int height,
                                          const char* label,
                                          bool* value, WORD id)
{
   id = Assign_id(id);

   Dialog_item item;
   item.Define_checkbox(id, left, top, width, height, label, value);

   return Add_item(id, item);
}


// -------------------- Function Add_first_radio_button -------------
unsigned int Dynamic_dialog::Add_first_radio_button(int left,
                                                   int top,
                                                   int width,
                                                   int height,
                                                   const char* label,
                                                   int* value,
                                                   WORD id)
{
   id = Assign_id(id);

   Dialog_item item;
   item.Define_first_radio_button(id, left, top, width, height,
                                  label, value);
   return Add_item(id, item);
}


// -------------------- Function Add_radio_button -------------------
unsigned int Dynamic_dialog::Add_radio_button(int left, int top,
                                              int width, int height,
                                              const char* label,
                                              WORD id)
{
   id = Assign_id(id);

   Dialog_item item;
   item.Define_radio_button(id, left, top, width, height, label);

   return Add_item(id, item);
}


// -------------------- Function Add_group_box ----------------------
unsigned int Dynamic_dialog::Add_group_box(int left, int top,
                                           int width, int height,
                                           const char* label,
                                           WORD id)
{
   id = Assign_id(id);

   Dialog_item item;
   item.Define_group_box(id, left, top, width, height, label);

   return Add_item(id, item);
}


// -------------------- Function Add_combo_or_list_box --------------
unsigned int Dynamic_dialog::Add_combo_or_list_box(bool is_combo,
                                              const char* prompt_str,
                                              int prompt_left,
                                              const char* unit_str,
                                              int left, int top,
                                              int width, int height,
                                              int* index,
                                              int n_entries,
                                              const char* entries[],
                                              WORD id)
{
   return Add_combo_or_list_box(is_combo, prompt_str, prompt_left,
                                unit_str, left, top, width, height,
                                index, 0, n_entries, entries, 0,
                                false, id);
}


// -------------------- Function Add_combo_or_list_box --------------
unsigned int Dynamic_dialog::Add_combo_or_list_box(bool is_combo,
                                              const char* prompt_str,
                                              int prompt_left,
                                              const char* unit_str,
                                              int left, int top,
                                              int width, int height,
                                              CString* str,
                                              int n_entries,
                                              const char* entries[],
                                              bool sort, WORD id)
{
   return Add_combo_or_list_box(is_combo, prompt_str, prompt_left,
                                unit_str, left, top, width, height,
                                0, str, n_entries, entries, 0, sort,
                                id);
}


// -------------------- Function Add_combo_or_list_box --------------
unsigned int Dynamic_dialog::Add_combo_or_list_box(bool is_combo,
                                             const char* prompt_str,
                                             int prompt_left,
                                             const char* unit_str,
                                             int left, int top,
                                             int width, int height,
                                             CString* str,
                                             const CStringList& list,
                                             bool sort, WORD id)
{
   return Add_combo_or_list_box(is_combo, prompt_str, prompt_left,
                                unit_str, left, top, width, height,
                                0, str, 0, 0, &list, sort, id);
}


// -------------------- Function Add_combo_or_list_box --------------
unsigned int Dynamic_dialog::Add_combo_or_list_box(bool is_combo,
                                             const char* prompt_str,
                                             int prompt_left,
                                             const char* unit_str,
                                             int left, int top,
                                             int width, int height,
                                             int* index,
                                             const CStringList& list,
                                             WORD id)
{
   return Add_combo_or_list_box(is_combo, prompt_str, prompt_left,
                                unit_str, left, top, width, height,
                                index, 0, 0, 0, &list, false, id);
}


// -------------------- Function Add_combo_or_list_box --------------
// Function to add a combo box or list box to the dialog. One of
// 'index' or str must be 0. Also, one of n_entries or CStringList
// must be 0.
// ------------------------------------------------------------------
unsigned int Dynamic_dialog::Add_combo_or_list_box(bool is_combo,
                                             const char* prompt_str,
                                             int prompt_left,
                                             const char* unit_str,
                                             int left, int top,
                                             int width, int height,
                                             int* index,
                                             CString* str,
                                             int n_entries,
                                             const char* entries[],
                                             const CStringList* list,
                                             bool sort, WORD id)
{
   const int first_index = d_strings.GetCount();

   // Copy the strings to d_strings.

   int  last_index = first_index + n_entries - 1;
   bool okay = true;

   if (n_entries > 0)  // Copy from the entries[] array.
      {
      int i = -1;
      while ((++i < n_entries) && (okay))
         {
         d_strings.AddTail(entries[i]);
         }
      }
   else                // Copy the CStringList directly
      {
      d_strings.AddTail(const_cast<CStringList*>(list));
      last_index = first_index + list->GetCount() - 1;
      }

   enum { e_height = 12 };  // height of an unexpanded combo box.

   if ((!okay) ||
       (!Add_prompt_and_unit_strings(prompt_str, prompt_left,
                                     unit_str, left, top, width,
                                     e_height)))
      {
      id = 0;    // flag failure
      }
   else
      {
      id = Assign_id(id);

      Dialog_item item;

      if (index)
         {
         item.Define_combo_or_list_box(is_combo, id, left, top,
                                       width, height, index,
                                       first_index, last_index,
                                       sort);
         }
      else
         {
         item.Define_combo_or_list_box(is_combo, id, left, top,
                                       width, height, str,
                                       first_index, last_index,
                                       sort);
         }

      id = Add_item(id, item);
      if (id)                 // Added the box successfully?
         {                    // Update count of combo or list boxes.
         if (is_combo)
            {
            ++d_n_combo_boxes;
            }
         else
            {
            ++d_n_list_boxes;
            }
         }
      }

   return id;
}


// -------------------- Function Assign_id --------------------------
// Function to assign an unique ID for a control. If the argument to
// this function is not zero, then the value is returned unchanged.
// Otherwise, the ID is set to 5000 plus the current count of
// controls in the dialog.
// ------------------------------------------------------------------
WORD Dynamic_dialog::Assign_id(WORD id) const
{
   enum { e_first_idc_number = 5000 };

   if (!id)
      {
      id = static_cast<WORD>(e_first_idc_number + d_items.GetSize());
      }

   return id;
}


// -------------------- Function DoModal ----------------------------
//virtual
int Dynamic_dialog::DoModal()
{
   bool okay = true;

   if (d_n_combo_boxes)   // Allocate required number of combo boxes.
      {
      try
         {
         d_combo_boxes = new CComboBox[d_n_combo_boxes];
         }

      catch (CMemoryException* e)
         {
         d_combo_boxes = 0;
         e->Delete();
         okay = false;
         }
      }

   if ((okay) && (d_n_list_boxes))  // Allocate required number of
      {                             // list boxes.
      try
         {
         d_list_boxes = new CListBox[d_n_list_boxes];
         }

      catch (CMemoryException* e)
         {
         d_list_boxes = 0;
         e->Delete();
         okay = false;
         }
      }

   // If DoModal() is called multiple times for a dialog, nothing in
   // the DLGTEMPLATE (or DLGITEMTEMPLATE's) has changed. There is no
   // need to repeat the Build_template_buffer() and
   // InitModalIndirect() calls. Under a debug build the application
   // ASSERTs if this code is called more than once.

   if ((okay) && (d_first_time))
      {
      // Ensure that the dialog template has the correct number of
      // controls.

      d_dialog_template.cdit =
                      static_cast<unsigned short>(d_items.GetSize());

      if ((!Build_template_buffer(d_dialog_template, d_items,
                                  d_title, d_font_name, d_font_size,
                                  &d_template_buffer)) ||
          //(!InitModalIndirect((DLGTEMPLATE*)d_template_buffer)))
          (!CreateIndirect((DLGTEMPLATE*)d_template_buffer)))
         {
         okay = false;
         }

      d_first_time = false;
      }

   // For consistency with CDialog::DoModal(), return -1 if something
   // failed above.

   const int return_value = 0; //(okay) ? CDialog::DoModal() : -1;
   // const int return_value = (okay) ? CDialog::DoModal() : -1;

   /*
   if (d_n_combo_boxes)
      {
      delete []d_combo_boxes;
      d_combo_boxes = 0;
      }

   if (d_n_list_boxes)
      {
      delete []d_list_boxes;
      d_list_boxes = 0;
      }*/

   return return_value;
}


// -------------------- Function Build_template_buffer --------------
// Function to build the dialog box template in memory, to be passed
// to CDialog::InitModalIndirect(). The template consists of a
// DLGTEMPLATE struct followed by control information.
// See "Templates in Memory" in the Windows Platform SDK for details.
// RETURN: 'true' if the template is built correctly.
// ------------------------------------------------------------------
//static
bool Dynamic_dialog::Build_template_buffer(
                     const DLGTEMPLATE& dialog_template,
                     CArray<Dialog_item, Dialog_item&>& dialog_items,
                     const char* dialog_caption,
                     const char* dialog_font_name,
                     const WORD dialog_font_size,
                     unsigned char** buffer)
{
   // This code is based on code in MSDN's dlgtempl example.

   // Calculate the required number of bytes to hold the dialog
   // template header in memory. The three WORD's are for specifying
   // that the dialog has no menu, that the predefined dialog box
   // class should be used for the dialog, and for the font size.
   // The last term is for the Unicode versions of the dialog title
   // and the font name.

   int buf_size = sizeof(DLGTEMPLATE) + 3 * sizeof(WORD) +
                  sizeof(WCHAR) * (strlen(dialog_caption) + 1 +
                  strlen(dialog_font_name) + 1);

   // Adjust buffer size so that first control will be DWORD-aligned.

   buf_size = (buf_size + 3) & ~3;

   // Now add in space required for all of the controls in the dialog

   const int n_dialog_items = dialog_items.GetSize();

   for (int i = 0; i < n_dialog_items; ++i)
      {
      // The last term is for the Unicode version of the control's
      // title. Two WORD's specify the control's type (button, list
      // box etc.); the third specifies that the creation data array
      // is 0.

      int n_bytes = sizeof(DLGITEMTEMPLATE) + 3 * sizeof(WORD) +
               sizeof(WCHAR) *
                     (dialog_items[i].Get_caption().GetLength() + 1);

      if (i != (n_dialog_items - 1))   // Last control doesn't need
         {                             // extra bytes
         n_bytes = (n_bytes + 3) & ~3; // Adjust so next control will
         }                             // be DWORD aligned.

      buf_size += n_bytes;
      }

   // Now allocate the memory to hold the entire dialog template.

   if (*buffer)
      {
      delete [](*buffer);
      }

   bool okay = true;

   try
      {
      *buffer = new unsigned char[buf_size];
      }

   catch (CMemoryException* e)
      {
      *buffer = 0;
      e->Delete();
      okay = false;
      }

   // Now fill the entire dialog template.

   if (okay)
      {
      unsigned char* pdest = *buffer;
      memset(pdest, 0, buf_size);

      // Copy the DLGTEMPLATE struct to the buffer

      memcpy(pdest, &dialog_template, sizeof(DLGTEMPLATE));
      pdest += sizeof(DLGTEMPLATE);
      *(WORD*)pdest = 0;            // The dialog has no menu.
      *(WORD*)(pdest + 1) = 0;      // The default window class is
      pdest += 2 * sizeof(WORD);    // required.

      // Make a Unicode version of the dialog title, and add it.

      pdest = Dialog_item::Write_string_to_buffer(dialog_caption,
                                                  pdest);

      if (pdest)  // Wrote the dialog title successfully?
         {
         // Add the font size and font name (as Unicode).

         *(WORD*)pdest = dialog_font_size;
         pdest += sizeof(WORD);
         pdest = Dialog_item::Write_string_to_buffer(
                                            dialog_font_name, pdest);
         }

      if (!pdest)      // failed to write both strings?
         {
         okay = false;
         }

      // Transfer the data for each one of the item templates.

      int i = -1;
      while ((okay) && ((++i) < n_dialog_items))
         {
         pdest = dialog_items[i].Write_to_template_buffer(pdest);
         if (!pdest)
            {
            okay = false;
            }
         }
      }

   return okay;
}


// -------------------- Function OnInitDialog -----------------------
// Function to override CDialog::OnInitDialog(). This function calls
// the base class' version then sets up the strings for each combo
// box and list box.
// ------------------------------------------------------------------
//virtual
BOOL Dynamic_dialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   POSITION pos = d_strings.GetHeadPosition();

   CComboBox* pcombo = d_combo_boxes;
   CListBox*  plist  = d_list_boxes;

   for (int i = 0; i < d_items.GetSize(); ++i)
      {
      const Dialog_item::controltype type =
                                       d_items[i].Get_control_type();
      if (type == Dialog_item::e_combobox)
         {
         d_items[i].Add_strings_to_box(d_strings, pos, pcombo++);
         }
      else if (type == Dialog_item::e_listbox)
         {
         d_items[i].Add_strings_to_box(d_strings, pos, plist++);
         }
      }

   return TRUE;
}


// -------------------- Function DoDataExchange ---------------------
// Function to override the base class' version of this function. The
// base class version (CDialog::DoDataExchange()) is called first,
// then data exchange and validation is performed as required for
// each control in the dialog.
// ------------------------------------------------------------------
//virtual
void Dynamic_dialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);

   CComboBox* pcombo = d_combo_boxes;
   CListBox*  plist  = d_list_boxes;

   for (int i = 0; i < d_items.GetSize(); ++i) // For each control...
      {
      switch (d_items[i].Get_control_type())   // Handle according
         {                                     // to its type.
         case Dialog_item::e_editcontrol:
            d_items[i].Edit_control_data_exchange(pDX);
            break;

         case Dialog_item::e_button:
            d_items[i].Button_control_data_exchange(pDX);
            break;

         case Dialog_item::e_combobox:
            d_items[i].Box_data_exchange(pDX, pcombo++);
            break;

         case Dialog_item::e_listbox:
            d_items[i].Box_data_exchange(pDX, plist++);
            break;

         default:   // Do nothing for static text.
            break;
         }
      }

   return;
}

// modified code from http://www.drdobbs.com/windows/building-mfc-dialogs-at-runtime/184405811
//
//:  Interface to the Dynamic_dialog class.

#ifndef DYNAMIC_DIALOG_H_

#define DYNAMIC_DIALOG_H_

#include <afxtempl.h>       // for CArray
#include "Dialog_item.h"



// -------------------- Class Dynamic_dialog ------------------------
// Class to allow dialogs to be built at runtime.
// ------------------------------------------------------------------


class Dynamic_dialog : public CDialog
{
public:
   // Overall dialog setup functions:
   // ==============================

   // ----------------- Function Dynamic_dialog ---------------------
   // Constructor for the Dynamic_dialog class.
   // title:        Text displayed in the dialog box's title bar. The
   //               text is "Dialog" if the default parameter value
   //               is used here.
   // width:        Width of the dialog, in dialog box units. Each
   //               text character is four units wide in the dialog
   //               box font.
   // height:       Height of the dialog, in dialog box units. A text
   //               character is eight units high in the dialog box
   //               font.
   //               (The default font is 8-point MS Sans Serif.)
   // position_x,y: Position where the dialog is displayed relative
   //               to the upper left corner of its owner's client
   //               area, in dialog units. When the default position
   //               (0, 0) is used, the dialog is centered in the
   //               owner's client area.
   // ---------------------------------------------------------------
   Dynamic_dialog(const char* title = 0, int width = 100,
                  int height = 100, int position_x = 0,
                  int position_y = 0);


   // ----------------- Function ~Dynamic_dialog --------------------
   virtual ~Dynamic_dialog();
  

   // ----------------- Function Set_dialog_font --------------------
   // Function to set the overall dialog font and its size. If this
   // function is not called, the default of "MS Sans Serif" with
   // size 8 is used.
   // ---------------------------------------------------------------
   void Set_dialog_font(const char* font_name, int size);


   // ----------------- Function Set_dialog_position ----------------
   // Function to determine the position of the dialog box within its
   // owner's client area. See constructor's comments for parameter
   // details.
   // ---------------------------------------------------------------
   void Set_dialog_position(int x_position, int y_position);


   // ----------------- Function Set_dialog_size --------------------
   // Function to set the overall dialog size, in dialog units.
   // See constructor's comments for parameter details.
   // ---------------------------------------------------------------
   void Set_dialog_size(int width, int height);


   // ----------------- Function Set_dialog_style -------------------
   // Function to set the overall dialog style. If this function is
   // not called, the default of
   // WS_CAPTION | WS_VISIBLE | WS_DLGFRAME | WS_POPUP |
   // DS_MODALFRAME | DS_SETFONT | WS_SYSMENU is used.
   // ---------------------------------------------------------------
   void Set_dialog_style(unsigned int dialog_style);


   // ----------------- Function Set_dialog_title -------------------
   // Function to set text displayed in the dialog box's title bar.
   // ---------------------------------------------------------------
   void Set_dialog_title(const char* title);


   // Functions to add controls to the dialog:
   // =======================================

   // Controls are displayed starting from 'left', 'top', with size
   // 'width' by 'height', all in dialog units.
   // The tab order is the same as the order in which controls are
   // added to the dialog; the first control (typically the "OK"
   // button) gets the focus when the dialog is first displayed.
   // Generally there is no need to supply a value for the resource
   // ID of the control unless the control has an associated message
   // map function that you will provide. If you supply your own id
   // to a control, avoid values 1 and 2 (used by OK and Cancel) and
   // also values starting from 5000, which this class uses for its
   // automatically-generated ID's.
   // The return value for an Add...() function is the resource ID of
   // the control, or 0 if an error was encountered in adding the
   // control.

   // Edit controls, combo boxes and list boxes all have these
   // parameters:
   // prompt_str:  String displayed to the left of the control. The
   //              string is aligned vertically so that the bottom of
   //              the text is aligned with the bottom of the
   //              control. If the pointer is 0, no string is
   //              displayed.
   // prompt_left: Horizontal starting position for 'prompt_str', if
   //              used.
   // unit_str:    String displayed to the right of the control. The
   //              string starts 3 units right of the right edge of
   //              the control, and is aligned vertically as for
   //              'prompt_str'. If the pointer is 0, no string is
   //              displayed.
   // id:          Resource ID to be associated with the control. If
   //              the value is 0, the class will assign its own
   //              resource ID.


   // ----------------- Function Add_combo_or_list_box --------------
   // Funtions to add either a list box (is_combo == false) or a
   // combo box to the dialog. Overloads are provided for supplying
   // the strings to be displayed as either a CStringList or
   // 'n_entries' elements of an array of const char*. Overloads are
   // also provided for controlling either an integer 'index' for the
   // string selection, or controlling a CString 'str'.
   // sort: A combo box or list box setting a CString value will
   //       display the strings sorted alphabetically if 'sort' is
   //       true.
   // ---------------------------------------------------------------
   unsigned int Add_combo_or_list_box(bool is_combo,
                                      const char* prompt_str,
                                      int prompt_left,
                                      const char* unit_str,
                                      int left, int top, int width,
                                      int height, int* index,
                                      int n_entries,
                                      const char* entries[],
                                      WORD id = 0);

   unsigned int Add_combo_or_list_box(bool is_combo,
                                      const char* prompt_str,
                                      int prompt_left,
                                      const char* unit_str,
                                      int left, int top, int width,
                                      int height, int* index,
                                      const CStringList& list,
                                      WORD id = 0);

   unsigned int Add_combo_or_list_box(bool is_combo,
                                      const char* prompt_str,
                                      int prompt_left,
                                      const char* unit_str,
                                      int left, int top, int width,
                                      int height, CString* str,
                                      int n_entries,
                                      const char* entries[],
                                      bool sort = false,
                                      WORD id = 0);

   unsigned int Add_combo_or_list_box(bool is_combo,
                                      const char* prompt_str,
                                      int prompt_left,
                                      const char* unit_str,
                                      int left, int top, int width,
                                      int height, CString* str,
                                      const CStringList& list,
                                      bool sort = false,
                                      WORD id = 0);


   // ----------------- Function Add_edit_control -------------------
   // Function to define an edit control for CString 'str'.
   // max_length: >0 to limit the length of the string; <= 0 for no
   //             length limit.
   // ---------------------------------------------------------------
   unsigned int Add_string_edit_control(const char* default_text,
                                        int left, int top, int width, int height,                                        
                                        int max_length = 0,
                                        unsigned int extra_styles = ES_LEFT,
                                        WORD id = 0);


   // ----------------- Function Add_edit_control -------------------
   // Function to define an edit control for numeric value 'value'.
   // All the "usual" integer and floating point types are supported.
   // min, max: values with the required limits for DDV/DDX range
   //           checking. If these limits are equal, then no range
   //           checking is performed.
   // ---------------------------------------------------------------
   template<class T>
   unsigned int Add_edit_control(const char* prompt_str,
                                 int prompt_left,
                                 const char* unit_str, int left,
                                 int top, int width, int height,
                                 T* value, T min = 0, T max = 0,
                                 WORD id = 0)
   {
      if (!Add_prompt_and_unit_strings(prompt_str, prompt_left,
                                 unit_str, left, top, width, height))
         {
         id = 0;             // flag failure
         }
      else                   // successfully added any strings
         {
         id = Assign_id(id);

         Dialog_item it;
         it.Define_edit_control(id, left, top, width, height, value,
                                min, max);
         id = Add_item(id, it);
         }

      return id;
   }


   // Pushbuttons:
   // -----------
   // Functions to add pushbuttons to a dialog.


   // ----------------- Function Add_Cancel_button ------------------
   // Function to add a "Cancel" button to the dialog.
   // RETURN: IDCANCEL (2) for success, otherwise 0.
   // ---------------------------------------------------------------
   unsigned int Add_Cancel_button(int left, int top, int width = 50,
                                  int height = 14);


   // ----------------- Function Add_OK_button ----------------------
   // Function to add an "OK" button to the dialog.
   // RETURN: IDOK (1) for success, otherwise 0.
   // ---------------------------------------------------------------
   unsigned int Add_OK_button(int left, int top, int width = 50,
                              int height = 14);


   // ----------------- Function Add_pushbutton ---------------------
   // Function to add a pushbutton with 'label' text. Generally this
   // function is called from a class derived from Dynamic_dialog,
   // and the derived class has a message map entry associated with
   // resource ID 'id'.
   // ---------------------------------------------------------------
   unsigned int Add_pushbutton(const char* label, WORD id, int left,
                               int top, int width = 50,
                               int height = 14);


   // Parameter to Add_Browse_button().

   enum e_browse_type { e_file_open, e_file_save,
                        e_select_directory };


   // ----------------- Function Add_Browse_button ------------------
   // Function to display a pushbutton, which when clicked displays a
   // dialog to allow the user to select a file or directory. If the
   // user selects OK from this second dialog, then the filename is
   // transferred to the edit control added immediately before the
   // pushbutton.
   // Call this function immediately after the edit control with
   // which the Browse button should be associated.
   // dialog_type: One of the e_browse_type enum values:
   //              e_file_open -        CFileDialog instance to open
   //                                   an existing file is displayed
   //              e_file_save -        CFileDialog instance to
   //                                   "Save As" is displayed.
   //              e_select_directory - displays a dialog to allow
   //                                   user to select a shell folder
   // file_dialog_title: Title for the second dialog box
   // file_filter:       File filter applied. If equal to 0, then
   //                    "All files (*.*)| *.*||" is used. This
   //                    parameter is ignored for
   //                    'dialog_type' == e_select_directory.
   // default_extension: The default filename extension. If the user
   //                    does not include an extension in the
   //                    Filename edit box, then 'default_extension'
   //                    is automatically appended to the filename.
   //                    If equal to 0, no file extension is
   //                    appended. This parameter is ignored for
   //                    dialog_type == e_select_directory.
   // label:             Text displayed in the button. If the default
   //                    value of 0 is used, the label displayed is
   //                    "Browse...".
   // ---------------------------------------------------------------
   unsigned int Add_Browse_button(e_browse_type dialog_type,
                                  const char* file_dialog_title,
                                  const char* file_filter,
                                  const char* default_extension,
                                  int left, int top, int width = 50,
                                  int height = 14,
                                  const char* label = 0);


   // Radio buttons:
   // -------------
   // Functions to add a group of radio buttons to a dialog. Start
   // with a call to Add_first_radio_button(), then as many calls as
   // required to Add_radio_button(), then finish the group with a
   // call to Add_group_box().


   // ----------------- Function Add_first_radio_button -------------
   unsigned int Add_first_radio_button(int left, int top, int width,
                                       int height, const char* label,
                                       int* value, WORD id = 0);


   // ----------------- Function Add_radio_button -------------------
   unsigned int Add_radio_button(int left, int top, int width,
                                 int height, const char* label,
                                 WORD id = 0);


   // ----------------- Function Add_group_box ----------------------
   unsigned int Add_group_box(int left, int top, int width,
                              int height, const char* label,
                              WORD id = 0);


   // Checkbox:
   // --------

   // ----------------- Function Add_checkbox -----------------------
   // Function to define a checkbox controlling bool value 'value'.
   // ---------------------------------------------------------------
   unsigned int Add_checkbox(int left, int top, int width,
                             int height, const char* label,
                             bool* value, WORD id = 0);


   // Static text:
   // -----------

   // Parameter to Add_static_text() to specify alignment. The first
   // three use default vertical alignment; the second three center
   // the text vertically.

   enum e_text_alignment { e_left, e_right, e_centered,
                           e_cv_left, e_cv_right, e_cv_centered };


   // ----------------- Function Add_static_text --------------------
   unsigned int Add_static_text(int left, int top, int width,
                                int height, const char* text,
                                e_text_alignment alignment = e_left,
                                WORD id = 0);


   // Functions to modify control attributes:
   // ======================================
   // These two functions are used to modify the attributes (such as
   // left justification of text) of the previous control added to
   // the dialog. So, calls to these functions usually occur
   // immediately after one of the above Add...() functions has been
   // called.


   // ----------------- Function Add_to_last_items_styles -----------
   // Function to add style 'extra_style' to the previous control's
   // attributes.
   // ---------------------------------------------------------------
   void Add_to_last_items_styles(unsigned int extra_style);


   // ----------------- Function Remove_from_last_items_styles ------
   // Function to remove style 'extra_style' to the previous
   // control's attributes. All other attributes are retained.
   // ---------------------------------------------------------------
   void Remove_from_last_items_styles(unsigned int remove_style);

   // Get value strings (e.g. edit)
   CString* get_edit_string(int id) {
     std::map<unsigned int, CString*>::iterator it;
     it = value_strings.find(id);
     if (it != value_strings.end()) {
       return it->second;
     }
     return 0;
   }

   // settext
   void set_text(int id, const char* newstr) {
     std::map<unsigned int, CString*>::iterator it;
     it = value_strings.find(id);
     if (it != value_strings.end()) {
       it->second->SetString(newstr); 
       if (!d_first_time) {
         UpdateData(FALSE);
       }       
     }     
   }

   // Function to display the dialog:
   // ==============================

   // ----------------- Function DoModal ----------------------------
   // Function to display the modal dialog. Calls to all the above
   // functions must occur before DoModal() is first called.
   // RETURN: The same return values as CDialog::DoModal() are used.
   // ---------------------------------------------------------------
   virtual int DoModal();

   WORD Assign_id(WORD id) const;


protected:
   CArray<Dialog_item, Dialog_item&> d_items;  // Contains one item
                                               // per control in the
                                               // dialog.

   // Members for implementing combo boxes and list boxes:

   CStringList   d_strings;       // Strings for all the combo boxes
                                  // and list boxes.
   int           d_n_combo_boxes; // Number of combo boxes in dialog.
   CComboBox*    d_combo_boxes;
   int           d_n_list_boxes;  // Number of list boxes in dialog.
   CListBox*     d_list_boxes;

   // Members for implementing browse buttons:

   // Up to four "Browse..." buttons are allowed in this class. The
   // number is fixed because of the need to define the message map
   // explicitly. There are four corresponding OnBrowse?() functions
   // in the message map. To change the number of Browse buttons,
   // change the enum 'e_n_browse_max' and ensure that there are the
   // corresponding number of OnBrowse?() functions.

   enum { e_n_browse_max = 4, e_first_browse_idc = 5200 };

   int           d_n_browse_used;         // Number of Browse buttons
                                          // used in this dialog.

   // Arrays holding (in order) the type of Browse dialog to be
   // displayed, the title to appear in the Browse dialog, the file
   // filter for the Browse dialog, the default extension for file
   // Browse dialog, and the index into d_items for the CString
   // variable associated with the Browse.

   e_browse_type d_browse_type[e_n_browse_max];
   CString       d_browse_titles[e_n_browse_max];
   CString       d_file_filters[e_n_browse_max];
   CString       d_file_extension[e_n_browse_max];
   int           d_item_index[e_n_browse_max];


   void Browse(int button);
   void OnBrowse0();
   void OnBrowse1();
   void OnBrowse2();
   void OnBrowse3();

   // These two functions override the CDialog functionality.

   virtual void DoDataExchange(CDataExchange* pDX);
   virtual BOOL OnInitDialog();

   DECLARE_MESSAGE_MAP()   

private:
   DLGTEMPLATE    d_dialog_template;
   unsigned char* d_template_buffer;

   CString        d_title;
   CString        d_font_name;
   WORD           d_font_size;

   bool           d_first_time;

   WORD Add_item(WORD id, Dialog_item& item);   

   unsigned int Add_combo_or_list_box(bool is_combo,
                                      const char* prompt_str,
                                      int prompt_left,
                                      const char* unit_str,
                                      int left, int top, int width,
                                      int height, int* index,
                                      CString* str, int n_entries,
                                      const char* entries[],
                                      const CStringList* list,
                                      bool sort, WORD id);

   bool Add_prompt_and_unit_strings(const char* prompt_str,
                                    int prompt_left,
                                    const char* unit_str,
                                    int edit_control_left, int top,
                                    int width, int height);

   static bool Build_template_buffer(
                     const DLGTEMPLATE& dialog_template,
                     CArray<Dialog_item, Dialog_item&>& dialog_items,
                     const char* dialog_caption,
                     const char* dialog_font_name,
                     const WORD dialog_font_size,
                     unsigned char** buffer);

   // Copy constructor and operator=() are not used for this class.
   Dynamic_dialog(const Dynamic_dialog& /* r */) {}
   const Dynamic_dialog& operator=(const Dynamic_dialog& /* r */) {}

   std::map<unsigned int, CString*> value_strings;   
};

#endif   // #ifndef DYNAMIC_DIALOG_H_

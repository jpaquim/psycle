// code from http://www.drdobbs.com/windows/building-mfc-dialogs-at-runtime/184405811
//
//:  Interface to the Dialog_item class.

#ifndef DIALOG_ITEM_H_

#define DIALOG_ITEM_H_

#include <afxcoll.h>    // for CStringList


// -------------------- Class Dialog_item ---------------------------
// Helper class for the Dynamic_dialog class.
// ------------------------------------------------------------------

class Dialog_item
{
public:
   // enum's are from the MSDN documentation for DLGITEMTEMPLATE.

   enum controltype { e_button = 0x0080, e_editcontrol,
                      e_static_text, e_listbox, e_scrollbar,
                      e_combobox };


   // ----------------- Function Dialog_item ------------------------
   // Default constructor for the Dialog_item class, required for
   // CStringList's allocations.
   // ---------------------------------------------------------------
   Dialog_item();


   // ----------------- Function ~Dialog_item -----------------------
   // Destructor for the Dialog_item class.
   // ---------------------------------------------------------------
   ~Dialog_item() {}


   // ----------------- Function Define_static_text -----------------
   // Function to set up a static text dialog item.
   // text_alignment: One of SS_LEFT, SS_RIGHT, SS_CENTER. This
   //                 parameter makes the overall style equivalent to
   //                 LTEXT, RTEXT or CTEXT, respectively.
   // ---------------------------------------------------------------
   void Define_static_text(WORD id, int left, int top, int width,
                           int height, const char* caption,
                           unsigned int text_alignment);


   // ----------------- Function Define_checkbox --------------------
   // Function to set up a checkbox.
   // ---------------------------------------------------------------
   void Define_checkbox(WORD id, int left, int top, int width,
                        int height, const char* caption,
                        bool* value);


   // ----------------- Function Define_combo_or_list_box -----------
   // Function to set up a dropdown list combo box.
   // ---------------------------------------------------------------
   void Define_combo_or_list_box(bool is_combo, WORD id, int left,
                                 int top, int width, int height,
                                 int* index, int first_string_index,
                                 int last_string_index, bool sort);

   void Define_combo_or_list_box(bool is_combo, WORD id, int left,
                                 int top, int width, int height,
                                 CString* str,
                                 int first_string_index,
                                 int last_string_index, bool sort);


   // ----------------- Function Define_edit_control ----------------
   // ---------------------------------------------------------------
   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, CString* value,
                            int n_char_limit);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, int* value, int min,
                            int max);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, double* value, double min,
                            double max);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, float* value, float min,
                            float  max);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, unsigned int* value,
                            unsigned int min, unsigned int max);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, BYTE* value, BYTE min,
                            BYTE max);

   void Define_edit_control(WORD id, int left, int top, int width,
                            int height, short* value, short min,
                            short max);


   // ----------------- Function Define_pushbutton ------------------
   // ---------------------------------------------------------------
   void Define_pushbutton(WORD id, int left, int top, int width,
                          int height, const char* caption);


   // ----------------- Function Define_first_radio_button ----------
   // ---------------------------------------------------------------
   void Define_first_radio_button(WORD id, int left, int top,
                                  int width, int height,
                                  const char* caption, int* value);


   // ----------------- Function Define_radio_button ----------------
   // ---------------------------------------------------------------
   void Define_radio_button(WORD id, int left, int top, int width,
                            int height, const char* caption);


   // ----------------- Function Define_group_box -------------------
   // ---------------------------------------------------------------
   void Define_group_box(WORD id, int left, int top, int width,
                         int height, const char* caption);


   // ----------------- Function Add_style_parameters ---------------
   // ---------------------------------------------------------------
   void Add_style_parameters(unsigned int extra_style);


   // ----------------- Function Remove_style_parameters ------------
   // ---------------------------------------------------------------
   void Remove_style_parameters(unsigned int style);


   // ----------------- Function Button_control_data_exchange -------
   // ---------------------------------------------------------------
   void Button_control_data_exchange(CDataExchange* pDX);


   // ----------------- Function Box_data_exchange ------------------
   // ---------------------------------------------------------------
   void Box_data_exchange(CDataExchange* pDX, CWnd* box);


   // ----------------- Function Edit_control_data_exchange ---------
   // ---------------------------------------------------------------
   void Edit_control_data_exchange(CDataExchange* pDX);


   // ----------------- Function Get_control_type -------------------
   // ---------------------------------------------------------------
   controltype Get_control_type() const;


   // ----------------- Function Is_string_edit_control -------------
   // ---------------------------------------------------------------
   bool Is_string_edit_control() const;


   // ----------------- Function Get_caption ------------------------
   // ---------------------------------------------------------------
   const CString& Get_caption() const;


   // ----------------- Function Set_position_and_size --------------
   // ---------------------------------------------------------------
   void Set_position_and_size(int left, int top, int width,
                              int height);


   // ----------------- Function Add_strings_to_box -----------------
   // Function to add the strings to the combo or list box. This
   // function is only called from OnInitDialog(), and is called for
   // each combo box in turn.
   // ---------------------------------------------------------------
   void Add_strings_to_box(const CStringList& strings, POSITION& pos,
                           CWnd* box) const;


   // ----------------- Function File_or_directory_select -----------
   // ---------------------------------------------------------------
   bool File_or_directory_select(CWnd* window, int dialog_type,
                                 const CString& dialog_title,
                                 const CString& file_filter,
                                 const CString& default_extension);


   // ----------------- Function Write_to_template_buffer -----------
   // ---------------------------------------------------------------
   unsigned char* Write_to_template_buffer(unsigned char* pc) const;


   // ----------------- Function operator= --------------------------
   const Dialog_item& operator=(const Dialog_item& r);


   // ----------------- Function Write_string_to_buffer -------------
   // ---------------------------------------------------------------
   static unsigned char* Write_string_to_buffer(const char* str,
                                                unsigned char* pc);

private:
   enum datatype { e_no_data, e_int, e_float, e_double, e_cstring,
                   e_unsigned_int, e_byte, e_short, e_bool };

   controltype     d_controltype;
   datatype        d_data_type;
   DLGITEMTEMPLATE d_item_template;
   CString         d_caption;
   void*           d_value;

   union uval { int i; float f; double d; unsigned int ui; short si;
                BYTE by; };

   uval            d_min;
   uval            d_max;

   void Set_edit_control_common_parameters(WORD id, int left,
                                           int top, int width,
                                           int height);
   void Set_combo_list_box_common_parameters(bool is_combo, WORD id,
                                             int left, int top,
                                             int width, int height,
                                             int first_string_index,
                                             int last_string_index,
                                             bool sort);

   static bool Find_fullpathname(const char* relative_path,
                                 char* full_path);
   static bool Select_directory(CWnd* window,
                                const char* dialog_title,
                                char* directory_name);

   Dialog_item(const Dialog_item& r);  // not used.
};

#endif   // #ifndef DIALOG_ITEM_H_

#pragma once
/** @file
 *  @brief 
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
namespace win32 {
	namespace gui {
		template< class base > 
		struct ex_dialog : public win32::gui::dialog
		{
			ex_dialog(const string & title = string(), create_subdialogs_type create = do_create_subdialogs ) 
				: ::win32::gui::dialog(title,create)
			{
				init_dialog_items();
			};
		private:
			void init_dialog_items()
			{	
				const HINSTANCE _res_inst = resource_instance();
				const int _dialog_id = static_cast<base&>(*this).dialog_id();
				const HRSRC _hrsrc = ::FindResource(_res_inst, MAKEINTRESOURCE(_dialog_id), _ATL_RT_DLGINIT);
				if (_hrsrc)
				{
					HGLOBAL _hres_data = ::LoadResource(_res_inst, _hrsrc);
					if (_hres_data)
					{
						UNALIGNED WORD* _pdlg_init = (UNALIGNED WORD*)::LockResource(_hres_data);
						if (_pdlg_init)
						{
							while (NULL != *_pdlg_init)
							{
								WORD wID = *_pdlg_init++;
								WORD wMsg = *_pdlg_init++;
								DWORD dwSize = *((UNALIGNED DWORD*&)_pdlg_init)++;

								// CB_ADDSTRING is stored as 0x403
								if (0x403 == wMsg)
								{
									if (-1 == ::SendDlgItemMessage(m_hwnd,wID, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)CA2T((LPSTR)_pdlg_init))))
									{
										WIN32GUI_THROW exception((format(_T("error: file: %s  line: %d")) % __FILE__ % __LINE__).str());
									}
								}
								// CBEM_INSERTITEM is stored as 0x1234
								else if (0x1234 == wMsg)
								{
									COMBOBOXEXITEM item;
									item.mask = CBEIF_TEXT;
									item.iItem = -1;
									CA2T _text((LPSTR)(_pdlg_init));
									item.pszText = _text;
									if (-1 == ::SendDlgItemMessage(m_hwnd,wID, CBEM_INSERTITEM, 0, (LPARAM)&item)){
										WIN32GUI_THROW exception((format(_T("error: file: %s  line: %d")) % __FILE__ % __LINE__).str());
									}
								}
								_pdlg_init = (LPWORD)((LPBYTE)_pdlg_init + dwSize);
							}
						}
					}
				}
			};
		};
	}
}

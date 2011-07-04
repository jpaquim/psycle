#if !defined(AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)
#define AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_
/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DSoundConfig.h : header file
//

#include <mmsystem.h>
#include <dsound.h>
#include <ex_dialog.h>

namespace view {

	using win32::gui::ex_dialog;
	using model::audiodriver::configuration::DirectSound;

	/////////////////////////////////////////////////////////////////////////////
	/**  DSoundConfig dialog */
	struct dsound_config_dlg : win32::gui::wnd_extend<ex_dialog<dsound_config_dlg>,dsound_config_dlg>
	{

		dsound_config_dlg(const DirectSound& dsound);
		dsound_config_dlg() {};
		static int dialog_id(){return IDD_DSOUNDCONFIG;};

		DirectSound* dsound_config(){return m_pconfig;};
		boost::scoped_ptr<DirectSound>& dsound_config_backup(){return m_pconfig_backup;};

	private:
		DirectSound* m_pconfig;
		boost::scoped_ptr<DirectSound> m_pconfig_backup;
	};
}
#endif // !defined(AFX_DSOUNDCONFIG_H__C1ED2948_8437_11D4_937A_E78B9D175439__INCLUDED_)

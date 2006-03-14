/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.4 $
 */
// DSoundConfig.cpp : implementation file
//

#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "DirectSound.h"
#include "resource.h"
#include ".\dsoundconfig.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
#define MIN_NUMBUF 2
#define MAX_NUMBUF 16

#define MIN_BUFSIZE 512
#define MAX_BUFSIZE 32768


using namespace win32::gui;
using model::audiodriver::configuration::DirectSound;
using boost::lexical_cast;

namespace view {


struct dsound_config_dlg_handler : event_handler<dsound_config_dlg_handler, ex_dialog<dsound_config_dlg>,dsound_config_dlg>
{
	/** OKボタンが押された場合の処理 */
	handle_event on_ok()
	{

		wnd<combo_box> _device_box = window()->child<combo_box>(IDC_DSOUND_DEVICE);
		DirectSound& _dsconfig(*(window()->dsound_config()));

		if (_device_box->item_count() > 0)
		{
			// Set Sample Rate 
			wnd<combo_box> _samplerate_box = window()->child<combo_box>(IDC_DSOUND_SAMPLERATE_COMBO);
			string _str = _samplerate_box->item_text(_samplerate_box->sel()).substr();
			_str = _str.substr(0,_str.find(_T(" hz"),0));
			_dsconfig.SamplesPerSec	
				= boost::lexical_cast<int>(_str);

			// Set Selected Device
			DirectSound::DeviceList::iterator it 
				= _dsconfig.DeviceLists.find(_device_box->item_text(_device_box->sel()));

			_dsconfig.IsExclusive = window()->child<check_box>(IDC_EXCLUSIVE)->is_checked();
			_dsconfig.IsDither = window()->child<check_box>(IDC_DSOUND_DITHER)->is_checked();

			_dsconfig.NumBuffers = boost::lexical_cast<int>(window()->child<edit>(IDC_DSOUND_BUFNUM_EDIT)->text());
			_dsconfig.BufferSize = boost::lexical_cast<int>(window()->child<edit>(IDC_DSOUND_BUFSIZE_EDIT)->text());

			//CString _dev_desc;
			//m_deviceComboBox.GetLBText(m_deviceComboBox.GetCurSel(),_dev_desc);
			//
			//string _dev_desc1(_dev_desc);

			//configuration::DirectSound::DeviceList::iterator it 
			//	= m_CurrentConfig.DeviceLists.find(_dev_desc1);

			if(it != _dsconfig.DeviceLists.end())
			{
				_dsconfig.DsGuid = (*it).second.Guid;
			} else {
				_dsconfig.DsGuid = GUID_NULL;
			}

			_dsconfig.DeviceDescription = (*it).first;

		}

		_dsconfig.Apply(*(window()->dsound_config_backup()));

		 return command<IDOK>().HANDLED_BY(&me::on_ok);
	 };

	/** キャンセルボタンが押された場合の処理 */
	handle_event on_cancel()
	{
		DirectSound& _dsconfig(*(window()->dsound_config()));
		_dsconfig.Cancel(*(window()->dsound_config_backup()));
		 return command<IDCANCEL>().HANDLED_BY(&me::on_cancel);
	};

	/** バッファ数が変更された */
	handle_event on_change_bufnum()
	{
		DirectSound& _dsconfig(*(window()->dsound_config()));
		_dsconfig.NumBuffers = boost::lexical_cast<int>(window()->child<edit>(IDC_DSOUND_BUFNUM_EDIT)->text()); 
	

		// Latencyを再計算して表示する
		window()->child<label>(IDC_DSOUND_LATENCY)->text((format(_T("Latency: %d ms")) % _dsconfig.Latency).str());
		return command<IDC_DSOUND_BUFNUM_EDIT,EN_CHANGE>().HANDLED_BY(&me::on_change_bufnum);
	};

	/** バッファサイズが変更された */
	handle_event on_change_buffersize()
	{
		DirectSound& _dsconfig(*(window()->dsound_config()));
		_dsconfig.BufferSize = 
			lexical_cast<int>(window()->child<edit>(IDC_DSOUND_BUFSIZE_EDIT)->text()); 

		// Latencyを再計算して表示する
		window()->child<label>(IDC_DSOUND_LATENCY)->text((format(_T("Latency: %d ms")) % _dsconfig.Latency).str());
		return command<IDC_DSOUND_BUFSIZE_EDIT,EN_CHANGE>().HANDLED_BY(&me::on_change_buffersize);
	};

	/** サンプルレートが変更された */
	handle_event on_change_samplerate()
	{
		DirectSound& _dsconfig(*(window()->dsound_config()));

		// Set Sample Rate 
		wnd<combo_box> _samplerate_box = window()->child<combo_box>(IDC_DSOUND_SAMPLERATE_COMBO);
		string _str = _samplerate_box->item_text(_samplerate_box->sel()).substr();
		_str = _str.substr(0,_str.find(_T(" hz"),0));
		_dsconfig.SamplesPerSec	
			= boost::lexical_cast<int>(_str);

		// Latencyを再計算して表示する
		window()->child<label>(IDC_DSOUND_LATENCY)->text((format(_T("Latency: %d ms")) % _dsconfig.Latency).str());
		return command<IDC_DSOUND_SAMPLERATE_COMBO,CBN_SELENDOK>().HANDLED_BY(&me::on_change_samplerate);
	};
};

dsound_config_dlg::dsound_config_dlg(const DirectSound&  dsound)
: extend_base(),m_pconfig(const_cast<DirectSound*>(&dsound))
, m_pconfig_backup(new DirectSound(dsound.DirectSoundObject))
{

	using ::model::audiodriver::configuration::DirectSound::DeviceInfo;
	using ::model::audiodriver::configuration::DirectSound::DeviceList;

	// backupをとる
	*m_pconfig_backup = dsound;

	wnd<combo_box> _box = child<combo_box>(IDC_DSOUND_DEVICE);
	
	model::audiodriver::configuration::DirectSound& _config = *m_pconfig;

	typedef model::audiodriver::configuration::DirectSound::DeviceList::iterator _iter;

	for(_iter it = _config.DeviceLists.begin(); it != _config.DeviceLists.end();++it)
	{
		_box->add_item((*it).first);
	}

	const int _index = _box->select_str(_config.DeviceDescription,0);
	if(_index != CB_ERR)
	{
		_box->sel(_index);
	} else {
		_box->sel(0);
		_config.DeviceDescription = _box->item_text(0);
	}

	//
	DeviceList& _lists = _config.DeviceLists;
	DeviceInfo& _info = _lists[_config.DeviceDescription];
	
	for(UINT i = 0; i < model::audiodriver::configuration::SAMPLE_RATES_SIZE; i++)
	{
		const UINT _rate = model::audiodriver::configuration::SAMPLE_RATES[i];
		if((_rate >= _info.Capabilities.dwMinSecondarySampleRate) && (_rate <= _info.Capabilities.dwMaxSecondarySampleRate)){
			child<combo_box>(IDC_DSOUND_SAMPLERATE_COMBO)->add_item(
				(format(_T("%d hz")) % _rate).str()
			);
		}
	}


	// Sample rate
	if(child<combo_box>(IDC_DSOUND_SAMPLERATE_COMBO)->select_str(
			(format(_T("%d hz")) % _config.SamplesPerSec).str(),0
		) == CB_ERR)
	{
		child<combo_box>(IDC_DSOUND_SAMPLERATE_COMBO)->select_str(_T("44100 hz"),-1);
	}

	// Check boxes
	child<check_box>(IDC_EXCLUSIVE)->check(_config.IsExclusive);
	child<check_box>(IDC_DSOUND_DITHER)->check(_config.IsDither);

	child<edit>(IDC_DSOUND_BUFNUM_EDIT)->text(boost::lexical_cast<string>(_config.NumBuffers));
	child<spin_button>(IDC_DSOUND_BUFNUM_SPIN)->range(MIN_NUMBUF, MAX_NUMBUF);

	child<edit>(IDC_DSOUND_BUFSIZE_EDIT)->text(boost::lexical_cast<string>(_config.BufferSize));
	child<spin_button>(IDC_DSOUND_BUFSIZE_SPIN)->range(MIN_BUFSIZE, MAX_BUFSIZE);
	UDACCEL _ac;
	_ac.nSec = 0;
	_ac.nInc = 512;
	spin_button::accel_array _a;
	_a.push_back(_ac);
	child<spin_button>(IDC_DSOUND_BUFSIZE_SPIN)->accels(_a);

	// Latencyを再計算して表示する
	child<label>(IDC_DSOUND_LATENCY)->text((format(_T("Latency: %d ms")) % _config.Latency).str());

};

}

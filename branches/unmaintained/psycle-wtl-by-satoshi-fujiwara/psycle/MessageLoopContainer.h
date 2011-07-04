#pragma once
/** @file
 *  @brief CMessageLoopContainer
 *  @author S.F. (Satoshi Fujiwara)
 */

namespace SF {

/** CMessageLoopContainer Class */
class CMessageLoopContainer
{
public:
	const DWORD MainThreadID(){return m_dwMainThreadID;};
	void MainThreadID(const DWORD value){m_dwMainThreadID = value;};
// Overrides of CComModule::Init and Term
	void Init()
	{
		m_dwMainThreadID = ::GetCurrentThreadId();
		typedef ATL::CSimpleMap<DWORD, CMessageLoop*>   _mapClass;
		m_pMsgLoopMap = NULL;
		ATLTRY(m_pMsgLoopMap = new _mapClass);
		//if(m_pMsgLoopMap == NULL)
		//	return E_OUTOFMEMORY;
		//m_pSettingChangeNotify = NULL;
	}

	void Term()
	{
		//TermSettingChangeNotify();
		delete m_pMsgLoopMap;
	};

// Message loop map methods
	const BOOL AddMessageLoop(WTL::CMessageLoop* pMsgLoop)
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::AddMessageLoop.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		ATLASSERT(pMsgLoop != NULL);
		ATLASSERT(m_pMsgLoopMap->Lookup(::GetCurrentThreadId()) == NULL);   // not in map yet

		BOOL bRet = m_pMsgLoopMap->Add(::GetCurrentThreadId(), pMsgLoop);

		lock.Unlock();

		return bRet;
	};

	const BOOL RemoveMessageLoop()
	{
		CStaticDataInitCriticalSectionLock lock;
		if(FAILED(lock.Lock()))
		{
			ATLTRACE2(atlTraceUI, 0, _T("ERROR : Unable to lock critical section in CAppModule::RemoveMessageLoop.\n"));
			ATLASSERT(FALSE);
			return FALSE;
		}

		BOOL bRet = m_pMsgLoopMap->Remove(::GetCurrentThreadId());

		lock.Unlock();

		return bRet;
	};

	WTL::CMessageLoop* GetMessageLoop(DWORD dwThreadID = ::GetCurrentThreadId())
	{
		boost::mutex::scoped_lock _lock(m_mutex);

		CMessageLoop* pLoop =  m_pMsgLoopMap->Lookup(dwThreadID);

		return pLoop;
	};

	static CMessageLoopContainer & Instance(){
		if(m_pMessageLoops.get() == NULL){
			boost::mutex::scoped_lock _lock(m_mutex);
	
			if(m_pMessageLoops.get() == NULL){
				m_pMessageLoops = std::auto_ptr<CMessageLoopContainer>(new CMessageLoopContainer());
			}
		}
		return (*m_pMessageLoops);
	};

	static CMessageLoopContainer * InstancePtr(){
		if(m_pMessageLoops.get() == NULL){
			boost::mutex::scoped_lock _lock(m_mutex);
			if(m_pMessageLoops.get() == NULL){
				m_pMessageLoops = std::auto_ptr<CMessageLoopContainer>(new CMessageLoopContainer());
			}
		}
		return m_pMessageLoops.get();
	};

	static void Release () {
		m_pMessageLoops.reset();
	};

private:
	CMessageLoopContainer(){;};
	DWORD m_dwMainThreadID;
	ATL::CSimpleMap<DWORD, CMessageLoop*>* m_pMsgLoopMap;
	ATL::CSimpleArray<HWND>* m_pSettingChangeNotify;
	static std::auto_ptr<CMessageLoopContainer> m_pMessageLoops;
	static boost::mutex m_mutex;
};

}// namespace SF
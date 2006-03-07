/** @file DirectSoundTest.cpp
 *  @brief implmentation of the DirectSoundTest class
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.1 $
 **/

//CUPPA:include=+
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include "stdafx.h"
#include "Dsp.h"
#include "AudioDriver.h"
//#include "WaveOut.h"
#include "DirectSound.h"
//#include "ASIOInterface.h"
#include "testcontroller.h"
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
//CUPPA:include=-

//CUPPA:namespace=+
//CUPPA:namespace=-

//template model::Controller;

class DirectSoundTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DirectSoundTest);
//CUPPA:suite=+
  CPPUNIT_TEST(LoadSaveTest);
  CPPUNIT_TEST(ConfigureTest);
//CUPPA:suite=-
  CPPUNIT_TEST_SUITE_END();
private:

  // your stuff...
	model::Controller *m_pController;
	model::audiodriver::configuration::DirectSound *m_pDirectSoundConfig;
	model::audiodriver::DirectSound* m_pDirectSound;

public:

  virtual void setUp() 
  { 
	m_pController = new model::Controller();/* initialize */ 
	m_pDirectSoundConfig = dynamic_cast<model::audiodriver::configuration::DirectSound*>(m_pController->OutputDriver()->Configuration());
	m_pDirectSound = dynamic_cast<model::audiodriver::DirectSound*>(m_pController->OutputDriver());
  }
  virtual void tearDown() { delete m_pController;/* terminate */ }

//CUPPA:decl=+
  void LoadSaveTest() {
	
	const GUID _guid =  {0x01010101,0x02020202,0x03030303,{1,2,3,4,5,6,7,8}};
	m_pDirectSoundConfig->NumBuffers = 5;
	m_pDirectSoundConfig->BufferSize = 4097;
	m_pDirectSoundConfig->DsGuid = _guid;
	m_pDirectSoundConfig->IsDither = true;
	m_pDirectSoundConfig->SamplesPerSec = 44200;
	//m_pDirectSoundConfig->ChannelMode = 4;
	m_pDirectSoundConfig->IsExclusive = true;

   // m_pDirectSoundConfig->Save(m_pDirectSound->DriverName);
	
	string _file_path = m_pController->CurrentDirectory();
	_file_path += m_pDirectSound->DriverName;
	_file_path += L".xml";

	// 設定フラグがfalse なため Saveできないはず
	//CPPUNIT_ASSERT(::PathFileExists(_file_path.c_str()) == FALSE);

	// 設定フラグをtrueにする
	m_pDirectSoundConfig->IsConfigured = true;
	m_pDirectSoundConfig->Save(m_pDirectSound->DriverName);

	CPPUNIT_ASSERT(::PathFileExists(_file_path.c_str()) == TRUE);

	m_pDirectSoundConfig->Load(m_pDirectSound->DriverName);
	
	CPPUNIT_ASSERT(m_pDirectSoundConfig->BitDepth == 16);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->NumBuffers == 5);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->BufferSize == 4097);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->DsGuid == _guid);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->IsDither == true);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->SamplesPerSec == 44200);
	//CPPUNIT_ASSERT(m_pDirectSoundConfig->ChannelMode == 4);
	CPPUNIT_ASSERT(m_pDirectSoundConfig->IsExclusive == true);

	//CPPUNIT_FAIL("no implementation");
  }

  void ConfigureTest() {
	 m_pController->Run(NULL,NULL,SW_SHOW);
	 //m_pController->OutputDriver()->Configuration()->Configure();
  }

//CUPPA:decl=-
};

//CUPPA:impl=+
//CUPPA:impl=-

CPPUNIT_TEST_SUITE_REGISTRATION(DirectSoundTest);

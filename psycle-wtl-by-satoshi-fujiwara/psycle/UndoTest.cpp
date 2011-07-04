//CUPPA:include=+

//CUPPA:include=-
#include "stdafx.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

// STL
#include <algorithm>
#include <cctype>
#include <boost/format.hpp>
#include <sstream>
#include "resource.h"
#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h
#include <memory>
#include "sfhelper.h"
#include "UndoRedo.h"
#include "PsycleWTLView.h"

//CUPPA:namespace=+
using namespace SF;
//CUPPA:namespace=-

class UndoTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(UndoTest);
//CUPPA:suite=+
  CPPUNIT_TEST(testCreate);
//CUPPA:suite=-
  CPPUNIT_TEST_SUITE_END();
private:

  // your stuff...
	SF::UndoController m_UndoController;
public:

	virtual void setUp() {
		;
	};
	
	virtual void tearDown(){
	   ;
	};

//CUPPA:decl=+
  void testCreate(){
	CPPUNIT_FAIL("no implementation");
	//CPPUNIT_ASSERT_EQUAL(4,m_pEnvelope->NumOfPoints());
	//CPPUNIT_ASSERT(m_pEnvelope->Point(0) == 100);
  };
//CUPPA:decl=-
};

//CUPPA:impl=+
//CUPPA:impl=-

CPPUNIT_TEST_SUITE_REGISTRATION(UndoTest);
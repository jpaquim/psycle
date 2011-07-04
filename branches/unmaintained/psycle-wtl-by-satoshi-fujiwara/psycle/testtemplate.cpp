//CUPPA:include=+

//CUPPA:include=-
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

//CUPPA:namespace=+
//CUPPA:namespace=-

class VolumeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(VolumeTest);
//CUPPA:suite=+
  CPPUNIT_TEST(test1);
//CUPPA:suite=-
  CPPUNIT_TEST_SUITE_END();
private:

  // your stuff...

public:

  virtual void setUp() { /* initialize */ }
  virtual void tearDown() { /* terminate */ }

//CUPPA:decl=+
  void test1() {
	//CPPUNIT_FAIL("no implementation");
  }
//CUPPA:decl=-
};

//CUPPA:impl=+
//CUPPA:impl=-

CPPUNIT_TEST_SUITE_REGISTRATION(VolumeTest);

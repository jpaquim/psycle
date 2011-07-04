//CUPPA:include=+

//CUPPA:include=-
#include "stdafx.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include "XMInstrument.h"
//CUPPA:namespace=+
using namespace SF;
//CUPPA:namespace=-

class EnvelopeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(EnvelopeTest);
//CUPPA:suite=+
  CPPUNIT_TEST(testAppend);
  CPPUNIT_TEST(testModify);
  CPPUNIT_TEST(testInsert);
  CPPUNIT_TEST(testDelete);
//CUPPA:suite=-
  CPPUNIT_TEST_SUITE_END();
private:

  // your stuff...
	SF::XMInstrument::Envelope *m_pEnvelope;
public:

	virtual void setUp() {
		m_pEnvelope = new SF::XMInstrument::Envelope();
	};
	
	virtual void tearDown(){
	  delete m_pEnvelope;
	};

//CUPPA:decl=+
  void testAppend(){
//	CPPUNIT_FAIL("no implementation");
	m_pEnvelope->Append(100,0.0f);
	m_pEnvelope->Append(200,0.1f);
	m_pEnvelope->Append(300,0.2f);
	m_pEnvelope->Append(400,0.3f);

	CPPUNIT_ASSERT_EQUAL(4,m_pEnvelope->NumOfPoints());

	CPPUNIT_ASSERT(m_pEnvelope->Point(0) == 100);
	CPPUNIT_ASSERT(m_pEnvelope->Point(1) == 200);
	CPPUNIT_ASSERT(m_pEnvelope->Point(2) == 300);
	CPPUNIT_ASSERT(m_pEnvelope->Point(3) == 400);
	CPPUNIT_ASSERT(m_pEnvelope->Point(4) == -1);

	CPPUNIT_ASSERT(m_pEnvelope->Value(0) == 0.0f);
	CPPUNIT_ASSERT(m_pEnvelope->Value(1) == 0.1f);
	CPPUNIT_ASSERT(m_pEnvelope->Value(2) == 0.2f);
	CPPUNIT_ASSERT(m_pEnvelope->Value(3) == 0.3f);
	//CPPUNIT_ASSERT(m_pEnvelope->Value(4) == 0.0f);
	
	m_pEnvelope->Clear();

  }

  void testModify()
  {
  		m_pEnvelope->Append(100,0.0f);
		m_pEnvelope->Append(200,0.1f);
		m_pEnvelope->Append(300,0.2f);
		m_pEnvelope->Append(400,0.3f);
		m_pEnvelope->Append(500,0.4f);
		m_pEnvelope->Append(600,0.5f);
		m_pEnvelope->Append(700,0.6f);
		m_pEnvelope->Append(800,0.7f);
		m_pEnvelope->Append(900,0.8f);
		m_pEnvelope->LoopStart(0);
		m_pEnvelope->LoopEnd(3);
		m_pEnvelope->SustainBegin(0);
		m_pEnvelope->SustainEnd(1);


		// 前に移動

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->PointAndValue(2,50,0.91f));
		
		CPPUNIT_ASSERT_EQUAL(50,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(8));

		CPPUNIT_ASSERT_EQUAL(0.91f,m_pEnvelope->Value(0));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainEnd());
		
		// 一番前を移動
		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->PointAndValue(0,10,0.92f));

		CPPUNIT_ASSERT_EQUAL(10,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(8));

		CPPUNIT_ASSERT_EQUAL(0.92f,m_pEnvelope->Value(0));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainEnd());


		// ない値を移動

		//m_pEnvelope->PointAndValue(10,1000,0.5f);

		//CPPUNIT_ASSERT_EQUAL(10,m_pEnvelope->Point(0));
		//CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		//CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		//CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(3));
		//CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(4));
		//CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(5));
		//CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(6));
		//CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(7));
		//CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(8));

		//CPPUNIT_ASSERT_EQUAL(9,m_pEnvelope->NumOfPoints());

		// 後ろに移動

		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->PointAndValue(0,300,0.99f));
		
		for(int i = 0;i < m_pEnvelope->NumOfPoints();i++)
		{
			ATLTRACE2("%d %d %f\n",i,m_pEnvelope->Point(i),m_pEnvelope->Value(i));

		}

		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(8));

		CPPUNIT_ASSERT_EQUAL(0.99f,m_pEnvelope->Value(2));

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainEnd());

		// 一番後ろに移動

		CPPUNIT_ASSERT_EQUAL(8,m_pEnvelope->PointAndValue(0,1000,0.999f));
		
		for(int i = 0;i < m_pEnvelope->NumOfPoints();i++)
		{
			ATLTRACE2("%d %d %f\n",i,m_pEnvelope->Point(i),m_pEnvelope->Value(i));

		}

		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(1000,m_pEnvelope->Point(8));

		CPPUNIT_ASSERT_EQUAL(0.999f,m_pEnvelope->Value(8));

		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainEnd());

		// 

		m_pEnvelope->SustainBegin(0);
		m_pEnvelope->SustainEnd(3);
		m_pEnvelope->LoopStart(0);
		m_pEnvelope->LoopEnd(3);

		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->PointAndValue(0,450,0.999f));

		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->SustainEnd());

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->PointAndValue(2,350,0.999f));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->SustainEnd());

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->PointAndValue(1,100,0.999f));

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(3,m_pEnvelope->SustainEnd());

		m_pEnvelope->SustainEnd(0);
		m_pEnvelope->PointAndValue(0,10000,1.0f);

		CPPUNIT_ASSERT_EQUAL(8,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(8,m_pEnvelope->SustainEnd());

		m_pEnvelope->Clear();

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->NumOfPoints());

	};
	
	void testInsert(){
		//	CPPUNIT_FAIL("no implementation");

  		m_pEnvelope->Append(100,0.0f);
		m_pEnvelope->Append(200,0.1f);
		m_pEnvelope->Append(300,0.2f);
		m_pEnvelope->Append(400,0.3f);
		m_pEnvelope->Append(500,0.4f);
		m_pEnvelope->Append(600,0.5f);
		m_pEnvelope->Append(700,0.6f);
		m_pEnvelope->Append(800,0.7f);
		m_pEnvelope->Append(900,0.8f);
		
		m_pEnvelope->LoopStart(0);
		m_pEnvelope->LoopEnd(3);
		m_pEnvelope->SustainBegin(0);
		m_pEnvelope->SustainEnd(1);

		// 一番前に追加

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->Insert(50,0.91f));

		// TEST
		CPPUNIT_ASSERT_EQUAL(50,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(8));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(9));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(4,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainEnd());

		// 真ん中に追加
		CPPUNIT_ASSERT_EQUAL(4,m_pEnvelope->Insert(350,0.911f));
	
		CPPUNIT_ASSERT_EQUAL(50,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(350,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(8));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(9));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(10));

		CPPUNIT_ASSERT_EQUAL(0.911f,m_pEnvelope->Value(4));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(5,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainEnd());

		// 一番後ろに追加
		CPPUNIT_ASSERT_EQUAL(11,m_pEnvelope->Insert(1000,0.91f));
	
		CPPUNIT_ASSERT_EQUAL(50,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(100,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(350,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(7));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(8));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(9));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(10));
		CPPUNIT_ASSERT_EQUAL(1000,m_pEnvelope->Point(11));

		CPPUNIT_ASSERT_EQUAL(0.91f,m_pEnvelope->Value(11));

		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(5,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(1,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->SustainEnd());
	}

	void testDelete(){
		//	CPPUNIT_FAIL("no implementation");

  		m_pEnvelope->Append(100,0.0f);
		m_pEnvelope->Append(200,0.1f);
		m_pEnvelope->Append(300,0.2f);
		m_pEnvelope->Append(400,0.3f);
		m_pEnvelope->Append(500,0.4f);
		m_pEnvelope->Append(600,0.5f);
		m_pEnvelope->Append(700,0.6f);
		m_pEnvelope->Append(800,0.7f);
		m_pEnvelope->Append(900,0.8f);
		
		m_pEnvelope->LoopStart(0);
		m_pEnvelope->LoopEnd(3);
		m_pEnvelope->SustainBegin(0);
		m_pEnvelope->SustainEnd(1);

		// 一番前を削除

		m_pEnvelope->Delete(0);

		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(300,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(6));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(7));

		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainEnd());

		m_pEnvelope->LoopStart(0);
		m_pEnvelope->LoopEnd(3);
		m_pEnvelope->SustainBegin(0);
		m_pEnvelope->SustainEnd(1);

		m_pEnvelope->Delete(1);

		CPPUNIT_ASSERT_EQUAL(200,m_pEnvelope->Point(0));
		CPPUNIT_ASSERT_EQUAL(400,m_pEnvelope->Point(1));
		CPPUNIT_ASSERT_EQUAL(500,m_pEnvelope->Point(2));
		CPPUNIT_ASSERT_EQUAL(600,m_pEnvelope->Point(3));
		CPPUNIT_ASSERT_EQUAL(700,m_pEnvelope->Point(4));
		CPPUNIT_ASSERT_EQUAL(800,m_pEnvelope->Point(5));
		CPPUNIT_ASSERT_EQUAL(900,m_pEnvelope->Point(6));

		CPPUNIT_ASSERT_EQUAL(0,m_pEnvelope->LoopStart());
		CPPUNIT_ASSERT_EQUAL(2,m_pEnvelope->LoopEnd());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainBegin());
		CPPUNIT_ASSERT_EQUAL(XMInstrument::Envelope::INVALID,m_pEnvelope->SustainEnd());
		m_pEnvelope->Clear();
	}

//CUPPA:decl=-
};

//CUPPA:impl=+
//CUPPA:impl=-

CPPUNIT_TEST_SUITE_REGISTRATION(EnvelopeTest);

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TextOutputter.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include "dout.h"


void testmain() {
	SF::dstream dout;
	CppUnit::TextUi::TestRunner runner;

	runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
	// std::ofstream ofs("result.txt");
	//CppUnit::CompilerOutputter::defaultOutputter(&runner.result(),SF::dout);
	runner.setOutputter(CppUnit::CompilerOutputter::defaultOutputter(&runner.result(),dout));
	_CrtMemState state;
	_CrtMemCheckpoint(&state);
	runner.run();
    _CrtMemDumpAllObjectsSince(&state);	
	return;
}

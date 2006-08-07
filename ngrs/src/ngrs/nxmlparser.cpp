/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "nxmlparser.h"
#include <iostream>
#include <xercesc/sax2/Attributes.hpp>

XERCES_CPP_NAMESPACE_USE;


class SAX2Handler : public DefaultHandler {

public:
    void startElement(
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

    void fatalError(const SAXParseException&);

    void setParser(NXmlParser* parser) { pParser = parser;};

    std::string getValue(const std::string & name, const Attributes& attrs) {
       std::string erg;
       try {
         XMLCh* str = XMLString::transcode(name.c_str());
         const XMLCh* strCh = attrs.getValue(str);
         char* id = XMLString::transcode(strCh);
         erg = std::string(id);
         XMLString::release(&id);
         XMLString::release(&str);
       } catch (std::exception e) {
           return "";
        }
      return erg;
    }

private:

    NXmlParser* pParser;

};


void SAX2Handler::startElement(const   XMLCh* const    uri,
                            const   XMLCh* const    localname,
                            const   XMLCh* const    qname,
                            const   Attributes&     attrs)
{
    char* message = XMLString::transcode(localname);
    //cout << "I saw element: "<< message << endl;
    std::string tagName = std::string(message);

    pParser->tagParse.emit(tagName);
    pParser->attrs = &attrs;
    XMLString::release(&message);
}


void SAX2Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    std::cout << "xml error: " << message
         << " at line: " << exception.getLineNumber()
         << std::endl;
}



NXmlParser::NXmlParser()
{
}


NXmlParser::~NXmlParser()
{
}


void NXmlParser::parseFile( const std::string & fileName )
{
  attrs = 0;

  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch) {
     char* message = XMLString::transcode(toCatch.getMessage());
     std::cout << "xml parse error: Exception message is: \n"
                 << message << "\n";
     XMLString::release(&message);
  }

  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();

  SAX2Handler* defaultHandler = new SAX2Handler();
  defaultHandler->setParser(this);
  parser->setContentHandler(defaultHandler);
  parser->setErrorHandler(defaultHandler);
  try {
    parser->parse(fileName.c_str());
  }
  catch (const XMLException& toCatch) {
     char* message = XMLString::transcode(toCatch.getMessage());
     std::cout << "xml parse error: Exception message is: \n"
                 << message << "\n";
     XMLString::release(&message);
   }
   catch (const SAXParseException& toCatch) {
     char* message = XMLString::transcode(toCatch.getMessage());
     std::cout << "xml parse error: Exception message is: \n"
                 << message << "\n";
     XMLString::release(&message);
   }
   catch (...) {
     std::cerr << "Xml Parser Unexpected Exception" << std::endl;
   }

  delete parser;
  delete defaultHandler;
  XMLPlatformUtils::Terminate();
}

std::string NXmlParser::getAttribValue( const std::string & name )
{
   std::string erg;
       try {
       XMLCh* str = XMLString::transcode(name.c_str());
       const XMLCh* strCh = attrs->getValue(str);
       char* id = XMLString::transcode(strCh);
       erg = std::string(id);
       XMLString::release(&id);
       XMLString::release(&str);
       } catch (std::exception e)
       {
           return "";
       }
      return erg;
}

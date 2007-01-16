/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper    *
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
#include "xmlparser.h"
#include "file.h"
#include <iostream>

#ifdef __unix__
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>


XERCES_CPP_NAMESPACE_USE;

#ifndef MEMPARSE_ENCODING
   #if defined(OS390)
      #define MEMPARSE_ENCODING "ibm-1047-s390"
   #elif defined(OS400)
      #define MEMPARSE_ENCODING "ibm037"
   #else
      #define MEMPARSE_ENCODING "ascii"
   #endif
#endif /* ifndef MEMPARSE_ENCODING */

// ---------------------------------------------------------------------------
//  Includes for all the program files to see
// ---------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <xercesc/util/PlatformUtils.hpp>


#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif




// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }


private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    target << toDump.localForm();
    return target;
}


class SAX2Handler : public DefaultHandler {

public:
    void startElement(
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

    void fatalError(const SAXParseException&);

    void setParser(ngrs::XmlParser* parser) { pParser = parser;};

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

  ngrs::XmlParser* pParser;

};


void SAX2Handler::startElement(const   XMLCh* const    uri,
                            const   XMLCh* const    localname,
                            const   XMLCh* const    qname,
                            const   Attributes&     attrs)
{
    char* message = XMLString::transcode(localname);
    //cout << "I saw element: "<< message << endl;
    std::string tagName = std::string(message);

    pParser->attrs = &attrs;
    pParser->tagParse.emit(*pParser, tagName);

    XMLString::release(&message);
}


void SAX2Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    std::cout << "xml error: " << message
         << " at line: " << exception.getLineNumber()
         << std::endl;
}
#endif


namespace ngrs {

  XmlParser::XmlParser()
  {
  }


  XmlParser::~XmlParser()
  {
  }


  int XmlParser::parseFile( const std::string & fileName )
  {
#ifdef __unix__
    int err = 0;
    attrs = 0;

    try {
      XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      std::cout << "xml parse error: Exception message is: \n"
        << message << "\n";
      XMLString::release(&message);
      err = 1;
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
      err = 1;
    }
    catch (const SAXParseException& toCatch) {
      char* message = XMLString::transcode(toCatch.getMessage());
      std::cout << "xml parse error: Exception message is: \n"
        << message << "\n";
      XMLString::release(&message);
      err = 1;
    }
    catch (...) {
      std::cerr << "Xml Parser Unexpected Exception" << std::endl;
      err = 1;
    }

    delete parser;
    delete defaultHandler;
    XMLPlatformUtils::Terminate();  
    return err;
#else
    if ( ngrs::File::fileIsReadable( fileName ) ) {
      std::string memparse = ngrs::File::readFile( fileName );
      parseString( memparse );
      return 1;
    } else {
      std::cout << "ngrs_runtime_xml_err:  file is not readable: \n"
        << fileName << "\n";
      return 0;
    }
#endif
  }

  std::string XmlParser::getAttribValue( const std::string & name ) const
  {
#ifdef __unix__
    std::string erg;
    try {
      XMLCh* str = XMLString::transcode(name.c_str());
      const XMLCh* strCh = attrs->getValue(str);
      char* id = XMLString::transcode(strCh);
      erg = std::string(id);
      XMLString::release(&id);
      XMLString::release(&str);
    } catch (std::exception e) {
      return "";
    }
    return erg;
#else  
    return attribs.value( name );    
#endif      
  }

  int XmlParser::parseString( const std::string & text )
  {
#ifdef __unix__
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
    SAX2Handler defaultHandler;
    defaultHandler.setParser(this);
    parser->setContentHandler(&defaultHandler);
    parser->setErrorHandler(&defaultHandler);

    const char* gMemBufId = "textparse";

    MemBufInputSource* memBufIS = new MemBufInputSource
      (
      (const XMLByte*)text.c_str()
      , strlen(text.c_str())
      , gMemBufId
      , false
      );

    std::cout << "hejdslkj" << std::endl;

    int errorCount = 0;
    int errorCode = 0;
    try
    {
      parser->parse(*memBufIS);
      const unsigned long endMillis = XMLPlatformUtils::getCurrentMillis();
      errorCount = parser->getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
      XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      errorCode = 5;
    }
    catch (const XMLException& e)
    {
      XERCES_STD_QUALIFIER cerr << "\nError during parsing memory stream:\n"
        << "Exception message is:  \n"
        << StrX(e.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
      errorCode = 4;
    }
    if(errorCode) {
      XMLPlatformUtils::Terminate();
      return errorCode;
    }

    delete parser;

    delete memBufIS;

    // And call the termination method
    XMLPlatformUtils::Terminate();

    if (errorCount > 0)
      return 4;
    else
      return 0;
#else

    NXmlPos xml_pos;
    xml_pos.err = 0;
    std::string::size_type pos = 0;

    while( !xml_pos.err ) {
      xml_pos = getNextTag( pos, text );
      if ( xml_pos.err == 0 && xml_pos.type == 0) {
        attribs.reset( text.substr( xml_pos.pos, xml_pos.len ) );       
        onTagParse( attribs.tagName() );
      }
      pos = xml_pos.pos + xml_pos.len + 1;
    }

    return 0;        
#endif
  }


  NXmlPos XmlParser::getNextTag( int pos,  const std::string & text ) const {

    NXmlPos xml_pos;
    xml_pos.err  = 1;
    xml_pos.type = 0;

    std::string::size_type next_pos = text.find( "<", pos );

    if ( next_pos + 1 < text.length() ) {   
      if ( text[next_pos+1] == '/' ) {
        xml_pos.type = 1;
      }
    }

    if ( next_pos != std::string::npos ) {
      std::string::size_type tag_end = text.find( ">", next_pos );
      if ( tag_end != std::string::npos ) {
        xml_pos.pos = next_pos + 1;
        xml_pos.len = tag_end - next_pos - 1 ;         
        xml_pos.err = 0;
      } else
        xml_pos.err = 1;        
    } else {
      xml_pos.err = 1;      
    }         

    return xml_pos;
  }         

  void XmlParser::onTagParse( const std::string & tagName ) {
    tagParse.emit( *this, tagName );
  }     

}

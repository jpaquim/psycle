#include "nxmlattributes.h"
#include <iostream>


NXmlAttributes::NXmlAttributes( ) {
                                
}                                

NXmlAttributes::NXmlAttributes( const std::string & tag_header )
{
  parseHeader( tag_header );
}


NXmlAttributes::~NXmlAttributes()
{

}

std::string NXmlAttributes::value( const std::string & name ) const {
   std::map< std::string, std::string>::const_iterator it;
   it = attrib_.find( name );
   if ( it != attrib_.end() ) 
     return it->second;
   else  
     return "";
}  

void NXmlAttributes::parseHeader( const std::string & text ) {
  unsigned int pos = text.find( " ", 0 );
  tagName_ = text.substr( 0, pos );
  while ( (pos = getNextAttribute( text, pos  ) ) != std::string::npos );
}

unsigned int NXmlAttributes::getNextAttribute( const std::string & text,  unsigned int pos ) {

  if ( pos >= text.length() ) return std::string::npos;          

  int start_idx = std::string::npos;
  int mid_idx   = std::string::npos;
  int midstart_idx = std::string::npos;
  int end_idx   = std::string::npos;
  
  std::string::const_iterator it = text.begin() + pos;
  for ( ; it < text.end(); it++, pos++ ) {
    char c = *it;
    if ( start_idx == std::string::npos && c != ' ' ) {
       start_idx = pos;       
    } else 
    if ( mid_idx == std::string::npos && start_idx != std::string::npos && c == '=' ) {
       mid_idx = pos;       
    } else
    if ( end_idx == std::string::npos && c == ' ' ) {
    } else 
    if ( end_idx == std::string::npos && c == '\'' ) {
       end_idx = pos;  
       midstart_idx = pos;
    } else
    if ( end_idx != std::string::npos && ( c =='\'' || c =='/'  ) ) {
       end_idx = pos;  
       break;
    }        
  }    
  
  
  std::string prefix  = text.substr( start_idx, mid_idx - start_idx );
  if  ( end_idx >= text.length() || midstart_idx >= text.length() ) return std::string::npos;
  std::string postfix = text.substr( midstart_idx + 1, end_idx - midstart_idx -1 );
  
  attrib_[ prefix ] = postfix;
  
  std::cout << prefix  << std::endl;
  std::cout << postfix << std::endl;
  
  return end_idx;
  
}         

void NXmlAttributes::reset( const std::string & tag_header ) {
  attrib_.clear();
  parseHeader( tag_header );   
}     

const std::string & NXmlAttributes::tagName() const {
  return tagName_;      
}      

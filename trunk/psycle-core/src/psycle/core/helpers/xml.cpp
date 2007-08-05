



///\todo This file should be moved to the common psycle/helpers




#include "../helpers.h"

namespace psy {
  namespace core {

    std::string replaceIllegalXmlChr( const std::string & text, bool strict )
    {
      std::string xml = text;
      
      // replace ampersand
      unsigned int search_pos = 0;
      while ( ( search_pos = xml.find("&", search_pos) ) != std::string::npos )
        xml.replace(search_pos++, 1, "&amp;" );
      
      // replace less than
      while ( ( search_pos = xml.find("<") ) != std::string::npos )
        xml.replace(search_pos, 1, "&lt;" );
      
      if ( strict ) {
        // replace greater than
        while ( ( search_pos = xml.find(">") ) != std::string::npos )
          xml.replace(search_pos, 1, "&gt;" );
        // replace apostrophe
        while ( ( search_pos = xml.find("'") ) != std::string::npos )
          xml.replace(search_pos, 1, "&apos;" );
        // replace quotation mark
        while ( ( search_pos = xml.find("\"") ) != std::string::npos ) 
          xml.replace(search_pos, 1, "&quot;" );
      }
      return xml;
    }
  }
}


///\todo This file should be moved to the common psycle/helpers
#include <libxml++/parsers/domparser.h>
#include <sstream>

namespace psy {
	namespace core {
		// helper function for xml writing
		
		// replaces entitys`s for xml writing
		// There are 5 predefined entity references in XML:
		// &lt;                 <                 less than 
		// &gt;                 >                 greater than
		// &amp;                &                 ampersand 
		// &apos;               '                 apostrophe
		// &quot;               "                 quotation mark
		// Only the characters "<" and "&" are strictly illegal in XML. Apostrophes, quotation marks and greater than signs are legal. strict = true replaces all.
		
		std::string replaceIllegalXmlChr( const std::string & text, bool strict = true );


    // TODO: move str and str_hex to its own helper file

    template<typename T>
    T str(const std::string &  value) {
      T result;
      
      std::stringstream str;
      str << value;
      str >> result;
      
      return result;
    }

    template<typename T>
    T str_hex(const std::string & value) {
      T result;
      
      std::stringstream str;
      str << value;
      str >> std::hex >> result;
      
      return result;
    }


    class xml_helper_element_not_found {
    };

    class xml_helper_attribute_not_found {
    public:
      std::string attr;

      xml_helper_attribute_not_found(std::string _attr)
        : attr(_attr) { }
    };

  xmlpp::Element& get_first_element(xmlpp::Node const& node, std::string tag);

  xmlpp::Attribute& get_attribute(xmlpp::Element const& e, std::string attr);

  template<class T> T get_attr(xmlpp::Element const& e, std::string attr) {
    return str<T>(get_attribute(e,attr).get_value());
  }

  template<class T> T get_attr_hex(xmlpp::Element const& e, std::string attr) {
    return str_hex<T>(get_attribute(e,attr).get_value());
  }

	}
}

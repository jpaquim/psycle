


///\todo This file should be moved to the common psycle/helpers




#include <string>
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
	}
}

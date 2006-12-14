#ifndef NXMLATTRIBUTES_H
#define NXMLATTRIBUTES_H

/*
 * No description
 */
 
#include <string>
#include <map>

struct NXmlPos {
  unsigned int pos;
  unsigned int len;
  int err;
  int type;     
};    
 
class NXmlAttributes
{
	public:

		NXmlAttributes( );

		NXmlAttributes( const std::string & tag_header );

		~NXmlAttributes();
		
		void reset( const std::string & tag_header );
		std::string value( const std::string & name ) const;
		
		const std::string & tagName() const;
		
	private:
            
        std::string tagName_;    
	    std::map< std::string, std::string > attrib_;
            
        void parseHeader( const std::string & text );    	
        unsigned int getNextAttribute( const std::string & text,  unsigned int pos );
	
};

#endif // NXMLATTRIBUTES_H

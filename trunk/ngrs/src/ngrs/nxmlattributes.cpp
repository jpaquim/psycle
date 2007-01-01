/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
  while ( (pos = getNextAttribute( text, pos+1  ) ) != std::string::npos );
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
  
  if ( start_idx == std::string::npos )  return std::string::npos;
  
  std::string prefix  = text.substr( start_idx, mid_idx - start_idx );
  if  ( end_idx >= text.length() || midstart_idx >= text.length() ) return std::string::npos;
  std::string postfix = text.substr( midstart_idx + 1, end_idx - midstart_idx -1 );
  
  attrib_[ prefix ] = postfix;   
  
  return end_idx;
  
}         

void NXmlAttributes::reset( const std::string & tag_header ) {
  attrib_.clear();
  parseHeader( tag_header );   
}     

const std::string & NXmlAttributes::tagName() const {
  return tagName_;      
}      

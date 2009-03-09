// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__XML__INCLUDED
#define PSYCLE__CORE__XML__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#ifdef _MSC_VER
	#define no_xml_available 1;
#endif

#ifdef no_xml_available
#else
///\todo This file should be moved to the common psycle/helpers
#include <libxml++/parsers/domparser.h>
#endif

#include <sstream>
namespace psy { namespace core {
/// helper function for xml writing.
///
/// replaces entitys`s for xml writing.
/// There are 5 predefined entity references in XML:
/// &lt;                 <                 less than ,
/// &gt;                 >                 greater than,
/// &amp;                &                 ampersand,
/// &apos;               '                 apostrophe,
/// &quot;               "                 quotation mark.
/// Only the characters "<" and "&" are strictly illegal in XML. Apostrophes, quotation marks and greater than signs are legal. strict = true replaces all.
std::string replaceIllegalXmlChr( const std::string & text, bool strict = true );


///\todo move str and str_hex to its own helper file

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

#ifdef no_xml_available
#else

class xml_helper_element_not_found {
};

class xml_helper_attribute_not_found {
	public:
		std::string attr;

		xml_helper_attribute_not_found(std::string _attr) : attr(_attr) {}
};

xmlpp::Element& get_first_element(xmlpp::Node const& node, std::string tag);

xmlpp::Attribute& get_attribute(xmlpp::Element const& e, std::string attr);

template<class T> T get_attr(xmlpp::Element const& e, std::string attr) {
	return str<T>(get_attribute(e,attr).get_value());
}

template<class T> T get_attr_hex(xmlpp::Element const& e, std::string attr) {
	return str_hex<T>(get_attribute(e,attr).get_value());
}

#endif

}}
#endif

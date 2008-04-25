// -*- mode:c++; indent-tabs-mode:t -*-
///\todo This file should be moved to the common psycle/helpers

#include "../helpers.h"
#include "xml.h"
namespace psy { namespace core {

std::string replaceIllegalXmlChr( const std::string & text, bool strict ) {
	std::string xml = text;
	
	// replace ampersand
	unsigned int search_pos(0);
	while((search_pos = xml.find("&", search_pos)) != std::string::npos)
		xml.replace(search_pos++, 1, "&amp;");
	
	// replace less than
	while((search_pos = xml.find("<")) != std::string::npos)
		xml.replace(search_pos, 1, "&lt;");
	
	if(strict) {
		// replace greater than
		while((search_pos = xml.find(">") ) != std::string::npos)
			xml.replace(search_pos, 1, "&gt;" );
		// replace apostrophe
		while((search_pos = xml.find("'")) != std::string::npos)
			xml.replace(search_pos, 1, "&apos;");
		// replace quotation mark
		while((search_pos = xml.find("\"")) != std::string::npos)
			xml.replace(search_pos, 1, "&quot;");
	}
	return xml;
}

xmlpp::Element& get_first_element(xmlpp::Node const& node, std::string tag) {
	xmlpp::Node::NodeList const &
	nodes(node.get_children(tag));
	if(nodes.begin() == nodes.end()) throw xml_helper_element_not_found();
	return dynamic_cast<xmlpp::Element&>(**nodes.begin());
}

xmlpp::Attribute& get_attribute(xmlpp::Element const& e, std::string attr) {
	xmlpp::Attribute* a = e.get_attribute(attr);
	if(!a) throw xml_helper_attribute_not_found(attr);
	return static_cast<xmlpp::Attribute&>(*a);
}

}}

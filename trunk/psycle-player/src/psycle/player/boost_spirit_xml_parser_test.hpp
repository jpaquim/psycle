
////////////////////////////////////////////////////
// This file is a test of boost spirit to parse xml
////////////////////////////////////////////////////


// This source is a derived from the following:
	/*=============================================================================
		http://spirit.sourceforge.net/repository/applications/xml.zip

		Spirit
		URL: http://spirit.sourceforge.net/

		Copyright (c) 2001, Daniel C. Nuffer

		This software is provided 'as-is', without any express or implied
		warranty. In no event will the copyright holder be held liable for
		any damages arising from the use of this software.

		Permission is granted to anyone to use this software for any purpose,
		including commercial applications, and to alter it and redistribute
		it freely, subject to the following restrictions:

		1.  The origin of this software must not be misrepresented; you must
			not claim that you wrote the original software. If you use this
			software in a product, an acknowledgment in the product documentation
			would be appreciated but is not required.

		2.  Altered source versions must be plainly marked as such, and must
			not be misrepresented as being the original software.

		3.  This notice may not be removed or altered from any source
			distribution.
	=============================================================================*/

#include <boost/spirit.hpp>
#include <boost/spirit/tree/ast.hpp>
#include <cwchar>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace xml
{
	int i;
					
	class grammar : public boost::spirit::grammar<grammar>
	{
		public:
			template<typename Scanner>
			class definition
			{
				public:
					typedef typename std::iterator_traits<typename Scanner::iterator_t>::value_type char_t;
					#if 1
						#define L(x) x
					#else
						#define L(x) L##x
					#endif
					
					definition(grammar const & /*self*/)
					{
						using boost::spirit::ch_p;
						using boost::spirit::anychar_p;
						using boost::spirit::str_p;
						using boost::spirit::alpha_p;
						using boost::spirit::digit_p;
						using boost::spirit::xdigit_p;
						typedef boost::spirit::chset<char_t> chset;

						// xml character classes
						// for ascii
						chset Sch("\x9\xA\xD\x20");
						chset Char("\x9\xA\xD\x20-\x7F");
						chset Letter("\x41-\x5A\x61-\x7A");
						chset Digit("0-9");
						chset Extender('\xB7');
						chset NameChar = Letter | Digit | chset("._:-") | Extender;

						document = prolog >> element >> *Misc;
						S = +(Sch);
						Name = (Letter | '_' | ':') >> *(NameChar);
						AttValue =
								L('"') >> boost::spirit::token_node_d[*((anychar_p - (chset(L('<')) | L('&') | L('"'))) | Reference)] >> L('"') |
								L('\'') >> boost::spirit::token_node_d[*((anychar_p - (chset(L('<')) | L('&') | L('\''))) | Reference)] >> L('\'');
						chset CharDataChar(anychar_p - (chset(L('<')) | chset(L('&'))));
						CharData = *(CharDataChar - str_p(L("]]>")));
						Comment1 = *(
							(Char - ch_p(L('-'))) |
							(ch_p(L('-')) >> (Char - ch_p(L('-')))));
						Comment = str_p(L("<!--")) >> Comment1 >> str_p(L("-->"));
						CDSect = str_p(L("<![CDATA[")) >> CData >> str_p(L("]]>"));
						CData = *(Char - str_p(L("]]>")));
						prolog = !XMLDecl >> *Misc >> !(doctypedecl >> *Misc);
						XMLDecl = str_p(L("<?xml")) >> VersionInfo >> !EncodingDecl >> !SDDecl >> !S >> str_p(L("?>"));
						VersionInfo = S >> str_p(L("version")) >> Eq >> (
							ch_p(L('\'')) >> VersionNum >> L('\'') |
							ch_p(L('"'))  >> VersionNum >> L('"'));
						Eq = !S >> L('=') >> !S;
						chset VersionNumCh(L("A-Za-z0-9_.:-"));
						VersionNum = +(VersionNumCh);
						Misc = Comment | S;
						doctypedecl =
							str_p(L("<!DOCTYPE")) >>
							*(Char - (chset(L('[')) | L('>'))) >> 
							!('[' >> *(Char - L(']')) >> L(']')) >>
							L('>');
						SDDecl = S >> str_p(L("standalone")) >> Eq >> (
								(ch_p(L('\'')) >> (str_p(L("yes")) | str_p(L("no"))) >> L('\'')) |
								(ch_p(L('"'))  >> (str_p(L("yes")) | str_p(L("no"))) >> L('"')));
						element = EmptyElemTag | STag >> content >> ETag;
						STag = L('<') >> boost::spirit::root_node_d[Name] >> *(S >> Attribute) >> !S >> L('>');
						Attribute = boost::spirit::token_node_d[Name] >> Eq >> AttValue;
						ETag = str_p(L("</")) >> boost::spirit::token_node_d[Name] >> !S >> L('>');
						content = !boost::spirit::token_node_d[CharData] >> *((element | Reference | CDSect | Comment) >> !boost::spirit::token_node_d[CharData]);
						EmptyElemTag = L('<') >> boost::spirit::token_node_d[Name] >> *(S >> Attribute) >> !S >> str_p(L("/>"));
						CharRef =
							str_p(L("&#")) >> +digit_p >> L(';') |
							str_p(L("&#x")) >> +xdigit_p >> L(';');
						Reference = EntityRef | CharRef;
						EntityRef = L('&') >> Name >> L(';');
						EncodingDecl = S >> str_p(L("encoding")) >> Eq >> (
									ch_p(L('"'))  >> EncName >> L('"') |
									ch_p(L('\'')) >> EncName >> L('\''));
						chset EncNameCh = VersionNumCh - chset(L(':'));
						EncName = alpha_p >> *(EncNameCh);
						
						i = 0;
					}

					boost::spirit::rule<Scanner> const & start() const { return document; }

				private:
					boost::spirit::rule<Scanner>
						prolog, element, Misc, Reference,
						CData, doctypedecl, XMLDecl, SDDecl, VersionInfo, EncodingDecl,
						VersionNum, Eq, EmptyElemTag, STag, content, ETag, Attribute,
						AttValue, CharData, Comment, CDSect,
						CharRef, EntityRef, EncName, document,
						Name, Comment1, S;
			};
	};
	
	class x
	{
		public:
			#if 0
			void parse_array(boost::spirit::rule<> const & rule, char const expr[])
			{
				boost::spirit::parse_info<char const *> result = boost::spirit::parse(expr, rule);
				if(result.full)
				{
					std::cout << "\t\t" << expr << " Parses OK\n\n\n";
				}
				else
				{
					std::cout << "\t\t" << expr << " Fails Parsing\n";
					std::cout << "\t\t";
					for (int i = 0; i < (result.stop - expr); ++i) std::cout << " ";
					std::cout << "^--Here\n\n\n";
				}
			}
			#endif

			typedef wchar_t char_t;
			typedef std::vector<char_t>::iterator iterator_t;
			typedef boost::spirit::tree_match<iterator_t, boost::spirit::node_iter_data_factory<> > parse_tree_match_t;
			typedef parse_tree_match_t::tree_iterator node_iter_t;

			template<typename Rule>
			void parse_vector(Rule const & rule, std::vector<char_t> & v) throw(std::exception)
			{
				iterator_t first = v.begin();
				iterator_t last = v.end();

				typedef boost::spirit::ast_match_policy<iterator_t, boost::spirit::node_iter_data_factory<> > match_policy_t;
				typedef boost::spirit::scanner<iterator_t, boost::spirit::scanner_policies<boost::spirit::iteration_policy, match_policy_t> > scanner_t;
				typedef boost::spirit::rule<scanner_t> rule_t;
				
				scanner_t scan(first, last);
				std::cout << "parsing\n";
				parse_tree_match_t hit = rule.parse(scan);
				
				if (hit && first == last) {
					std::cout << "Parses OK\n";
					build_dom(hit);
				} else {
					std::cout << "Fails parsing\n";
					for (int i = 0; i < 50; ++i) {
						if (first == last) break;
						std::cout << static_cast<char>(*first++);
					}
					std::cout << "\n";
				}
			}

			template<typename Rule>
			void parse_stream(Rule const & rule, std::istream & in) throw(std::exception)
			{
				if (!in) throw std::runtime_error("cannot read input stream");

				// Turn off white space skipping on the stream
				in.unsetf(std::ios::skipws);

				typedef unsigned char stream_char;
				std::vector<stream_char> tmp = std::vector<stream_char>(
					std::istream_iterator<stream_char>(in),
					std::istream_iterator<stream_char>());

				// The correct thing to do here would be to use std::wifstream or to translate the file into wide characters,
				// but for now it works just fine with ascii streams.
				std::vector<char_t> v = std::vector<char_t>(tmp.begin(), tmp.end());
				parse_vector(rule, v);
			}

			template<typename Rule>
			void parse_file(Rule const & rule, char const * filename) throw(std::exception)
			{
				std::ifstream in(filename);
				if (!in)
				{
					std::ostringstream s; s << filename << ": cannot read file";
					throw std::runtime_error(s.str().c_str());
				}
				parse_stream(rule, in);
			}

			class dom_node
			{
				public:
					std::string value;
					typedef std::map<std::string, dom_node> map_type;
					map_type child_nodes;
			};
			dom_node dom_root_node;

			void build_dom(parse_tree_match_t hit)
			{
				build_dom(dom_root_node, hit.trees.begin());
			}

			void build_dom(dom_node & dom_current_node, node_iter_t const & i)
			{
				if (i->children.begin() == i->children.end()) { // terminal node
					//std::copy(i->value.begin(), i->value.end(), std::ostream_iterator<char>(std::cout));
					std::string s(i->value.begin(), i->value.end());
					dom_current_node.value = s;
					std::cout << s << "\n";
				} else {
					for(node_iter_t chi = i->children.begin(); chi != i->children.end(); ++chi)
					{
						std::string s(i->value.begin(), i->value.end());
						dom_node child_node;
						dom_current_node.child_nodes[s] = child_node;
						build_dom(child_node, chi);
					}
				}
			}
	};

	int main(int argc, char* argv[])
	{
		x x;
		grammar g;

		if (argc == 2) {
			x.parse_file(g, argv[1]);
			return 0;
		}
		#if 0    	
		std::cout << "The following should fail parsing:\n";
		x.parse_array(g,
				"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
				"<!DOCTYPE greeting [<!ELEMENT greeting (#PCDATA)>]>"
				"<![CDATA[<greeting>Hello, world!</greeting>]]>");

		std::cout << "The following should parse OK:\n";
		x.parse_array(g,
							"<?xml version=\"1.0\"?>"
								"<!DOCTYPE note ["
									"<!ELEMENT note    (to,from,heading,body)>"
									"<!ELEMENT to      (#PCDATA)>"
									"<!ELEMENT from    (#PCDATA)>"
									"<!ELEMENT heading (#PCDATA)>"
									"<!ELEMENT body    (#PCDATA)>"
								"]>"
								"<note>"
									"<to>Tove</to>"
									"<from>Jani</from>"
									"<heading>Reminder</heading>"
										"<body>Don't forget me this weekend!</body>"
									"</note>");
		#endif
		return 0;
	}
}

int main(int argc, char* argv[]) { return xml::main(argc, argv); }

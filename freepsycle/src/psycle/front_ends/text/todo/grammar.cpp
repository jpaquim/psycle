// -*- mode:c++; indent-tabs-mode:t -*-
#include <psycle/detail/project.private.hpp>
#include <boost/spirit/core.hpp>
#include <vector>
#include <psycle/shell/shell.h>
#include <psycle/engine/engine.h>
#include <psycle/host/host.h>
namespace psycle
{
	namespace front_ends
	{
		namespace text
		{
			namespace grammar
			{
				using namespace boost::spirit;
				
				class line
				{
				public:
					inline operator std::string() const { return string_; }
					inline std::string & operator()() { return string_; }
				private:
					std::string string_;
				};
			
				std::istream & operator>>(std::istream & source, line & sink) { return std::getline(source, sink()); }
			
				void parse(const std::string & file_name)
				{
					using namespace std;
					{
						ifstream i(file_name.c_str());
						list<string> l;
						string s;
						copy(istream_iterator<line>(i), istream_iterator<line>(), back_inserter(l));
						copy(l.begin(), l.end(), ostream_iterator<string>(cout,"\n"));
					}
					{
						line line;
						double sample;
						ifstream i(file_name.c_str());
						i >> line >> sample; // reads a file whose format is a line, followed by a float
					}
				}
			
				bool parse_numbers(const char * string, vector<double> & v)
				{
					return parse
						(
							string,
							(
								real_p[append(v)] >> *(',' >> real_p[append(v)])
							),
							space_p
						).full;
				}
			}
		}
	}
}

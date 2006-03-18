#pragma once
#include "detail/project.hpp"
#include <cstdint>
namespace psycle
{
	namespace host
	{
		///\todo this class is unfortunatly not used
		template<typename Entry>
		class pattern : protected std::vector< std::vector<Entries> > // anything more direct for matrices?
		{
			public:
				typedef Entry entry;

			public:
				pattern(unsigned int const & lines, unsigned int const & columns)
				:
					columns_(columns)
				{
					this->lines(lines);
				}

			public:
				void inline name(std::string const & value) throw() { name_ = value; }
				std::string const inline & name() const throw() { return name_; }
			private:
				std::string name_;

			public:
				unsigned int const inline & lines() const throw() { return size(); }
				void lines(unsigned int const & value)
				{
					resize(lines);
					for(iterator i(begin()) ; i != end() ; ++i) *i.resize(columns);
				}

			public:
				unsigned int const inline & columns() const throw() { return column_; }
				void columns(unsigned int const & value)
				{
					columns_ = value;
					for(iterator i(begin()) ; i != end() ; ++i) *i.resize(columns);
				}
			private:
				unsigned int columns_;
		};
	}
}

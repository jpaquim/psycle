#pragma once
#include "detail/project.hpp"
#include "boost/multi_array.hpp"
namespace psycle
{
	namespace host
	{
		///\todo this class is unfortunatly not used
		template<typename Entry>
		class pattern : public boost::multi_array<Entry, 2>
		{
			public:
				typedef Entry entry;

			public:
				pattern(unsigned int const & lines, unsigned int const & columns)
				:
					boost::array<Entry, 2>(lines, columns)
				{
				}

			public:
				void inline name(std::string const & value) throw() { name_ = value; }
				std::string const inline & name() const throw() { return name_; }
			private:
				std::string name_;

			#if 0 // std::vector
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
			#endif
		};
	}
}

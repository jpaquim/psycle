// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007 psycledelics http://psycle.sourceforge.net
///\file
///\interface psycle::file_format_upgrade::detail::version3::root
namespace psycle {
	namespace file_format_upgrade {
		namespace detail {
			namespace version3 {
				class root
				{
					public:
						std::string name, author, comment;
					public:
						void write(output &);
				};
			}
		}
	}
}

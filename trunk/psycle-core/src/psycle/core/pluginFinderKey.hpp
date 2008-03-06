// -*- mode:c++; indent-tabs-mode:t -*-
#ifndef PLUGINFINDERKEY_HPP
#define PLUGINFINDERKEY_HPP

#include <string>

namespace psy
{
	namespace core
	{
		class PluginFinderKey
		{
			public:
				PluginFinderKey( );
				//FIXME: dllPath as part PluginFinderKey??? One of the reasons to have a plugin finder
				// is to find out its dll path from the relative path stored in the song. Obviously, this
				// functionality is not used right now.
				PluginFinderKey( const std::string & name, const std::string & dllPath, int index = 0 );
				~PluginFinderKey();

				static PluginFinderKey internalSampler();

				const std::string & name() const;
				const std::string & dllPath() const;
				int index() const;

				bool operator<(const PluginFinderKey & key) const;
				bool operator ==( const PluginFinderKey & rhs ) const;
			private:
				std::string name_;
				std::string dllPath_;
				int index_;
		};
	}
}

#endif

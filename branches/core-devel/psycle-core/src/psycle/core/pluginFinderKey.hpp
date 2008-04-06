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
				PluginFinderKey( const std::string & name, const std::string & dllPath, int index = 0 );
				~PluginFinderKey();

				static PluginFinderKey internalSampler();
				static PluginFinderKey internalMixer();

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

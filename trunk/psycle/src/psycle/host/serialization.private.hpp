#pragma once
#include "song.hpp"
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
//#include <boost/serialization/level.hpp>
//BOOST_CLASS_IMPLEMENTATION(psycle::host::Song, boost::serialization::object_serializable)
//#include <boost/serialization/version.hpp>
//BOOST_CLASS_VERSION(psycle::host::Song, 0)
//#include <boost/serialization/tracking.hpp>
//BOOST_CLASS_TRACKING(psycle::host::Song, boost::serialization::track_never)
//#include <boost/serialization/export.hpp>
//BOOST_CLASS_EXPORT_GUID(psycle::host::Song, "song")
//#include <boost/serialization/binary_object.hpp>
namespace psycle
{
	namespace host
	{
		template<typename Archive>
		void serialize(Archive & archive, PatternEntry & instance, unsigned int const version)
		{
		}

		template<typename Archive>
		void serialize(Archive & archive, Pattern & instance, unsigned int const version)
		{
		}

		template<typename Archive>
		void serialize(Archive & archive, Machine & instance, unsigned int const version)
		{
		}

		template<typename Archive>
		void serialize(Archive & archive, Instrument & instance, unsigned int const version)
		{
		}

		template<typename Archive>
		void serialize(Archive & archive, Song & instance, unsigned int const version)
		{
			using boost::serialization::make_nvp;

			archive & make_nvp("type"   , std::string("PSY3SONG"));
			archive & make_nvp("name"   , std::string(instance.Name   ));
			archive & make_nvp("author" , std::string(instance.Author ));
			archive & make_nvp("comment", std::string(instance.Comment));

			// speed
			{
				double const hertz(instance.m_LinesPerBeat * instance.m_BeatsPerMin / 60.);
				archive & BOOST_SERIALIZATION_NVP(hertz);
			}

			// sequence
			{
				archive & make_nvp("sequence-length", instance.playLength);
				for(unsigned int i(0); i < instance.playLength; ++i) archive & make_nvp("pattern", instance.playOrder[i]);
			}

			// patterns
			{
				{
					unsigned int patterns(0);
					for(unsigned int pattern(0) ; pattern < MAX_PATTERNS ; ++pattern) if(instance.IsPatternUsed(pattern)) ++patterns;
					archive & BOOST_SERIALIZATION_NVP(patterns);
				}
				for(unsigned int pattern(0) ; pattern < MAX_PATTERNS ; ++pattern)
				{
					if(!instance.IsPatternUsed(pattern)) continue;
					archive & BOOST_SERIALIZATION_NVP(pattern);
					archive & make_nvp("name", std::string(instance.patternName[pattern]));
					PatternEntry * const lines(reinterpret_cast<PatternEntry*>(instance.ppPatternData[pattern]));
					archive & make_nvp("lines", instance.patternLines[pattern]);
					archive & make_nvp("tracks", instance.SONGTRACKS);
					for(unsigned int line(0) ; line < instance.patternLines[pattern] ; ++line)
					{
						archive & BOOST_SERIALIZATION_NVP(line);
						PatternEntry * const events(lines + line * MAX_TRACKS);
						for(unsigned int track(0); track < instance.SONGTRACKS ; ++track)
						{
							archive & BOOST_SERIALIZATION_NVP(track);
							PatternEntry & event(events[track]);
							archive & make_nvp("command"    , event._cmd      );
							archive & make_nvp("instrument" , event._inst     );
							archive & make_nvp("machine"    , event._mach     );
							archive & make_nvp("note"       , event._note     );
							archive & make_nvp("parameter"  , event._parameter);
						}
					}
				}
			}

			// machines
			{
				{
					unsigned int machines(0);
					for(unsigned int i(0) ; i < MAX_MACHINES; ++i) if(instance._pMachine[i]) ++machines;
					archive & BOOST_SERIALIZATION_NVP(machines);
				}
				for(unsigned int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(!instance._pMachine[i]) continue;
					archive & make_nvp("id", i);
				//	archive & *instance._pMachine[i];
				}
			}

			// instruments
			{
				{
					unsigned int instruments(0);
					for(unsigned int i(0) ; i < MAX_INSTRUMENTS; ++i) if(!instance._pInstrument[i]->Empty()) ++instruments;
					archive & BOOST_SERIALIZATION_NVP(instruments);
				}
				for(unsigned int i(0) ; i < MAX_INSTRUMENTS; ++i)
				{
					if(instance._pInstrument[i]->Empty()) continue;
					archive & make_nvp("id", i);
				//	archive & *instance._pInstrument[i];
				}
			}
		}
	}
}

namespace psycle
{
	namespace host
	{
		void Song::SaveXML(std::string const & file_name) throw(std::exception)
		{
			try
			{
				std::ofstream ostream(file_name.c_str());
				boost::archive::xml_oarchive archive(ostream);
				Song const & const_reference(*this); // archive needs a const reference
				archive << boost::serialization::make_nvp("psycle", const_reference);
			}
			catch(...)
			{
				throw;
			}
		}
	}
}

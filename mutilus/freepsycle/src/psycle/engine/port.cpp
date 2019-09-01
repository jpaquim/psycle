// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2000-2007 psycledelics http://psycle.sourceforge.net

///\implementation psycle::engine::port
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "port.hpp"
#include "graph.hpp"
#include "node.hpp"
#include "buffer.hpp"
#include <universalis/compiler/typenameof.hpp>
#include <iomanip>
namespace psycle
{
	namespace engine
	{
		port::port(parent_type & parent, name_type const & name, int const & channels)
		:
			port_type(parent),
			named(name),
			buffer_(0),
			seconds_per_event_(0)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << this->qualified_name() << ": new port";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(!channels)
			{
				channels_ = 0;
				channels_immutable_ = false;
			}
			else
			{
				channels_immutable_ = false;
				this->do_channels(channels);
				channels_immutable_ = true;
				if(loggers::information()())
				{
					std::ostringstream s;
					s << this->qualified_name() << ": port channels is immutable from now on";
					loggers::information()(s.str());
				}
			}
		}
	
		port::~port()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << this->qualified_name() << ": delete port";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		port::name_type port::qualified_name() const
		{
			return parent().qualified_name() + '.' + name();
		}
	
		port::name_type port::semi_qualified_name() const
		{
			return parent().name() + '.' + name();
		}
	
		void port::buffer(typenames::buffer * const buffer)
		{
			if(false && loggers::trace()())
			{
				std::ostringstream s;
				s << "assigning buffer " << buffer << " to port " << qualified_name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(this->buffer_ == buffer)
			{
				if(false && loggers::trace()())
				{
					std::ostringstream s;
					s << "already assigned the same buffer";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				return;
			}
			do_buffer(buffer); // polymorphic virtual call
		}
	
		void port::do_buffer(typenames::buffer * const buffer)
		{
			assert("not yet assigned another buffer: " && (not buffer or not this->buffer_));
			this->buffer_ = buffer;
		}
	
		void port::channels(int const & channels) throw(exception)
		{
			propagate_channels_to_node(channels);
			do_propagate_channels(); // polymorphic virtual call
		}
	
		void port::seconds_per_event(real const & seconds_per_event)
		{
			if(loggers::information()())
			{
				std::ostringstream s;
				s << qualified_name() << " port events per second changing to " << 1 / seconds_per_event;
				loggers::information()(s.str());
			}
			propagate_seconds_per_event_to_node(seconds_per_event);
			do_propagate_seconds_per_event(); // polymorphic virtual call
		}
	
		void port::propagate_channels(int const & channels) throw(exception)
		{
			if(this->channels() == channels) return;
			channels_transaction(channels);
			do_propagate_channels(); // polymorphic virtual call
		}
	
		void port::propagate_seconds_per_event(real const & seconds_per_event)
		{
			if(this->seconds_per_event() == seconds_per_event) return;
			this->seconds_per_event(seconds_per_event);
			do_propagate_seconds_per_event(); // polymorphic virtual call
		}
	
		void port::connect(port & port) throw(exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "connecting " << this->qualified_name() << " to " << port.qualified_name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// channels negociation
			{
				if(this->channels())
					if(port.channels())
						if(this->channels() != port.channels())
							throw exceptions::runtime_error("the channel counts of the two ports are not the same", UNIVERSALIS__COMPILER__LOCATION);
						else
							{ /* nothing to do */ }
					else
						port.propagate_channels_to_node(this->channels());
				else if(port.channels())
					this->propagate_channels_to_node(port.channels());
				else
					{ /* nothing can be done for now */ }
			}
			// seconds per event negociation
			{
				if(this->seconds_per_event())
					if(port.seconds_per_event())
						if(this->seconds_per_event() != port.seconds_per_event())
							throw exceptions::runtime_error("the events per second of the two ports are not the same", UNIVERSALIS__COMPILER__LOCATION);
						else
							{ /* nothing to do */ }
					else
						port.propagate_seconds_per_event_to_node(this->seconds_per_event());
				else if(port.seconds_per_event())
					this->propagate_seconds_per_event_to_node(port.seconds_per_event());
				else
					{ /* nothing can be done for now */ }
			}
		}
	
		void port::propagate_channels_to_node(int const & channels) throw(exception)
		{
			channels_transaction(channels);
			parent().channel_change_notification_from_port(*this);
		}
	
		void port::propagate_seconds_per_event_to_node(real const & seconds_per_event)
		{
			this->seconds_per_event_ = seconds_per_event;
			parent().seconds_per_event_change_notification_from_port(*this);
		}
	
		void port::channels_transaction(int const & channels) throw(exception)
		{
			int const rollback(this->channels());
			try
			{
				this->do_channels(channels);
			}
			catch(...)
			{
				if(!channels_immutable()) this->channels(rollback);
				throw;
			}
		}
	
		void port::do_channels(int const & channels) throw(exception)
		{
			if(loggers::information()())
			{
				std::ostringstream s;
				s << qualified_name() << " port channels changing to " << channels;
				loggers::information()(s.str());
			}
			if(channels_immutable()) throw exceptions::runtime_error("channel count of port " + qualified_name() + " is immutable", UNIVERSALIS__COMPILER__LOCATION);
			this->channels_ = channels;
			if(buffer_ && buffer().size() < channels) buffer().resize(channels);
		}
	
		void port::dump(std::ostream & out, /*const std::string & kind,*/ int const & tabulations) const
		{
			for(int t(0) ; t < tabulations - 1 ; ++t) out << '\t';
			out
				<< std::setw(60) << universalis::compiler::typenameof(*this) << '\t'
				<< std::setw(16) << name()
				<< "\tchannels: " << channels()
				<< "\tevents per second: " << std::setw(6) << events_per_second()
				<< "\tconnections: ";
		}
	
		std::ostream & operator<<(std::ostream & out, port const & port)
		{
			port.dump(out);
			return out;
		}
	}
}

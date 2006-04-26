// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::port
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "port.hpp"
#include "graph.hpp"
#include "node.hpp"
#include "buffer.hpp"
#include <universalis/compiler/typenameof.hpp>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		port::port(parent_type & parent, name_type const & name, int const & channels)
		:
			port_base(parent),
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
	
		port::~port() throw()
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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		port::port(engine::node & node, name_type const & name, int const & channels)
		:
			named(name),
			node_(node),
			buffer_(0),
			seconds_per_event_(0)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": new port";
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
					s << qualified_name() << ": port channels is immutable from now on";
					loggers::information()(s.str());
				}
			}
		}
	
		port::~port() throw()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << qualified_name() << ": delete port";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		port::name_type port::qualified_name() const
		{
			return node().qualified_name() + '.' + name();
		}
	
		port::name_type port::semi_qualified_name() const
		{
			return node().name() + '.' + name();
		}
	
		void port::buffer(engine::buffer * const buffer)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "assigning buffer " << buffer << " to port " << qualified_name();
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(this->buffer_ == buffer)
			{
				if(loggers::trace()())
				{
					std::ostringstream s;
					s << "already assigned the same buffer";
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				return;
			}
			do_buffer(buffer); // polymorphic virtual call
		}
	
		void port::do_buffer(engine::buffer * const buffer)
		{
			assert("not yet assigned another buffer: " && (!buffer || !this->buffer_));
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
			node().channel_change_notification_from_port(*this);
		}
	
		void port::propagate_seconds_per_event_to_node(real const & seconds_per_event)
		{
			this->seconds_per_event_ = seconds_per_event;
			node().seconds_per_event_change_notification_from_port(*this);
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

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

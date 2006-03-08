// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::loggers
#pragma once
#include PACKAGENERIC
#include "exception.hpp"
#include <universalis/compiler/compiler.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/location.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
namespace universalis
{
	namespace operating_system
	{
		namespace loggers
		{
			class multiplex_logger;
		}
		
		/// logger.
		class logger
		{
			public:
				virtual ~logger() throw() {}
				void inline operator()(int const & level, std::string const & message) throw() { log(level, PACKAGENERIC__MODULE__NAME ": " + message); }
				void inline operator()(int const & level, std::string const & message, compiler::location const & location) throw() { log(level, location + message); }
			protected:
				boost::mutex inline & mutex() throw() { return mutex_; }
				void log(int const & level, std::string const & string) throw(); friend class loggers::multiplex_logger;
				void virtual do_log(int const & level, std::string const & string) throw() = 0;
			private:
				boost::mutex mutex_;
		};

		namespace loggers
		{
			/// levels of importance of logger messages.
			namespace levels
			{
				enum level
				{
					trace,       ///< very low level, debug, flooding output.
					information, ///< normal, informative output.
					warning,     ///< warnings.
					exception,   ///< exceptions thrown from software, via "throw some_exception;".
					crash        ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, \see universalis::operating_system::exceptions::hardware.
				};
				int const compiled_threshold =
				(
					#if defined UNIVERSALIS__OPERATING_SYSTEM__LOGGERS__LEVELS__COMPILED_THRESHOLD
						UNIVERSALIS__OPERATING_SYSTEM__LOGGERS__LEVELS__COMPILED_THRESHOLD
					#elif defined NDEBUG
						information
					#else
						trace
					#endif
				);
			}

			/// handling of threshold levels for a logger.
			template<typename Logger, loggers::levels::level const Threshold_Level = loggers::levels::compiled_threshold>
			class logger_threshold_level
			{
				public:
					typedef Logger logger;
					inline operator logger & () const throw() { return logger::singleton; }
					int const inline threshold_level() const throw() { return Threshold_Level; }
					inline operator bool () const throw() { return threshold_level() >= loggers::levels::compiled_threshold; }
					bool inline operator()() const throw() { return *this; }
					void inline operator()(std::string const & string) throw() { if(*this) logger::singleton()(threshold_level(), string); }
					void inline operator()(std::string const & message, compiler::location const & location) throw() { if(*this) logger::singleton()(threshold_level(), message, location); }
			};

			/// handling of threshold levels for a multiple_logger.
			template<loggers::levels::level const Threshold_Level = loggers::levels::compiled_threshold>
			class multiplex_logger_threshold_level : public logger_threshold_level<multiplex_logger, Threshold_Level> {};

			/// very low level, debug, flooding output.
			///\see levels::trace
			multiplex_logger_threshold_level<levels::trace> inline & trace() throw()
			{
				multiplex_logger_threshold_level<levels::trace> static once;
				return once;
			}
			/// normal, informative output.
			///\see levels::information
			multiplex_logger_threshold_level<levels::information> inline & information() throw()
			{
				multiplex_logger_threshold_level<levels::information> static once;
				return once;
			}
			/// warnings.
			///\see levels::warning
			multiplex_logger_threshold_level<levels::warning> inline & warning() throw()
			{
				multiplex_logger_threshold_level<levels::warning> static once;
				return once;
			}
			/// exceptions thrown from software, via "throw some_exception;".
			///\see levels::exception
			multiplex_logger_threshold_level<levels::exception> inline & exception() throw()
			{
				multiplex_logger_threshold_level<levels::exception> static once;
				return once;
			}
			/// exceptions thrown from cpu/os.
			/// They are translated into c++ exceptions, see operating_system::exceptions::translated.
			///\see levels::crash
			multiplex_logger_threshold_level<levels::crash> inline & crash() throw()
			{
				multiplex_logger_threshold_level<levels::crash> static once;
				return once;
			}

			/// logger which forwards to multiple loggers.
			class multiplex_logger : public logger, protected std::vector<logger*>
			{
				public:
					virtual ~multiplex_logger() throw() {}

				///\name container operations
				///\{
					bool add   (logger       & logger);
					bool remove(logger const & logger);
				///\}

				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE do_log(int const & level, std::string const & string) throw();

				///\name singleton
				///\{
				public:
					multiplex_logger static inline & singleton() throw() { return singleton_; }
				private:
					multiplex_logger static          singleton_;
				///\}
			};

			/// logger which outputs to a stream.
			class stream_logger : public logger
			{
				public:
					stream_logger(std::ostream &);
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE do_log(int const & level, std::string const & string) throw();

				protected:
					std::ostream inline & ostream() const throw() { return ostream_; }
				private:
					std::ostream &        ostream_;

				///\name default
				///\{
				public:
					logger static inline & default_logger() throw() { return default_logger_; }
				private:
					stream_logger static   default_logger_;
				///\}
			};
		}
	}
}

// arch-tag: f973c871-0494-401d-8703-cef1952051f3

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface universalis::operating_system::loggers
#pragma once
#include <packageneric/module.private.hpp>
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
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__LOGGERS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace loggers
		{
			class multiplex_logger;
		}
		
		/// logger.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK logger
		{
			public:
				virtual ~logger() throw() {}

			public:
				void inline operator()(int const & level, std::string const & message) throw() { log(level, message); }
				void inline operator()(int const & level, std::string const & message, compiler::location const & location) throw() { log(level, message + "\n" + location); }

			protected:
				void            log(int const & level, std::string const &) throw(); friend class loggers::multiplex_logger;
				void virtual do_log(int const & level, std::string const &) throw() = 0;

			protected:
				boost::mutex inline & mutex() throw() { return mutex_; }
			private:
				boost::mutex          mutex_;
		};

		namespace loggers
		{
			/// logger which forwards to multiple loggers.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiplex_logger : public logger, protected std::vector<logger*>
			{
				public:
					virtual ~multiplex_logger() throw() {}

				///\name container operations
				///\{
				public:
					bool add   (logger       & logger);
					bool remove(logger const & logger);
				///\}

				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE do_log(int const & level, std::string const &) throw();

				///\name singleton
				///\{
				public:
					multiplex_logger static inline & singleton() throw() { return singleton_; }
				private:
					multiplex_logger static          singleton_;
				///\}
			};

			/// logger which outputs to a stream.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK stream_logger : public logger
			{
				public:
					stream_logger(std::ostream &);

				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__IMPLEMENTS_PURE do_log(int const & level, std::string const & string) throw();

				///\name underlying stream
				///\{
				protected:
					std::ostream inline & ostream() const throw() { return ostream_; }
				private:
					std::ostream        & ostream_;
				///\}

				///\name default stream singleton
				///\{
					public:
						       logger static inline & default_logger() throw() { return default_logger_; }
					private:
						stream_logger static          default_logger_;
				///\}
			};

			/// levels of importance of logger messages.
			namespace levels
			{
				enum level
				{
					trace,       ///< very low level, debug, flooding output.
					information, ///< normal, informative output.
					warning,     ///< warnings.
					exception,   ///< exceptions thrown from software, via "throw some_exception;".
					crash        ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, \see processor::exception and operating_system::exception.
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
			template<typename Logger, levels::level const Threshold_Level = levels::compiled_threshold>
			class logger_threshold_level
			{
				public:
					typedef Logger logger;
					inline operator logger & () const throw() { return logger::singleton(); }
					int const inline threshold_level() const throw() { return Threshold_Level; }
					inline operator bool () const throw() { return threshold_level() >= levels::compiled_threshold; }
					bool inline operator()() const throw() { return *this; }
					void inline operator()(std::string const & string) throw() { if(*this) logger::singleton()(threshold_level(), string); }
					void inline operator()(std::string const & message, compiler::location const & location) throw() { if(*this) logger::singleton()(threshold_level(), message, location); }
			};

			/// very low level, debug, flooding output.
			///\see levels::trace
			logger_threshold_level<multiplex_logger, levels::trace> inline & trace() throw()
			{
				logger_threshold_level<multiplex_logger, levels::trace> static once;
				return once;
			}
			/// normal, informative output.
			///\see levels::information
			logger_threshold_level<multiplex_logger, levels::information> inline & information() throw()
			{
				logger_threshold_level<multiplex_logger, levels::information> static once;
				return once;
			}
			/// warnings.
			///\see levels::warning
			logger_threshold_level<multiplex_logger, levels::warning> inline & warning() throw()
			{
				logger_threshold_level<multiplex_logger, levels::warning> static once;
				return once;
			}
			/// exceptions thrown from software, via "throw some_exception;".
			///\see levels::exception
			logger_threshold_level<multiplex_logger, levels::exception> inline & exception() throw()
			{
				logger_threshold_level<multiplex_logger, levels::exception> static once;
				return once;
			}
			/// exceptions thrown from cpu/os.
			/// They are translated into c++ exceptions, \see processor::exception and operating_system::exception.
			///\see levels::crash
			logger_threshold_level<multiplex_logger, levels::crash> inline & crash() throw()
			{
				logger_threshold_level<multiplex_logger, levels::crash> static once;
				return once;
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>

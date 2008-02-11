// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

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
namespace universalis { namespace operating_system {

namespace loggers {
	class multiplex_logger;
}
		
/// logger.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK logger {
	public:
		virtual ~logger() throw() {}

		void operator()(int const level, std::string const & message) throw() { log(level, message); }
		void operator()(int const level, std::string const & message, compiler::location const & location) throw() { log(level, message, location); }

	protected: friend class loggers::multiplex_logger;
		void            log(int const level, std::string const &, compiler::location const & location) throw();
		void            log(int const level, std::string const &) throw();
		void virtual do_log(int const level, std::string const & message, compiler::location const &) throw() = 0;
		void virtual do_log(int const level, std::string const &) throw() = 0;

	protected: boost::mutex & mutex() throw() { return mutex_; }
	private:   boost::mutex   mutex_;
};

namespace loggers {

	/// logger which forwards to multiple loggers.
	/// note: puting UNIVERSALIS__COMPILER__DYNAMIC_LINK at class level does not work for this class on msvc because of the static data member
	class multiplex_logger : public logger, protected std::vector<logger*> {
		public:
			virtual ~multiplex_logger() throw() {}

		///\name container operations
		///\{
			public:
				UNIVERSALIS__COMPILER__DYNAMIC_LINK bool add   (logger       & logger);
				UNIVERSALIS__COMPILER__DYNAMIC_LINK bool remove(logger const & logger);
		///\}

		protected:
			UNIVERSALIS__COMPILER__DYNAMIC_LINK void do_log(int const level, std::string const & message, compiler::location const &) throw() /* override pure */;
			UNIVERSALIS__COMPILER__DYNAMIC_LINK void do_log(int const level, std::string const &) throw() /* override pure */;

		///\name singleton
		///\{
			public:                                      multiplex_logger static & singleton() throw() { return singleton_; }
			private: UNIVERSALIS__COMPILER__DYNAMIC_LINK multiplex_logger static   singleton_;
		///\}
	};

	/// logger which outputs to a stream.
	/// note: puting UNIVERSALIS__COMPILER__DYNAMIC_LINK at class level does not work for this class on msvc because of the static data member
	class stream_logger : public logger {
		public:
			UNIVERSALIS__COMPILER__DYNAMIC_LINK stream_logger(std::ostream &);

		protected:
			UNIVERSALIS__COMPILER__DYNAMIC_LINK void do_log(int const level, std::string const & message, compiler::location const &) throw() /* override pure */;
			UNIVERSALIS__COMPILER__DYNAMIC_LINK void do_log(int const level, std::string const &) throw() /* override pure */;

		///\name underlying stream
		///\{
			protected: std::ostream & ostream() const throw() { return ostream_; }
			private:   std::ostream & ostream_;
		///\}

		///\name default stream singleton
		///\{
			public:                                             logger static & default_logger() throw() { return default_logger_; }
			private: UNIVERSALIS__COMPILER__DYNAMIC_LINK stream_logger static   default_logger_;
		///\}
	};

	/// levels of importance of logger messages.
	namespace levels {
		/// levels of importance of logger messages.
		enum level {
			trace,       ///< very low level, debug, flooding output.
			information, ///< normal, informative output.
			warning,     ///< warnings.
			exception,   ///< exceptions thrown from software, via "throw some_exception;".
			crash        ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, \see processor::exception and operating_system::exception.
		};
		
		/// the compile-time threshold level for the loggers
		int const compiled_threshold(
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
	class logger_threshold_level {
		public:
			typedef Logger logger;
			operator logger & () const throw() { return logger::singleton(); }
			int const threshold_level() const throw() { return Threshold_Level; }
			operator bool () const throw() { return threshold_level() >= levels::compiled_threshold; }
			bool operator()() const throw() { return *this; }
			void operator()(std::string const & string) throw() { if(*this) logger::singleton()(threshold_level(), string); }
			void operator()(std::string const & message, compiler::location const & location) throw() { if(*this) logger::singleton()(threshold_level(), message, location); }
	};

	/// very low level, debug, flooding output.
	///\see levels::trace
	logger_threshold_level<multiplex_logger, levels::trace> inline & trace() throw() {
		logger_threshold_level<multiplex_logger, levels::trace> static once;
		return once;
	}
	
	/// normal, informative output.
	///\see levels::information
	logger_threshold_level<multiplex_logger, levels::information> inline & information() throw() {
		logger_threshold_level<multiplex_logger, levels::information> static once;
		return once;
	}
	
	/// warnings.
	///\see levels::warning
	logger_threshold_level<multiplex_logger, levels::warning> inline & warning() throw() {
		logger_threshold_level<multiplex_logger, levels::warning> static once;
		return once;
	}
	
	/// exceptions thrown from software, via "throw some_exception;".
	///\see levels::exception
	logger_threshold_level<multiplex_logger, levels::exception> inline & exception() throw() {
		logger_threshold_level<multiplex_logger, levels::exception> static once;
		return once;
	}
	
	/// exceptions thrown from cpu/os.
	/// They are translated into c++ exceptions, \see processor::exception and operating_system::exception.
	///\see levels::crash
	logger_threshold_level<multiplex_logger, levels::crash> inline & crash() throw() {
		logger_threshold_level<multiplex_logger, levels::crash> static once;
		return once;
	}
}}}
#include <universalis/compiler/dynamic_link/end.hpp>


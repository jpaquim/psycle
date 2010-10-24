// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__WAVE_FILE_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__WAVE_FILE_OUT__INCLUDED
#pragma once

#include "audiodriver.h"
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <psycle/helpers/dither.hpp>
#include <psycle/helpers/riff.hpp>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

class WaveFileOut : public AudioDriver {
	public:
		/*override*/ AudioDriverInfo info() const;
		WaveFileOut();
		~WaveFileOut() throw();

		void Write(float* data, unsigned int nframes);

		void set_pdf(psycle::helpers::dsp::Dither::Pdf::type pdf) {
			dither_.SetPdf(pdf);
		}

		void set_noiseshaping(psycle::helpers::dsp::Dither::NoiseShape::type shape) {
			dither_.SetNoiseShaping(shape);
		}

		void set_dither_enabled(bool on) { dither_enabled_ = on; }

	protected:
		/*override*/ void do_open() throw(std::exception);
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception);

	private:
		void thread_function();
		void writeBuffer();
		thread * thread_;
		typedef class scoped_lock<mutex> scoped_lock;
		mutex mutable mutex_;
		bool stop_requested_;

		float left[65335];
		float right[65335];
		psycle::helpers::WaveFile wav_file_;
		psycle::helpers::dsp::Dither dither_;
		bool dither_enabled_;
};

}}
#endif

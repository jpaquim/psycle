/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community
*   psycle.sf.net
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef JACKOUT_H
#define JACKOUT_H
#if defined PSYCLE__JACK_AVAILABLE
#include "audiodriver.h"
#include <jack/jack.h>
#include <string>
namespace psy {
	namespace core {

class JackOut : public AudioDriver {
public:

    JackOut();

    ~JackOut();

    virtual AudioDriverInfo info() const;

    virtual JackOut* clone()  const;   // Uses the copy constructor

    virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
            virtual bool Initialized();
            virtual void configure();
            virtual bool Enable(bool e);

private:

    // psycle variables
    bool _initialized;
    void* _callbackContext;
    AUDIODRIVERWORKFN _pCallback;
    bool running_;

    // jack variables
    jack_port_t *output_port_1;
    jack_port_t *output_port_2;

    jack_client_t *client;
    const char **ports;

    std::string clientName_;
    std::string serverName_;

    bool registerToJackServer();

    static int process (jack_nframes_t nframes, void *arg);
    int fillBuffer( jack_nframes_t nframes );

};

	} // namespace core
} // namespace psy
#endif // defined PSYCLE__JACK_AVAILABLE
#endif

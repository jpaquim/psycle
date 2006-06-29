/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
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
#ifndef ESOUNDOUT_H
#define ESOUNDOUT_H

#include <audiodriver.h>

/**
@author Stefan Nattkemper
*/
class ESoundOut : public AudioDriver
{
public:
    ESoundOut();

    ~ESoundOut();

    virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
    virtual bool Initialized();
    virtual void configure();
    virtual bool Enable(bool e);

private:

    void setDefault();

    int iret1;
    static void* pollerThread(void* ptr);
    void* _callbackContext;
    AUDIODRIVERWORKFN _pCallback;
    pthread_t threadid;

    unsigned int channels;
    unsigned int bits;
    unsigned int rate;

    bool _initialized;
    bool _running;


    int get_bit_flag(int bits);
    int get_channels_flag(int channels);
    std::string ESoundOut::translate_device_string(const std::string & server, int port);
    int esd_in, esd_out, esd_duplex;
    int esd_in_fd, esd_out_fd, esd_duplex_fd;
    char device_string[1024];

    int open_output();

    std::string esound_out_server;
    int esound_out_port;
    long device_buffer;

    int read_buffer(char *buffer, long size);
    int write_buffer(char *buffer, long size);

};

#endif

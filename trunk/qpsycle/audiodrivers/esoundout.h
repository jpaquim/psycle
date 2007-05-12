/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper, Johan Boule                  *
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
#if defined PSYCLE__ESOUND_AVAILABLE
#include "audiodriver.h"
#include <pthread.h>
#include <exception>
#include <unistd.h>
namespace psy {
	namespace core {

class ESoundOut : public AudioDriver
{
    public:
        ESoundOut();
        ~ESoundOut();
        
        virtual ESoundOut* clone()  const;   // Uses the copy constructor

        virtual AudioDriverInfo info() const;


    public:
        virtual void Configure();

    public:
        virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
        virtual bool Initialized();			
    private:
        bool initialized_;
        
    public:		
        virtual bool Enable(bool e);			
        
    private:
        unsigned int channels_;
        int channelsFlag();

        unsigned int bits_;
        int bitsFlag();
        
        unsigned int rate_;
        
        void setDefaults();
        
        int open();
        int close();
        std::string host_;
        int port_;
        std::string hostPort();
        int output_;
        int fd_;

        pthread_t threadId_;
        static int audioOutThreadStatic(void*);
        void audioOutThread();
        bool threadRunning_;
        bool killThread_;

        AUDIODRIVERWORKFN callback_;
        void* callbackContext_; // Player callback
        
        int writeBuffer(char * buffer, long size);
        long deviceBuffer_;

        

};
	} // namespace core
} // namespace psy
#endif // defined PSYCLE__ESOUND_AVAILABLE
#endif

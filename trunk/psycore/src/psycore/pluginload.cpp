/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "pluginload.h"

#ifdef HPUX
# include <dl.h>
#elif __unix__
# include <dlfcn.h>
#else
#include <windows.h>
#endif

#ifdef HPUX
# define SHARED_LIB_EXT ".sl"
#elif __unix__
# define SHARED_LIB_EXT ".so"
#else
# define SHARED_LIB_EXT ".dll"
#endif

#ifndef RTLD_NOW
# define RTLD_NOW 0
#endif


namespace psy {
  namespace core {

    PluginLoad::PluginLoad()
      : _libHandle(0)
    {
    }

    PluginLoad::~PluginLoad()
    {
      close();
    }

    bool PluginLoad::open( const std::string& path ) {
      close();
#ifdef HPUX
      _libHandle = ::shl_load( path.c_str(), BIND_DEFERRED, 0); 
#elif __unix__
      _libHandle = ::dlopen( path.c_str(), RTLD_NOW);
#else
      // Set error mode to disable system error pop-ups (for LoadLibrary)
      UINT uOldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
      _libHandle = ::LoadLibrary( path.c_str() );
      // Restore previous error mode
      ::SetErrorMode( uOldErrorMode );
#endif
      return _libHandle != 0;
    }

    void PluginLoad::close()
    {
      if ( _libHandle ) {
#ifdef HPUX
        shl_unload( reinterpret_cast<shl_t>(_libHandle) );
#elif __unix__
        dlclose( _dll );
#else
        ::FreeLibrary( reinterpret_cast<HINSTANCE> (_libHandle) ) ;
#endif
      }
    }

    void* PluginLoad::loadProcAdress( const std::string& path ) {
      if ( !_libHandle) return 0;
#ifdef HPUX
      void *gpi;
      shl_t h = reinterpret_cast<shl_t>(_libHandle);
      if ((shl_findsym(&h, path.c_str(), TYPE_PROCEDURE, &gpi)) != 0)
        gpi = NULL;
      return gpi;
#elif __unix__
      return dlsym( _libHandle, path.c_str() );
#else
      return ::GetProcAddress( reinterpret_cast<HINSTANCE>(_libHandle), path.c_str() );
#endif
    }

    std::string PluginLoad::error() const {
#ifdef HPUX
      return "An error occured while reading a dynamic library";
#elif __unix__
      return dlerror();
#else
      return "An error occured while reading a dynamic library";
#endif
    }

  }
}

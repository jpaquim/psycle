// BASED ON EXAMPLEVIDSOURCE_CPP
/*

  Nullsoft WASABI Source File License

  Copyright 1999-2001 Nullsoft, Inc.

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


  Brennan Underwood
  brennan@nullsoft.com

*/

// This is a sample video input source to the core.

//
// Always start with std.h
#include "../../Wasabi_SDK/studio/bfc/std.h"
#include "../../Wasabi_SDK/studio/bfc/memblock.h"
#include "../../Wasabi_SDK/studio/bfc/timerclient.h"
//
#include "PsycleSource.h"


static WACNAME wac;
WACPARENT *the = &wac;                     

// {42D610C8-CC4B-437c-93F4-27AD0F92876E}
static const GUID <<name>> = 
{ 0x42d610c8, 0xcc4b, 0x437c, { 0x93, 0xf4, 0x27, 0xad, 0xf, 0x92, 0x87, 0x6e } };


WACNAME::WACNAME() : WACPARENT("PsycleSource Component") {
  registerService(new MediaConverterCreator<svc_PsycleSource>);
}

WACNAME::~WACNAME() {
}

GUID WACNAME::getGUID() {
  return guid;
}

#include <packageneric/pre-compiled.private.hpp>
#include "PsycleSource.hpp"
#include <Wasabi_SDK/studio/bfc/std.h>
#include <Wasabi_SDK/studio/bfc/memblock.h>
#include <Wasabi_SDK/studio/bfc/timerclient.h>

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

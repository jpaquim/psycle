#ifndef MODULE__INCLUDED
#define MODULE__INCLUDED
#pragma once

#define module_begin(x) namespace x { namespace private_ {
#define module_import(x) namespace x = x::public_;
#define module_export(x) } namespace public_ { using private_::x; } namespace private_ {
#define module_end }}

#endif


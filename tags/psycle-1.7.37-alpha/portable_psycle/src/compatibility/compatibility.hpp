///\file
/// compatibility tweakings
#pragma once
#define INCLUDE(x) <x>
#define PROJECT compatibility
#include INCLUDE(PROJECT/project.hpp)
#undef PROJECT
#undef INCLUDE

C-Psycle is a variant of Psycle in "C" language (instead of C++, except
of some plugin and driver apis written in C++) with the goal of being
compatible with mfc-psycle and sharing as much features as possible.

Building (Windows)

Check out the psycle svn trunk, open in the cpsycle directory 
"cpsycle.msvc-2019.sln" or "cpsycle.msvc-2008.sln" and adjust the build
target and compile the solution. If something went wrong press CTRL + SHIFT + B
to try again.

Installation

In cpsycle/debug{release}/x86{x64} you find host.exe asio.dll mme.dll
mmemidi.dll directx.dll wasapi.dll. Copy them to the bin  directory
(e.g: C:\Program Files (x86)\Psycle Modular Music Studio) of an existing
installation and start psycle with host.exe.

Documentation

See in cpsycle/doc for more information how the host is structured.
C-Psycle is a variant of Psycle in "C" language (instead of C++, except
of some plugin and driver apis written in C++) with the goal of being
compatible with mfc-psycle and sharing as much features as possible. Work is
quite completed execept XMSampler plays some mods incorrect and ScreamTracker
and ImpulseTracker format aren't yet supported.

Building

Visual Studio

You can use the current community version 2022 with the install option
"Windows desktop development with C++ in Visual Studio".
To build the windows version, check out the psycle svn trunk, open in the
cpsycle directory  "cpsycle.msvc-2019.sln", choose
Build target Win32 or X64) and ensure that the startproject is host. Compile
the solution. If something went wrong press CTRL + SHIFT + B to try again.

Linux

A basic port has now been done to let cpsycle run on a linux platform. It is
experimental, but if you like to compile and test it still, install the base
compiler system and needed dev libaries of x11, xft and asound2 and others.
Some Linux vst will work, lv2 plugins most probably not (just started the host),
ladspa "should" work. The alsa driver should work, jack probably not.
Window resize is extreme laggy and needs to be improved. If you want to exclude
the non-free vst2 interface, outcomment the USE_VST flag 
in ./detail/psyconfig.h.

List for debian:
sudo apt-get update
sudo apt-get install build-essential liblilv-dev liblua5.4-dev libfreetype-dev libfontconfig-dev libgl-dev libx11-dev libxft-dev libxext-dev libxmu-dev libasound2-dev libjack-jackd2-dev libstk-dev

In cpsycle run
    make
    make all        Executes the makefiles of the host and plugins submodules	
    make host       Executes the makefiles of the host
    make plugins    Executes the makefiles of the plugins. The plugins are
                    build in cpsycle/plugins/build.
If the host was build, in cpsycle start the host with ./psycle

To clean call
    make clean          cleans all (host and plugins)
    make clean-host     cleans only the host 
    make clean-plugins  cleans only the plugins

Installation

Windows 

In cpsycle/debug{release}/x86{x64} you find host.exe asio.dll mme.dll
mmemidi.dll directx.dll wasapi.dll you need to copy  into the bin 
directory (e.g: C:\Program Files (x86)\Psycle Modular Music Studio)
of an existing installation. Now start psycle with host.exe.

Documentation

See in cpsycle/doc for more information how the host is structured.

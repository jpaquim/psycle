C-Psycle is a variant of Psycle in "C" language (instead of C++, except
of some plugin and driver apis written in C++) with the goal of being
compatible with mfc-psycle and sharing as much features as possible.
Work is now done to reach the point of releasing a 1.0 version fixing some
incompatibilities in XMSampler and the psy fileformat and cleaning the code.

If anyone wants to try it, it should be easier to compile with visual studio
2019 than Psycle, but at this point it still requires some Psycle Plugins and
some manual changes to code for correct configuration.

The audio core is sequencer based and completly separated from the ui and makes
it possbile to build the command line player under linux. Audio and event input
(Midi/keyboard) are driver orientated.
The ui was reduced from mfc to win32 and bridged to allow in the future a
toolkit for posix aswell.
Besides english, the host can be viewed in spanish aswell (except the help files).

Structure of the host

audio           machines, patterns, instruments, sequence, player, sequencer,
                song import/export, library load, ...
host            graphical psycle front end
player          textmode psycle front end
ui              bridged win32/x11(not finished) calls
uitest          a test program for X11 (placeholder, just a few
                tests)
audiodrivers    win: asio, mme, directx, wasapi
                linux: alsa
eventdrivers    mmemidi, dxjoystick

external libs
lua             script engine used since psycle 1.10
scintilla       plugin editor, help file viewer

helper libs
container       list, hashtable, properties, signals
dsp             buffer operations, adsr, notetab defines, sse2 optimizations,
                dither, resampler, filter
file            file/dir routines

Additional includes
compiler
detail          global header defines, platform tweaks

Compiling

MS-Windows

It compiles only with the VC2019(x86/x64 Win10), other builds needs to be
updated. VC6/2008 are not updated or tested at every commit but if so it will
compile with VC6 x86 (win98), VC2008(x86 XP SP1), too. Install on older
platforms the DirectX SDK and add the include/lib sdk directories
(Tools/Options/Directories).
At this point the host will run. Around 50 plugins can now be compiled but for
loading all natives you need some modules from an existing psycle mfc 
installation.
Copy the appropiate vc runtime and universalis dlls from an existing psycle
installation to the output dir(cpsycle/debug and cpsycle/release).
Only in VC6 set the working dir (host project settings) to
your-path\cpsycle\debug or your-path\cpsycle\release) that the host can
find the audio driver dlls. Or copy the release host.exe and the audiodriver
dlls (directx.dll etc) to the existing mfc psycle installation and start from
there.

List of runtime files to be copied:

boost_chrono_your_version.dll, (from bin path)
boost_date_time_your_version.dll, (from bin path)
boost_filesystem_your_version.dll, (from bin path)
boost_signals_your_version.dll, (from bin path)
boost_systems_your_version.dll, (from bin path)
boost_thread_your_version.dll, (from bin path)
universalis.dll (depending on your version needed or not) (from bin path)
scintilla.dll (vc6, vc2008 only, search for an old compatible win98/xp dll)

Command line player:

Windows: it compiles with the psycle solution file. Then manually copy the
driver dlls
from: cpsycle/{target}/{platform}
to:   cpsycle/player/{target}
{target} Debug or Release
{platform} x86 or x64
CMD: player filename.psy
or run inside
Visual studio: Player Project Settings/Debugging/Command Line Options add
       filename.psy

Linux: At the moment only an alsa driver for linux exists. Since the posix
psycle plugin makefiles are out of date it use is very limited, but to build:

go to cpsycle/player
run: make
cd src
Linux ./psyplayer filename.psy


Command Line Help:
No options and no filename: prints help

Posix Host:

The host part for linux is unfinished and we still have to decide which ui
toolkit to fill the implementation side for linux.


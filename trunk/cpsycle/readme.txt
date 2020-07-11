This is a test version and has no intent to be a full psycle replacement, but
hopefully helps some new programmers to continue psycle. 
It should help replacing MFC from Psycle, making the host more responsive to
different screen resolutions, adding a sequencer, unifying the instruments and
wave controller, sampulse and sampler ps1 use, and avoiding ui dependencies in
the audio core. A command line player makes use of this separation. The player
will serve as first step to make a posix version available. The ui is reduced
from mfc to win32 and wrapped behind an own layer. In a next step this layer
will be used to add xt motif. The version is written in c with c++ parts, in
which some plugin and driver apis are written in.

Structure of the host

audio           machines, patterns, instruments, sequence, player,
                song import/export, library load, ...
host            graphical psycle front end
player          textmode psycle front end
ui              bridged win32 calls, setup for x/motif (placeholder)
audiodrivers	win: asio, mme, mmemidi, directx, wasapi
                linux: alsa (placeholder)

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

It compiles with VC6 x86, VC2008(x86 tested, SP1), VC2017 (x86/x64) and
VC2019(x86/x64), although VC6/2008 may not be updated or tested at every
commit. You need at least Win98, tested versions: Win98/XP/10.
There are VC6/VC2008/2019 files, that can be used to import by different 
VC versions. To build checkout the whole psycle trunk. After migrating you
might need setting Gy- to Gy in each project (settings code
generation). For the 64Bit Configuration adjust the Output File Target
in Project Linker settings for the host project. Set the Output File
Target to the project standard. Install on older platforms the DirectX
SDK and add the include/lib sdk directories (Tools/Options/Directories).
Copy the appropiate vc runtime and universalis dlls from an existing psycle
installation, copy bwltbl.dll from the existing psycle plugin path,
to the output dir(cpsycle/debug and cpsycle/release).
Only in VC6 set the working dir (host project settings) to
your-path\cpsycle\debug or your-path\cpsycle\release) that the host can
find the audio driver dlls.

List of runtime files to be copied:

bwltbl.dll, (from plugin path)
boost_chrono_your_version.dll, (from bin path)
boost_date_time_your_version.dll, (from bin path)
boost_filesystem_your_version.dll, (from bin path)
boost_signals_your_version.dll, (from bin path)
boost_systems_your_version.dll, (from bin path)
boost_thread_your_version.dll, (from bin path)
universalis.dll (depending on your version needed or not) (from bin path)
scintilla.dll (vc6, vc2008 only, search for an old compatible win98/xp dll)

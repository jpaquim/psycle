This version has now most of the psycle mfc host and hopefully helps some new
programmers to continue psycle. It is written in c with c++ parts, in which
some plugin and driver apis are written in. It replaced MFC from Psycle, can
be viewed in different screen resolutions, has a sequencer, unified the
instruments and the wave controller Sampulse and Sampler ps1 use and avoids
ui dependencies in the audio core.  The ui was reduced from mfc to win32 and
wrapped behind an own layer. A X11 implementation is started but not finished.
The command line player needs to be finsihed, too. Still missing is the full
XMSampler reimplementation, XM and IT Songimport and some Mixer options.
Event Input like Midi and Keyboard are now driver orientated and make it
possible to have more than one midi input or other inputs like a
a game controller. This still needs to be fully worked out, but at least works
for the keyboard and midi input. Midi sync is not fully working, too.
Additional to MFC Psycle, the version has a pianoroll, multitracking and
optional a sequenceview that displays the sequence in a gantt layout. Besides
english, more language are possible, and a spanish language file was added.
Many limititation of Psycle are related to the file format. Internally the use
of fixed arrays were replaced by dynamic structures, but many limitations
remain to maintain file compatibility. This needs to be worked out further.

Structure of the host

audio           machines, patterns, instruments, sequence, player,
                song import/export, library load, ...
host            graphical psycle front end
player          textmode psycle front end
ui              bridged win32/x11(not finished) calls
uitest			a test program for X11 (placeholder, just a few
                tests)
audiodrivers	win: asio, mme, mmemidi, directx, wasapi
                linux: alsa
eventdrivers	

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

Momentarily it compiles only with the VC2019(x86/x64 Win10), the other builds
needs to be updated. VC6/2008 are not updated or tested at every commit. If
this is done it will compile with VC6 x86 (win98), VC2008(x86 XP SP1), too.
For the 64Bit Configuration adjust the Output File Target in Project Linker
settings for the host project. Set the Output File Target to the project
standard. Install on older platforms the DirectX SDK and add the include/lib
sdk directories (Tools/Options/Directories).
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

Posix

The X11 part is unfinished, but to build execute the makefiles in the src
directories of audio, container, dsp, file, ui, build the alsa driver and
finally the host. The devel libs for X11 with xft, freetype, xmu, xshape and
in case of building the alsa driver (linux) the alsa devel libs needs to be
installed. Other unix sound systems driver don't exist, but we wrote in the
past for qpsycle some (jack, esound, oss). The psycle, qpsycle, cpsycle drivers
are very simililar, so they can be added later. Unfortunatley the makefiles of
the qpsycle plugins aren't working completly on a recent system without fixing
them manually, thus only the internal plugins are currently available.


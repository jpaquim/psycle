This is a test version and has no intent to be a full psycle replacement, but
hopefully helps some new programmers to continue psycle. 
It should help replacing MFC from Psycle, making the host more responsive to
different screen resolutions, adding a sequencer, unifying sampulse and
sampler ps1 and their instruments, and avoiding ui dependencies in the audio
core. A command line player makes use of this separation. This player will
serve as first step to make a posix version available. The ui is reduced from
mfc to win32 and wrapped behind an own layer. In a next step this layer will
be used to add xt motif. The version is written in c with c++ parts, in which
some plugin and driver apis are written in. 

Structure of the host

audio           machines, patterns, instruments, sequence, player,
                song import/export, ...			
container       list, hashtable, properties, signals
dsp             buffer operations, adsr, notetab defines
detail          global header defines
driver          mme, mmemidi, directx	
file            file/dir routines
host            graphical psycle front end
lua             script engine used since psycle 1.10
ui              wrapped win32 calls

Compiling

It compiles with VC6 x86, VC2008(x86 tested, SP1), VC2017 (x86/x64) and
VC2019(x86/x64). You need at least Win98, tested versions: Win98/XP/10.
There are VC6/VC2008 files, that can be imported by any later VC version.
To build checkout the whole psycle trunk. After migrating to
VC 2008, 2017 or 2019 set Gy- to Gy in each project (settings code
generation). For the 64Bit Configuration adjust the Output File Target
in Project Linker settings for the host project. Set the Output File
Target to the project standard. Install also mfc and atl support
(resource includes need it) and install on older platforms the DirectX
SDK and add the include/lib sdk directories (Tools/Options/Directories).







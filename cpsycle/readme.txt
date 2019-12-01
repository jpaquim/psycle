These files are a setup for a test version to help replacing MFC from 
Psycle and adding a sequencer to the player. This version is neither a
replacement or has any aims to be a full psycle version. To reduce
complexity and incompatibilities between c++ compilers, its written in 
C. For now, it uses win32 behind an abstraction layer. Work is needed to
be done to make first the command player functional under posix systems
and later adjust the ui layer for xt motif. 

The basic setup and goals consist of:
- audio basic structures like machine, pattern, sequence
        develop a sequencer orientated player
- dsp   basic dsp block operations
- host  basic setup for pattern/machine and settingview
        avoiding audio core dependencies on the host
- container basic structures: list, hashtable, properties, signals
            developing a common property system for drivers, 
            audio modules, host views and plugin scanning
- file      psyfile, dir, datacompression, ini fileio (with properties)  
- lua 
- drivers (directx, mme)
          module based approach
- ui   wrapped win32 calls
       Abstraction layer for other toolkits
- player command line player

Compiling

It compiles with VC6 x86, VC2017 (x86/x64) and VC2019 (x86/x64).
There are VC6 files, that can be imported by any later VC version.
To build checkout the whole psycle trunk. After migrating to
VC 2017 or 2019 set Gy- to Gy in each project (settings code generation).
For the 64Bit Configuration adjust the Output File Target in Project
Linker settings for the host project. Set the Output File Target to the 
project standard. Install also mfc and atl support (resource includes 
need it). 








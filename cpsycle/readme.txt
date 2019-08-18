This files are a setup for a test version to help replacing MFC from Psycle and adding a sequencer to the player. This version is neither a replacement or has any aims to be a full psycle version. To reduce complexity and incompatibilities between c++ compilers, its written in C. It uses win32 behind an abstraction layer. 

The basic setup and goals consist of:
- audio basic structures like machine, pattern, sequence
        develop a sequencer orientated player
- dsp   basic dsp block operations
- host  basic setup for pattern/machine and settingview
        avoiding audio core dependencies on the host
- file  developing a common property system for drivers, audio modules, host views and 	plugin scanning
- lua 
- drivers (empty for now)
          module based approach
- ui   wrapped win32 calls
       Abstraction layer for other toolkits

Compiling

There are VC6 files, that can be imported by any later VC version.










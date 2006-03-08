This directory contains the modified code for the VSTHost,
which origin is the excellent vsthost from Hermann Seib :
http://www.hermannseib.com/english/vsthost.htm

The license of this source is not Free. The use is 
limited to these conditions:

[QUOTE]

.) If you want to include parts into Psycle,
.) and if you put a copyright notice on top of the file that states "Parts taken (or something to this effect) from VSTHost (http://www.hermannseib.com/english/vsthost.htm), Copyright (c) H. Seib, 2002-2005",
.) and if a little notice about my contribution appears in the About box,
.) and if you don't make any money from it,
you have my expressed permission to do so.
If you do make money from it, I want my share.

You may put anything you can derive from my code under GPL, I have absolutely no problem with that - the licensing issues with the Steinberg SDKs are your problem alone. 

[/QUOTE]



CVSTHost

This implements the audio callback needed by an effect plugin as a neatly encapsulated VST host class. This is practically platform-independent code. The few platform-specific things are encapsulated in...


CEffect

This encapsulates a VST effect, as loaded by the VST host. This class isn't fully complete - since I don't have a Mac, I've only included the Windows code. If a kind MacIntosh programmer could fill out the few Mac-specific sections and send it to me it would be great and, of course, publicized here!
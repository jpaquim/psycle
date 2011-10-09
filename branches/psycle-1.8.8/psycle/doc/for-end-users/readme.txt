				------------------------------------------------
				]==-> Psycle Modular Music Creation Studio <-==[
				------------------------------- 1.10.0 ---------
--------------------------------------------------------------------------------

contents:
---------

1. About Psycle
2. OpenSource Notes/Licensing
3. Developement
4. User Resources and Links

--------------------------------------------------------------------------------

1. About Psycle:
----------------

Psycle is a free, Open-Source, music creation program that offers an easy way to
create your own music, fast, and with high quality.

Psycle uses a classical tracker interface (a text grid of notes which are
sequenced one after the other), coupled with modularity (plugins) that allow to
extend the sounds that you get from it.

It supports three ways to get sound: its own native format, the VST Plugin
standard, and recorded .wav samples using the sampler machine.

Notes are entered in patterns, where you can also use commands (plugin specific,
or global), as well as automating parameters via the mcm and twk commands.
These patterns are then organized in a linear sequence, which becomes the
song order.

Here some features of Psycle:

- A 64 Tracks Pattern Editor, with up to 1024 lines.
- 256 sequence positions.
- Audio Routing (machine) view to interconnect the virtual machines in order to
  apply different effects.
- Internal Stereo Sampler unit which supports .wav and .iff files of any
  sampling rate, 8/16 and 24bits, mono or stereo.
- 70+ Native Plugins (generators & effects), most of them open-source and with 
  an easy to use API to create new ones.
- Multithreaded audio engine.
- Support for VST1 and VST2 plugin standard, including support for JBridge.
- Importing of .xm's/.s3m's/.it's/.mod's.
- Exporting to wav, as well as recording realtime to .wav
- Configurable Keyboard Layout
- Midi-In Support ( control Psycle using a midi keyboard, or even a software
  sequencer)
- Themes and skins support
and much more.


If you need help starting with Psycle, you should take a look at the "Docs"
section at the Psycledelics site (official Psycle Users Site) (See the links
at the bottom of this document)


Note:
LADSPA is supported in a forthcoming linux version, and DSSI support is planned

--------------------------------------------------------------------------------

2. Open source Notes and License:
---------------------------------

Psycle is open source.

What this means is that everyone is invited to participate in its development.
You can download the code and help extending its feature set, as well as fixing
the bugs you might find during its usage.


Arguru Licensed Psycle 1.0 sources as "Public Domain".
This means that those sources of Psycle can be used in whatever form you want.
Effectively, this licensing applies ONLY to those sources.


The current team prefers to enforce that psycle sources remain free to everyone,
which means everyone can see and modify the source, but in doing so, he/she
cannot distribute them in a closed way. Much in line with GPL ideas.

You might, on the other hand, distrubute any modification you do as your own,
keeping a notice in the sources and/or text files that they are
"a derived work from Psycle".


The team would appreciate that any modification done to Psycle sources, get 
incorporated back to the main sources, which would mean that Psycle and its
community grows instead of being divided.


Source code is stored via SVN at SourceForge site. Point your SVN client to
this link to get the latest source code:

	https://psycle.svn.sourceforge.net/svnroot/psycle/trunk
	
	
There are Forums and Bug trackers in SourceForge site and in Psycledelics site
where you can contact us.  We check the Psycledelics site often.

If you need to contact specifically the developers, you can do it via the
sourceforge member list: 

	http://sourceforge.net/project/memberlist.php?group_id=10834
	
	(ignore the "No Specific Role" ones. They are retired.)
	

You are allowed to develop psycle Machines without any restriction, although if you
decide to make it closed source, you will have to update whenever we make changes
to the API.
There is no SDK ready, so the best way to start a new one is getting the sources
of an existing one. Arguru synth is an usual template.


Psycle 1.8.5 and above includes a freeware VST pack containing software from the following vendors:

Digital Fish Phones	( http://www.digitalfishphones.com/ )
DiscoDSP		( http://www.discodsp.com/ )
SimulAnalog		( http://www.simulanalog.org/ )
Jeroen Breebaart	( http://www.jeroenbreebaart.com/ )
George Yohng		( http://www.yohng.com/ )
Christian Budde		( http://www.savioursofsoul.de/Christian/ )
DDMF			( http://www.ddmf.eu/ )
Loser			( http://loser.asseca.com/ )
E-phonic		( http://www.e-phonic.com/ )
Argu			( http://www.aodix.com/ )
Oatmeal by Fuzzpilz  	( http://bicycle-for-slugs.org/ )

--------------------------------------------------------------------------------

3. Development:
---------------
Psycle:
	Psycle 1.10: This release most probably will see bugfixes.
    
	Psycle 1.11/2.0: The evolution of Psycle implies necessarily to componentize it. Also, new user interfaces are possible nowadays that weren't possible some years ago, as such, Psycle should try to use tablets and smartphones in some way. HTML5 and new media APIS being added to browsers open also another opportunity for diversification. But all these need a real team of developers.


QPsycle:
	QPsycle was made uncompilable during the integration of psycle-core into Psyclemfc during 2009. No real effort has been done to fix that.
    
	psycle-core: The psycle-core engine needs a rethinking on what should it be. There are two ways to go from here on, take psycle-mfc engine and convert it into a library like psycle-core using part of psycle-core where possible, or take psycle-core, remove the parts that are more problematic, and base that on current psycle-mfc or new designs.


For up to date information, don't forget to visit the Developement forums on
Psycledelics site.

--------------------------------------------------------------------------------

4. User Resources and Links:
----------------------------

* Psycledelics' Community:

This is the site of Psycle users. It contains several forums, documents and 
how-to's, download links and also a song section where that you can share
your compositions with others.
It is a relatively small community, and quite helpful.

		http://psycle.pastnotecut.org/

* Psycle's wiki:

There is an user contribute-able wiki site hosted in sourceforge which is worth
checking. There are several tutorials, news, and some how-to's. Take a visit any
day for information.

		http://psycle.sourceforge.net/wiki
		http://psycle.wikia.com/

* Psycle's site at sourceforge:

The developer page at sourceforge allows you to enter/track bug reports and
download current and old versions of Psycle, as well as other psycle related
files. You can also get the source code from the SVN repository:

	project page: http://sourceforge.net/projects/psycle
	development webpage : http://psycle.sourceforge.net/
	SVN repository : https://psycle.svn.sourceforge.net/svnroot/psycle/trunk


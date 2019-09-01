				------------------------------------------------
				]==-> Psycle Modular Music Creation Studio <-==[
				------------------------------- 1.8.5  ---------
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
- Support for VST1 and VST2 plugin standard.
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
Plans for supporting DXi are there, but it's not a primary work.
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
	

You are allowed to develop psycle Machines without any restriction
There is no SDK ready, so the best way to start a new one is getting the sources
of an existing one. Arguru synth is an usual template.

--------------------------------------------------------------------------------

3. Development:
---------------

* Psycle (windows/MFC)

	1.8: Current stable version. Development will follow with bugfixes and
	     new additions which don't imply big changes to the underlying
	     code.

	1.9/2.0: The existing source code of 1.9 will be discarded and 1.8.5
	     will serve as the next 1.9 codebase. It is expected to add
	     multichannel audio input/output, audio recording (live input),
	     separation of sources to different libs and multipattern sequence
	     ability, without losing the tracker feeling. Don't expect a Cubase,
	     instead, a tracker that would allow to make a pattern with pieces.

* qpsycle (linux/QT windows/QT)

	alpha: A native linux version is being developed with some code that
	       will be used for Psycle 1.9. It already contains a multipattern
	       sequencer, the posibility to have more effect columns, support
	       for LADSPA plugins and a new fileformat to contain all these
	       new additions.

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

* Psycle's site at sourceforge:

The developer page at sourceforge allows you to enter/track bug reports and
download current and old versions of Psycle, as well as other psycle related
files. You can also get the source code from the SVN repository:

	project page: http://sourceforge.net/projects/psycle
	development webpage : http://psycle.sourceforge.net/
	SVN repository : https://psycle.svn.sourceforge.net/svnroot/psycle/trunk



Also, chat with other Psycle users and developers in our IRC - Channel:
join #psycle on EFNet network (server list: http://www.efnet.org/servers.html)
If you don't have an irc client, you can use:
	http://psycle.sourceforge.net/irc.html
or
	http://psycle.free.fr/irc
with any "web" browser.

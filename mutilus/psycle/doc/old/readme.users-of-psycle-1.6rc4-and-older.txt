
		Instalation Instructions for previous users
		-------------------------------------------

A) The problem:
---------------

In this version of Psycle (1.6Final), we have changed the Way Plugins "talk" to Psycle
(we've changed the machineinterface.h a bit).

Doing so, previous plugins have stopped to work and we have had to recompile them again.


B) The changes:
---------------

* We have Removed All arguru synths except "Arguru Synth 2f".
Songs which have "Arguru Bass", "Arguru Synth" , "Arguru Synth2" and "Arguru Synth2.1" will
have these machines replaced by "Arguru Synth 2f" automatically.

* Arguru Synth 2.2 has been recompiled, but it is still labeled old

* Arguru Guru6 No longer exists (we don't have the sourceS so we cannot recompile it)

* M3 hasn't been updated either (we have the sources of the buzz plugin, so we will port it
  soon)

* All other plugins remain there.

C) Your ToDo:
-------------

Remove your old "psycle.exe" and your _NATIVE_ plugins. You can zip them if you have songs
that use Guru6 or M3 so that you can still listen to them.

D) Your DO NOT:
---------------

Do not unzip the new version in another directory and start it. If you do so, the new Psycle
version will try to load the OLD plugins because it saves the settings into the registry.
Either first change the directory location (in psycle settings, running the old version),
or replace the old files with the new ones.

Do not just unzip the new version over the old one without removing the old plugins.
If you don't do that, and load any of the old plugins (this includes loading a song),
psycle will crash.

IF YOU USE THE WINAMP PLUGIN, do not just indicate "Plugins\" dir for the plugins directory.
Even when this is a perfect solution to be able to run both versions (old and new) without
worrying, the winamp plugin won't be able to locate the plugins.
supercard sd patcher
====================

this program is a replacement for the original [supercard SD flashcart software](https://github.com/supercard-archive/supercard-sd-sw)
that only works on M$ (R) Windows (TM).
it allows to patch all known games (as of nointro set 202111) for usage on the
mentioned hardware on any standard UNIX platform.

how it works
------------

originally, my plan was to reverse engineer and decompile the original sw,
however due to it being written in Borland C++, this ended up to be way too
time-consuming.
so i ended up using a tedious, mostly [automated process](https://github.com/supercard-archive/supercard-sd-sw/commit/60e270ba15356ae63de6614ec8cde52faf38ed52)
to run the original supercard sd sw on the full nointro set
(except those with "x" gameid, like x001)
and created binary patches against the original rom.
the supercard software was configured so it would create patched roms with
4 different settings: "enable save", "enable saver patch", "enable restart",
"enable realtime save".
in order to minimize the size of the patches, i've been using truncated
input files, because that's the first step the original sw does when patching
a rom. care was taken to run numerous checks on the patched/truncated roms
to make sure they match the output of Supercardsd.exe with untruncated input.

upon executing `patcher`, it will extract the game code from the rom, and
then, depending on steps selected, apply one or more of those patches to a
copy of the input rom, then save it to output file, optionally accompanied
by sav and sci (real time save) files.

disadvantages of this approach
------------------------------
the original supercard sd sw has the following additional features, not covered
by this patchset:

- "enable coercive restart" - this seems to be an option designed for homebrew
  software to get out without a hard reset.
- cheats - the original sw has some cheats for some games
- "enable add text file" - the original sw has an option to append a textfile
  to a rom that can used via a key combination to read e.g. a game manual.

also naturally, this patcher can only be used with roms that are in the set
of patches created.

how the original supercard sw works
-----------------------------------
once you click the "add" button, supercardsd.exe copies the input file to
a random temporary filename in temp/, truncates the trailing 0xff bytes to the
last 16-byte aligned boundary, and patches 3 or so 4 byte words.
this is what i call the "base patch", which is identical to the output
when you select only "enable save". this base patching is done to every rom,
regardless of which settings you later select.
if "enable saver patch" is enabled, it then runs "srampatch.exe".
only if srampatch.exe fails with the bug i documented in its source code which
leads to an empty output file, it will then call "gep.exe" instead.
gep.exe is basically the same as srampatch.exe, but only targetting EEPROM
roms. if gep.exe needs to be called, supercardsd.exe further truncates the
input rom to a 4-byte aligned (so it might become shorter than first truncation
step), pads it with 0x200 zero bytes, then calls gep.exe which then puts its
payload into that padding if needed.
i didn't investigate in detail yet what is done for the steps "enable restart"
and "enable real time saver", but i suspect generic code which is stored in
Supercardsd.exe itself or inside rominfo.dat most likely based on
Boot-O-Mat Advance aka [BOMA](http://www.xromguide.50webs.com/boma.htm), which
i didn't have the opportunity yet to analyze/dissassemble/decompile due to not
having found a copy of it so far.
the code in Supercardsd.exe that deals with rominfo.dat seems to use some
primitive xor-based encryption using the current file offset as key.


requirements
------------
- a C compiler (at least during first execution)
- [haxdiff](https://github.com/rofl0r/haxdiff) installed in PATH
- POSIX conforming shell and base utilities
- contents of this repo cloned into a writable directory
- max ~100 MB of free storage for temporary files

usage
-----
run `patcher` and read the usage output. on first execution, it will compile
2 helper programs with the system's C compiler.

if an option is selected that would create a SAV or SCI file using the original
software, either `[i]save.sav/sci` or `more.sav/sci` from either sav or rts
directories are copied into place. sav/ contains several more, game-specific
.sav file templates that aren't automatically selected (supercardsd.exe doesn't
either). these sav files use the numeric id that was commonly used during the
times were GBA games were still released. unfortunately recent no-intro sets/
[Dat-O-Matic](http://datomatic.no-intro.org/) engineers decided to change the
numbering, so you need to look up the original number of your rom using an
old list.
i put one into this repo as `oldromnumbers.txt` but don't know whether it's
identical to the one used back when supercardsd.exe was developed.

alternatives
------------
in case you need to patch a ROM that's not covered by this software, the
following alternatives come to mind:

- run original Supercardsd.exe manually on the rom using wine
- use the reverse-engineered [littlewriter X-Rom](http://www.xromguide.50webs.com/littlewriter.htm)'s [srampatch.exe sourcecode](https://github.com/supercard-archive/supercard-sd-sw/blob/master/srampatch.c)
  to at least make battery saves possible
- [ezgba](https://github.com/langest/ezgba)
  open source application that does roughly the same as srampatch.exe.
  unfortunately it's a gui app and written in C++, so it might be difficult
  to compile and use it. i'd be really happy if someone would take this
  and remove the GUI crap and replace C++ by C code.
- gbata, a program produced by some folks at gbatemp.net. i haven't come
  across its source code so far.

about supercard sd battery saving mechanism
-------------------------------------------
after your rom is patched with battery saver support and doing a regular
in-game save, your save isn't actually being written to SD card, as you
would expect. rather it's stored in the flashcart's SRAM. in order to
tell the supercard firmware to actually store it on the SD card, you need to
hit L+R+SEL+A and select "save" (and if you use "more saver" option, select
one of the 4 64KB save slots).
one major limitation of the supercard firmware is that it treats all saves
as 64KB, even for games that use 128KB aka 1MBit saves such as the pokemon
series. in case of pokemon, you're lucky - at least firered version uses
its 1MBit save memory to store TWO copies, or rather last save and current
save. therefore it will always detect the second save slot as corrupt, but
then successfully restores the "old" save. just make sure to always save
to the first slot when selecting "more saver" option when patching.
according to pokemon firered decompilation project's src/save.c, each
save slot takes up 56KB (14*4KB), so you'll never be able to successfully
use the second save slot, nor hall of fame/battle recording features,
which follow slot 2.
since the game stores alternating to slot 1 and 2, it's probably a good idea
to save TWICE in-game and once using the SCSD firmware before powering off
the device, so the lower slot is always using latest data.

README for Linux DOOM Source distribution
=========================================

DISCLAIMER
----------
This is not "The DOOM Source Code" dump for a bunch of reasons. It is based on a
DOOM development directory snapshot as of January 10th, but has been stripped
and changed. Thus it is the DOOM source, but there are many minor differences to
the source as last used by id Software.

Note that thus neither John Carmack nor Dave Taylor nor anybody else at id is
responsible for the contents of this archive, or the changes introduced to the
original source.

If there are any questions, contact me at bk@gamers.org, or preferably post to
the mailing list at
    doom-editing@gamers.org
(send mail to majordomo@gamers.org, content just a single "info
doom-editing"). I will post any update or notification of corrections there. I
will probably put some stuff at
    http://www.gamers.org/dEngine/doom/
as well. Look there for the "Unofficial DOOM Specs" as minimal recommended
documentation.

REMARKS
-------
I made a few minor bug fixes, added some experimental sound code, and changed
the handling of IWAD dependent game modes. Most of the changes though have been
shuffling around sources in a sometimes futile attempt to separate modules more
cleanly, and make certain parts easier to locate and modify. There is still much
left to do, but I hope that the current source is a good base to start with,
especially with a cooperative effort in mind. Those so inclined will find the
source prepared for CVS.

there is a list of changes and fixes I did not get around to in TODO, and an
incomplete worklog in ChangeLog, that also includes some minor TODO statements
scattered throughout the log.

A) Linux SVGA
There is no SVGA support. For development and debug purposes, the X11 version
seems to be more handle

B) Sound - see README.sound, and the sndserver.tgz archive.

C) GLDOOM - see README.gl

D) Win32
There is no Win32 support in the original dump.

E) DOS
Original DOS support (including the texture mapping and fixed point assembler)
hsa been removed, mainly because of the lack of sound support.

F) DoomEd
The NeXTStep DoomEd sources in the dump were garbled (filenames - prolly an
issue of ISO9660 with or w/o extensions). Somehow Bear never got around to send
me a list of the correct filename, and I won't bother guessing without a NeXT
box at hand.

There is a plethora of useful editors for DOOM. I suggest DEU for X11.

G) BSP Tools
the BSP builder and other tools have been released by John Carmack long ago, and
since improved/replaced by others. Again I recommend taking a pick among the
tools available for Linux.

H) DOOM game tools
There are a number of tools that have not been released, namely those which
compiled the Things and State Tables, the frame animation LUT's, sound tables
etc. Basically, they compile similarly complex LUT's to general C files. the
tools are omitted from this distribution.

There are some files in the distribution (info.h/c, sounds.h/c) that are
essentially the output of these tools. This is the data that defines DOOM (as a
game) for all practical purposes.

I recommend keeping them, as they are part of the source. In the long run,
handling them as well as the action/animation functions as a separate game.so
library (as with Quake2) seems to be a good idea.

I) Artwork
Neither the original artwork nor the misc. WAD files are included in this
archive. You will at least need the shareware WAD file to run the executable,
but it shouldn't be too difficult to get a hold of that.

Note that the mechanism to detect the presence of a registered or commercial
version is still in the source, and homebrew maps are still disabled. This is
easily removed now, but as FinalDOOM, Ultimate DOOM and DOOM 2 are still in the
shops, it is probably polite not to distribute a soure or binary without that
mechanism.

This version of LinuxDoom supports Plutonia and TNT WAD from FinalDOOM as
well. No guarantees, though.

Enoy!

b. 97/12/22
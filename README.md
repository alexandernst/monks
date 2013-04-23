Procmon
=======

Procmon alternative for Linux


This is a kernel module that hijacks sys_calls and printfs messages whenever a sys_call is called.
In the future, instead of printfs-ing messages, some kind of events will be sent to an UI which will
be similar to what Procmon (for Windows) offers right now.

Keep in mind that this is a WIP and you can end up with a totally frozen kernel!


In order to build this module you'll need some basic stuff (make, gcc) and the headers of the kernel 
you're running on.
Once you have all those you just need to run ```make```.

Loading the module isn't any different from loading any other module. ```insmod procmon.ko``` for 
loading it and ```rmmod procmon.ko``` for unloading it.

To start the actual hijack process, once loaded the module, run ```echo 1 /proc/procmon```.
Once started, you'll probably want to run ```dmesg -w``` (or ```dmesg``` if your kernel doesn't support ```-w```)
in another console to see an actual outpu.

To stop it just run ```echo 0 /proc/procmon```.

Also keep in mind that unloading the module without stopping it previously will *probably* cause
some bad stuff, maybe even data loss. You have been warned.
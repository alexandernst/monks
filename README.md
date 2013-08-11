Procmon
=======

Procmon alternative for Linux


This is a kernel module that hijacks sys_calls and printfs messages whenever a sys_call is called.
In the future, instead of printfs-ing messages, some kind of events will be sent to an UI which will
be similar to what Procmon (for Windows) offers right now.

Keep in mind that this is a WIP and you can end up with a totally frozen kernel!


In order to build this module you'll need some basic stuff (make, gcc) and the headers of the kernel 
you're running on.
Once you have all those you just need to run ```make``` inside the ```procmon_kmodule``` folder.

Loading the module isn't any different from loading any other module. ```insmod procmon.ko``` for 
loading it and ```rmmod procmon.ko``` for unloading it.

To start the actual hijack process, once loaded the module, run ```echo 1 > /proc/procmon```.
Once started, you'll probably want to run ```dmesg -w``` (or ```watch -n 0.1 -t 'dmesg | grep -v " grep " | grep -v " tail " | tail -50'``` if your kernel doesn't support ```-w```)
in another console to see an actual output.

To stop it just run ```echo 0 > /proc/procmon```.

Also keep in mind that unloading the module without stopping it previously will *probably* cause
some bad stuff, maybe even data loss. You have been warned.

The UI part will be based on ```rbcurses```. You'll need Ruby 1.9.3 or newer to play with this part.
First you need to build the Ruby C extension that will allow playing with ```kmod``` from Ruby. Go to the
```procmon``` and run ```ruby extconf.rb``` and then ```make```. If everything went fine you'll be able to
go to the main directory and run ```ruby procmon.rb``` (which, for now, will just check if the module is loadad,
and if it isn't it will load it and unload it).

Why Procmon
=======

I'm completely aware of ```kprobes```, ```perf``` and all other kernel debug systems/methods. Probably all of them work better than Procmon, but they have one disadvantage: they require you to recompile the kernel or they are not enabled by default in some distros.

On the other hand, Procmon will ```just work```.
What this module does to ```just work``` is hijack/replace all (relevant/interesting) syscalls from the syscall table. While this is risky, it will allow you to have a similar tool to Procmon for Windows, without having to recompile the kernel.

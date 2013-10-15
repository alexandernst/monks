Procmon
=======

[![Build Status](https://drone.io/github.com/alexandernst/procmon/status.png)](https://drone.io/github.com/alexandernst/procmon/latest)

Procmon alternative for Linux - [Main webpage](http://alexandernst.github.io/procmon "Procmon's Homepage")


This is a kernel module that hijacks sys_calls and printfs messages whenever a 
sys_call is called. In the future, instead of printfs-ing messages, some kind 
of events will be sent to an UI which will be similar to what Procmon (for 
Windows) offers right now.

Keep in mind that this is a WIP and you can end up with a totally frozen 
kernel!


In order to build this module you'll need some basic stuff (make, gcc) and the 
headers of the kernel you're running on. Once you have all those you just need
to run ```make``` inside the root folder.

Loading the module isn't any different from loading any other module. 
```insmod procmon.ko``` for loading it and ```rmmod procmon.ko``` for 
unloading it.

To start the actual hijack process, once loaded the module, run 
```sysctl procmon.state=1```. Once started, you'll probably want to run 
```./procmon-viewer``` to see an actual output.

To stop it just run hit ```Ctrl + C```. To stop the module run 
```sysctl procmon.state=0```.

Keep in mind that the module will protect your kernel while unloading. That 
means that if any process (both in userland and in the kernel itself) expect 
to call one of the hijacked syscalls, the module will wait those processes to 
run what they need to run. This may take from 1ms to days. If there's a really 
long delay, try killing/restarting some processes that may have scheduled a 
call. For example, the module won't unload until you press ```Enter``` on all 
consoles that had any activity while the module was loaded.

The UI part will be based on ```rbcurses``` (may change). You'll need Ruby 
1.9.3 or newer and Ruby-dev to play with this part. Note that this is 
experimental and has absolutely no support at the moment. Basic instructions:

First you need to build the Ruby C extension that will allow playing with 
```kmod``` from Ruby. Go to the ```procmon/ui``` and run ```ruby extconf.rb``` 
and then ```make```. If everything went fine you'll be able to run 
```sudo ruby procmon.rb``` in the root directory.

![Screenshot](https://raw.github.com/alexandernst/procmon/screenshots/screenshot1.jpeg)

Why Procmon
=======

I'm completely aware of ```kprobes```, ```perf``` and all other kernel debug 
systems/methods. Probably all of them work better than Procmon, but they have 
one disadvantage: they require you to recompile the kernel or they are not 
enabled by default in some distros.

Yet another reason: I have fun doing it! I don't seek for this project to be 
merged into mainline nor being used by every Linux user out there. I'm doing 
it for myself. Anyways, I'd be glad if it works for you too :)

On the other hand, Procmon will ```just work```.
What this module does to ```just work``` is hijack/replace all 
(relevant/interesting) syscalls from the syscall table. While this is risky, 
it will allow you to have a similar tool to Procmon for Windows, without having
to recompile the kernel.

Contributing
=======

Just send me patches, if they are ok I'll give you push access :)

About the editing, note that I'm using ```TAB```s, so please keep it that way.

License
=======

The license is WTFPL (Do What The Fuck You Want To Public License), but keep
in mind it's good for both sides if you use this project, fix/add things and
push them back.
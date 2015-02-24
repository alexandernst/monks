[![Build Status](https://drone.io/github.com/alexandernst/monks/status.png)](https://drone.io/github.com/alexandernst/monks/latest) Procmon alternative for Linux - [Main webpage](http://alexandernst.github.io/monks "Monks's Homepage")

What is Monks
=======

Monks is a kernel module that hijacks sys calls and sends information about which
processes called which sys calls, with what arguments did they call them with, 
what was the return value, etc, and sends that information to a nice ncurses 
interface.

Said in another way, Monks is like ```strace```, but tracing all and every single
process from any user, at any level.

That's how it works:

![How it works](https://raw.github.com/alexandernst/monks/master/screenshots/monks.gif)

Why that name?
=======

At first I thought naming the project ```Procmon```, but because that name is already
a registered trademark and I don't want any troubles, I decided to call it ```Monks```,
which stands for ```MONitoring Kernel Syscalls```.

Setting Monks
=======

Keep in mind that this is a WIP and you can end up with a totally frozen 
kernel! Do *NOT* run this in production machines. I'm *NOT* responsible
for any data loss or damage in any way. That said, I test on a daily basis
this module on quite some virtual machines, 7, to be precise. Both x86 and
x64, different distros, different compilers, different kernels from 2.6.37
up to 3.12.

In order to build this module you'll need some basic stuff (make, gcc), the 
headers of the kernel you're running on and ncurses library. Once you have all
those you just need to run ```make``` inside the root folder of the project.

Loading the module isn't any different from loading any other module. 
```insmod monks.ko``` for loading it and ```rmmod monks.ko``` for 
unloading it.

To start the actual hijack process, once loaded the module, run 
```sysctl monks.state=1```, then you'll probably want to run 
```./monks-viewer``` to see an actual output.

To stop it just run hit ```q```. To stop the module run ```sysctl monks.state=0```.

If your distro has ```libkmod```, you can use monks's viewer command line
switches instead to do all those actions (load/unload, start/stop).

The UI is based on ```ncurses```. Anyways, right now there is almost no functional
code, just a basic viewer.

![Screenshot](https://raw.github.com/alexandernst/monks/master/screenshots/screenshot1.jpeg)

Why Monks
=======

I'm completely aware of ```kprobes```, ```perf``` and all other kernel debug 
systems/methods. Probably all of them work better than Monks, but they have 
one disadvantage: they require you to recompile the kernel or they are not 
enabled by default in some distros.

Also, Monks will ```just work``` (UI included). What this module does to ```just work``` is
hijack/replace all (relevant/interesting) syscalls from the syscall table.
While this is risky, it will allow you to have a similar tool to Procmon for 
Windows, without having to recompile the kernel.

Yet another reason: I have fun doing it! I don't seek for this project to be 
merged into mainline nor being used by every Linux user out there. I'm doing 
it for myself. Anyways, I'd be glad if it works for you too :)

Contributing
=======

Just send me patches, if they are ok I'll give you push access :)

About the editing, note that I'm using ```TAB```s, so please keep it that way.

License
=======

The license is WTFPL (Do What The Fuck You Want To Public License), but keep
in mind it's good for both sides if you use this project, fix/add things and
push them back.

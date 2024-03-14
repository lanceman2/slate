To edit this file see:
[Markdown-Cheatsheet](https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet)

# The main idea

We want to do 2D drawing very quickly.  The wayland protocol may just help
us do that while letting us still have a desktop windowing environment
that lets us run preexisting programs at the same time as we do 2D drawing
very quickly.  By using a wayland client we should out-perform X11 client
programs.  We can draw and color a pixels on the screen, and much more
directly than we ever could with X11 clients.  Each "drawing step" does
not necessitate a system call like with an X11 client.


## This totally explains what wayland is

See this C code
[hello wayland](https://github.com/emersion/hello-wayland.git).

We are currently developing "slate" on Debian GNU/Linux 12 (bookworm) with
the Gnome 3 desktop.  An older version of Gnome 3 desktop was missing
symbols needed to compile hello wayland client program without linking to
a fuck ton of libraries.  But, now I can compile and run the "hello
wayland" program with just:
```sh
  $ ldd ./hello-wayland
```
```
	linux-vdso.so.1 (0x00007ffc2dcec000)
	libwayland-client.so.0 => /lib/x86_64-linux-gnu/libwayland-client.so.0 (0x00007f7a87371000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f7a87190000)
	libffi.so.8 => /lib/x86_64-linux-gnu/libffi.so.8 (0x00007f7a87184000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f7a8717f000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f7a873c1000)
```

Testing shows that it runs with just those libraries (it does not seen to
dynamically load more libraries); see by running:
```sh
  $ LD_DEBUG=files ./hello-wayland
```


## Testing libwayland-client.so and libffi.so for system resource leaks




We'll add that as a test program in ./tests


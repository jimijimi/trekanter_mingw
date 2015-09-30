This is the C version of Trekanter, a very simple STL geometry viewer for the
WIndows OS. This version improves and extend the previously released trekante.rb Ruby version.

TrekantER C
===========


- is faster since it is written in C ( really? ).
- can handle 'n' number of geometries, the limitation is the local host memory.
- can read STL binary and ascii files
- can display additional information such as bounding box size, triangle count.
- can display the mesh details.
- can show different background colors.
- has a file chooser dialog window.
- can run in in windows versions from Windows 2000 to Windows 10.
- can run in ReactOS.
- can run in Wine.
- has full zoom, pan and rotations ( using quaternions ) capability.
- is compiled into a compact single EXE file of ~250 kb in size.
- can be compiled under 32 bits and 64 bits.
- requires no installation, the exe file can copied to any location and run with a
  simple double-click or from the command line by invoking the application name.
- compiled version will be made available for download in bitbucket/github.


Treakenter C is not perfect and many changes will be carried out in the near/mid future.

- The current arquitecture is (very) messy and a new one is under development.
- Has a lot global variables. Because of this the code looks spaghetti-esque.
  This will be handled in the next arquitecture.
- It uses OpenGL 1.1/1.4 with immediate mode ( Yes, you hear well ) and it will
  maintained for legacy systems. However Modern OpenGL will also be utilized in
  the new version.
- Windows only for the moment but it can run on Linux using Wine. Native Linux version
  will be considered in the future.
- As of today it is unknown if Trekant.c will compile under a real Windows environment.
  All the development was carried out under Debian 7/8 using the win32-mingw toolchain.
  The EXE file however runs perfectly fine in basically any Windows version.
- The code can be further improved to gain additional speed. Real C programmers will
  immediately see some awkward code. 
- The code lacks comments but I believe most of it is self-explanatory.
- The code contains some 'magic' numbers those will be addressed in a future version.  


The development environment is:

- Debian 8
- x86_64-w64-mingw32-gcc 4.9.1 to produce 64 bit windows executables
- i686-w64-mingw32-gcc 4.9.1 to produce 32 bit windows executables
- icoutils 0.31.0
- GNU Emacs 24.4.1
- GNU make 3.81


License
=======


TrekanER C has been released under the GPL v2. See the LICENSE.txt file for more information.


Author
======


Jaime Ortiz.  quark dot charm at gmail.com


Compilation
-----------

     $ make <enter>
	
This will produce two exe files, one for Windows 32 bits and the other for Windows 64 bits.

     $ trekanter --help  <enter>

To show the help.

  


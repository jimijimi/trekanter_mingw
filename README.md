This is the C version of Trekanter, a very simple STL geometry viewer for the
WIndows OS. This version improves and extend the previously released trekante.rb Ruby version.

Nov 23, 2015: All necessary files have been uploaded and the program should compile without
issues.

Nov 21, 2015: I am about to finish to upload the code. I have
been doing cleaning and text format changes. It may take a few more days to fully complete that.


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
- can be compiled under 32 bits and 64 bits.
- requires no installation, the exe file can copied to any location and run with a
  simple double-click or from the command line by invoking the application name.
- compiled version will be made available for download in bitbucket/github.
- Very Small size ~350 kb. Can I make it smaller?
- It can load any STL file (ASCII or Binary) and of any size.
  The only limitation is the system memory.



Treakenter C is not perfect and many changes will be carried out in the near/mid future.

- The current arquitecture work will stop and a new one is under development.
- Has a lot global variables. Because of this the code looks spaghetti-esque.
  This will be handled in the next arquitecture.
- It uses OpenGL 1.1/1.4 with immediate mode ( Yes, you hear well ) and it will
  maintained for legacy systems. However Modern OpenGL will also be utilized in
  the new version.
- Windows only for the moment but it can run on Linux using Wine with no issues.
  A native Linux version will be considered in the future.
- As of today it is unknown if Trekant.c will compile under a real Windows environment.
  All the development was carried out under Debian 7/8 using the win32-mingw toolchain.
  The EXE file however runs perfectly fine in basically any Windows version. I have tested all of
  them since WIndows XP.
- The code can be further improved to gain additional speed. Real C programmers will
  immediately see some awkward code. I am also working on this.  
- The code lacks comments but I believe most of it is self-explanatory.
- The code contains some 'magic' numbers those will be addressed in a future version.  
- Selection has not been implemented yet. I have a ruby prototype with selection now and
  I will translate it to C soon.
- It will read any NASTRAN files ( short, long, open format and with/withou continuation lines )
- It will read OBJ/wavefront files.  
- I will try to include additional formats if I can find the "format" specification.


Trekanter C what is in the future.

- Under the hood my own CFD engine. 
- More to come, I will be writting my progress here. 
  

My development environment is:

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

Install the crosscompilation toolchain indicated above. It should work on any Linux machine.

     $ make <enter>
	
This will produce two exe files, one for Windows 32 bits and the other for Windows 64 bits.

     $ trekanter32 --help  <enter>
     $ trekanter64 --help  <enter>

To show the help. 

  
Precompiled Binary Builds
=========================

The folder builds/ contains precompiled binaries

32-bit build checksums. trekanter32.exe

md5sum    4D65AB6828F72021E7A3A94A02AE7D4B
sha1sum   A8ECF77D0226F3D612050C32C50C0646E80E2D2D
sha256sum 3D3C3573C1F5BBE5660929E6DF4ABB1050E9A4BAC6D32C5F87750118CB7619B4

64-bit build checksums. trekanter64.exe

md5sum    3AC6BD93DC4572908B11CCC7AAADDBBD    
sha1sum   CCB48FCA5EEAEE24FEE3B0E0C90D713847468791
sha256sum B0925E7E3A45E77B850DD16848E39D96FE175B84EF2E2257A68EA4FF753539C7


Screenshots
===========

![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_001.jpg)
![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_002.jpg)
![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_003.jpg)
![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_004.jpg)
![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_005.jpg)
![Picture](https://raw.github.com/jimijimi/trekanter_mingw/master/pictures/trekanter_006.jpg)

The Tesla Model S 3D model is from: http://3dmag.org/en/market/download/item/227


FUNCTIONALITY
----------------------
CC2430/1 flash programming
CC2430   command line debugger
CC2430   NoICE 8051 Debug server

INSTALLING
----------------------
Linux
----------------------
  You need libusb
  then
  $ make
  
Windows
----------------------
  cygwin with libusb( +install the included driver )
  $ make -f Makefile.win32
  
USING
----------------------
For help
$ cc_flasher -h

For writing flash
$ cc_flasher --filename <binary file> --write

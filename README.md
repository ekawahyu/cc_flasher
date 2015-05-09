# CC_Flasher
This a copy of the legacy [cc_flasher](http://sourceforge.net/projects/ccflasher/) software by Modula.si, to program TI CC2430 chip. This repository includes minor fixes to build on OS X (Yosemite) with [macports](http://www.macports.org/). The original project seems to be abandoned because TI came out with a cheaper solution to flash its entire line of 802.15.4 SoC using [CC Debugger](http://www.ti.com/tool/cc-debugger).

TI provides only software support for Windows. Fortunately, there is an open source tool called [cc-tool](https://github.com/dashesy/cc-tool) for Linux and OS X users. It is still at Alpha stage but usable.

The cc_flasher can be a replacement of cc-tool to flash CC2430.

##Dependencies
Macport GCC - any release should work, tested with gcc48.

##Build instruction
1. Install gcc48 using macport
2. Select default gcc to mp-gcc48
3. Change directory to cc_flasher
4. Type 'make'
5. Type 'sudo make install', and enter your root password

##How to use
Type 'cc-tool -h' to show the help screen

##No more firmware update for the hardware programmer
The contact person I talked to from Modula.si was Peter Kuhar. He said that there will be no more firmware update for hardware programmer, nor support for latest TI chips like CC2530. Do not use this repository for new project development. It is provided here as a legacy only.
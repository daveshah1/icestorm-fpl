# Icestorm Workshop at FPL 2019

This is the material for the Icestorm Open Source iCE40 tools workshop at
FPL 2019, using the Icebreaker development boards by 1bitsquared.

## Getting Started

 - If on a provided Pi, no setup is needed and everything is ready to go.
 - If you want to work on this tutorial on your own laptop:
 	- First follow the setup instructions at
 	  http://www.clifford.at/icestorm/
 	  (see "Where are the Tools? How to Install?" and also the UDEV rule
 	  instructions)

 	- Install any 32-bit or 64-bit RISC-V toolchain. As both 32-bit and
 	  64-bit gcc can build 32-bit binaries when standard libraries aren't
 	  needed, the "riscv64-linux-gnu-gcc" or "riscv64-unknown-elf-gcc"
 	  usually provided by a distro suffice.

 	- Install a serial terminal. This tutorial uses `screen`, which you
 	  may well have installed already.

## Examples

 - The first example is a simple "blinky" to make sure everything is
	  working and get to grips with the tooling.
 - The second example example is a RISC-V SoC with a slightly unusual
	  peripheral...
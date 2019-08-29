# 2 picosoc

## Overview

PicoSoC is a small, low power 32-bit RISC-V system-on-chip comprising of the
PicoRV32 processor core, RAM, a UART, an SPI flash controller for code storage,
and some simple interconnect.

PicoSoC and PicoRV32 are developed by Clifford Wolf and licensed under the
permissive ISC license. See https://github.com/cliffordwolf/picorv32 for
more information.

This example builds PicoSoC for the icebreaker FPGA board, taking advantage
of the 128kB of single-port RAM in the UltraPlus for main memory.

Note you will need a RISC-V toolchain to build this project. 64-bit RISC-V
toolchains can build 32-bit code (so long as it doesn't depend on the any
standard libraries) and are usually more commonly provided by distributions.
Usually the package you want to install is `riscv64-linux-gnu-gcc` or
equivalent.

## Structure

Gateware:
 - picorv32.v is the PicoRV32 processor core
 - simpleuart.v is the UART core
 - spimemio.v is a memory-mapped SPI/DSPI/QSPI flash controller
 - ice40up5k_spram.v is a wrapper for the 128kB SPRAM hard block
 - picosoc.v glues together PicoRV32, the UART, the RAM the SPI flash
 - peripheral.v is the configurable logic peripheral (see below)
 - icebreaker.v is the top level design

 Software:
  - firmware.c is the main C firmware
  - start.s implements pre-C startup and some low level SPI flash
    control routines
  - sections.lds is the linker script

## Configurable Logic

Attached to the system-on-chip is a custom peripheral, designed to mimic
a simple configurable logic device. It contains 8 programmable 4-input
gates, each of which implements 4-input OR, AND, XOR or NAND. Imagine
that this is what a Zynq might have looked like if such a thing existed
in the early 80s...

Each gate input can be selected from 8 choices: constant 0 or 1,
the 3 button inputs (named A-C), or 3 intermidate signals (named
X-Z.)

5 of the outputs of these gates drive LEDs (named E-I), the remaining 3
are  "temporary" signals X-Z that can be used as intermediate values
input into other gates.

The configuration for this logic is memory mapped. The firmware
implements a simple shell to inferface with this.

## Running the example

If everything is set up correctly run

    $ make prog

This will build and program both the FPGA gateware and firwmare. To
program just the firmware, run

    $ make prog_fw

Very occasionally programming will fail if the flash is interrupted
in the middle of an operation. Try again if this happens and it will
usually work.

Connect to the serial console (you can also use another tool like
`cu` if you prefer):

    $ screen /dev/ttyUSB1 115200

You should see a "Press ENTER to continue.." prompt. Press Enter,
and it will then boot to the logic console. Use "?" for help on
the syntax for the logic configuration.

Try out programming the configurable logic peripheral with a full
adder:

    > R
    > X=^AB
    > Y=&AB
    > Z=&XC
    > E=^XC
    > F=^YZ

Test the full adder using the buttons.

## Exercises

These may be attempted in any order, depending on what you feel
like.

  1. Try implementing simple logic circuits using the prompt.
    Examples:
     - Build a 3:5 decoder
   	 - Test that Demorgan's theorem works
   	 - Build SR or D latches
  2. Extend the logic with "registers", storing the last value
     of the gates with a low frequency divided clock (see the
     skeleton code and notes in comments). In the prompt, use
     lower case letters to represent the registered version of
     a signal. Use this to build a small counter.
  3. Add a readback function, by connecting up `mem_rdata`
     accordingly (only one address is needed, so responding
     to all addresses is fine.) Typing 'P' should print the
     current state of all inputs and outputs.

Alternatively, try modifying it differently if you have an
interesting idea!

Have fun!
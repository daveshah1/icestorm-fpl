/*
 *  PicoSoC - A simple example SoC using PicoRV32
 *
 *  Copyright (C) 2017  Clifford Wolf <clifford@clifford.at>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef ICEBREAKER
#  define MEM_TOTAL 0x20000 /* 128 KB */
#elif HX8KDEMO
#  define MEM_TOTAL 0x200 /* 2 KB */
#else
#  error "Set -DICEBREAKER or -DHX8KDEMO when compiling firmware.c"
#endif

// a pointer to this is a null pointer, but the compiler does not
// know that because "sram" is a linker symbol from sections.lds.
extern uint32_t sram;

#define reg_spictrl (*(volatile uint32_t*)0x02000000)
#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)
#define reg_leds (*(volatile uint32_t*)0x03000000)

// --------------------------------------------------------

extern uint32_t flashio_worker_begin;
extern uint32_t flashio_worker_end;

void flashio(uint8_t *data, int len, uint8_t wrencmd)
{
	uint32_t func[&flashio_worker_end - &flashio_worker_begin];

	uint32_t *src_ptr = &flashio_worker_begin;
	uint32_t *dst_ptr = func;

	while (src_ptr != &flashio_worker_end)
		*(dst_ptr++) = *(src_ptr++);

	((void(*)(uint8_t*, uint32_t, uint32_t))func)(data, len, wrencmd);
}

#ifdef ICEBREAKER
void set_flash_qspi_flag()
{
	uint8_t buffer[8];

	// Read Configuration Registers (RDCR1 35h)
	buffer[0] = 0x35;
	buffer[1] = 0x00; // rdata
	flashio(buffer, 2, 0);
	uint8_t sr2 = buffer[1];

	// Write Enable Volatile (50h) + Write Status Register 2 (31h)
	buffer[0] = 0x31;
	buffer[1] = sr2 | 2; // Enable QSPI
	flashio(buffer, 2, 0x50);
}

void set_flash_mode_spi()
{
	reg_spictrl = (reg_spictrl & ~0x007f0000) | 0x00000000;
}

void set_flash_mode_dual()
{
	reg_spictrl = (reg_spictrl & ~0x007f0000) | 0x00400000;
}

void set_flash_mode_quad()
{
	reg_spictrl = (reg_spictrl & ~0x007f0000) | 0x00240000;
}

void set_flash_mode_qddr()
{
	reg_spictrl = (reg_spictrl & ~0x007f0000) | 0x00670000;
}

void enable_flash_crm()
{
	reg_spictrl |= 0x00100000;
}
#endif

// --------------------------------------------------------

void putchar(char c)
{
	if (c == '\n')
		putchar('\r');
	reg_uart_data = c;
}

void print(const char *p)
{
	while (*p)
		putchar(*(p++));
}

void print_hex(uint32_t v, int digits)
{
	for (int i = 7; i >= 0; i--) {
		char c = "0123456789abcdef"[(v >> (4*i)) & 15];
		if (c == '0' && i >= digits) continue;
		putchar(c);
		digits = i;
	}
}

void print_dec(uint32_t v)
{
	if (v >= 1000) {
		print(">=1000");
		return;
	}

	if      (v >= 900) { putchar('9'); v -= 900; }
	else if (v >= 800) { putchar('8'); v -= 800; }
	else if (v >= 700) { putchar('7'); v -= 700; }
	else if (v >= 600) { putchar('6'); v -= 600; }
	else if (v >= 500) { putchar('5'); v -= 500; }
	else if (v >= 400) { putchar('4'); v -= 400; }
	else if (v >= 300) { putchar('3'); v -= 300; }
	else if (v >= 200) { putchar('2'); v -= 200; }
	else if (v >= 100) { putchar('1'); v -= 100; }

	if      (v >= 90) { putchar('9'); v -= 90; }
	else if (v >= 80) { putchar('8'); v -= 80; }
	else if (v >= 70) { putchar('7'); v -= 70; }
	else if (v >= 60) { putchar('6'); v -= 60; }
	else if (v >= 50) { putchar('5'); v -= 50; }
	else if (v >= 40) { putchar('4'); v -= 40; }
	else if (v >= 30) { putchar('3'); v -= 30; }
	else if (v >= 20) { putchar('2'); v -= 20; }
	else if (v >= 10) { putchar('1'); v -= 10; }

	if      (v >= 9) { putchar('9'); v -= 9; }
	else if (v >= 8) { putchar('8'); v -= 8; }
	else if (v >= 7) { putchar('7'); v -= 7; }
	else if (v >= 6) { putchar('6'); v -= 6; }
	else if (v >= 5) { putchar('5'); v -= 5; }
	else if (v >= 4) { putchar('4'); v -= 4; }
	else if (v >= 3) { putchar('3'); v -= 3; }
	else if (v >= 2) { putchar('2'); v -= 2; }
	else if (v >= 1) { putchar('1'); v -= 1; }
	else putchar('0');
}

char getchar_prompt(char *prompt)
{
	int32_t c = -1;

	uint32_t cycles_begin, cycles_now, cycles;
	__asm__ volatile ("rdcycle %0" : "=r"(cycles_begin));

	reg_leds = ~0;

	if (prompt)
		print(prompt);

	while (c == -1) {
		__asm__ volatile ("rdcycle %0" : "=r"(cycles_now));
		cycles = cycles_now - cycles_begin;
		if (cycles > 12000000) {
			if (prompt)
				print(prompt);
			cycles_begin = cycles_now;
			reg_leds = ~reg_leds;
		}
		c = reg_uart_data;
	}

	reg_leds = 0;
	return c;
}

char getchar()
{
	return getchar_prompt(0);
}

#define LINEBUF_SIZE 80
char linebuf[LINEBUF_SIZE + 1];
int linebuf_ptr;

int getline()
{
	linebuf_ptr = 0;
	unsigned char c = 0;
	while (1) {
		c = reg_uart_data;
		if (c == 0xFF)
			continue;
		if (c == '\r' || c == '\n')
		{
			print("\r\n");
			linebuf[linebuf_ptr] = '\0';
			return linebuf_ptr;
		} else if (c == '\b' || c == 0x7F) {
			if (linebuf_ptr > 0) {
				print("\b \b");
				--linebuf_ptr;	
			} else {
				print("\a");
			}
		} else {
			if (linebuf_ptr < LINEBUF_SIZE) {
				if (c != ' ')
					linebuf[linebuf_ptr++] = c;
				putchar(c);
			} else {
				print("\a");
			}
		}
	}
}

void cmd_print_spi_state()
{
	print("SPI State:\n");

	print("  LATENCY ");
	print_dec((reg_spictrl >> 16) & 15);
	print("\n");

	print("  DDR ");
	if ((reg_spictrl & (1 << 22)) != 0)
		print("ON\n");
	else
		print("OFF\n");

	print("  QSPI ");
	if ((reg_spictrl & (1 << 21)) != 0)
		print("ON\n");
	else
		print("OFF\n");

	print("  CRM ");
	if ((reg_spictrl & (1 << 20)) != 0)
		print("ON\n");
	else
		print("OFF\n");
}

// --------------------------------------------------------

void cmd_read_flash_id()
{
	uint8_t buffer[17] = { 0x9F, /* zeros */ };
	flashio(buffer, 17, 0);

	for (int i = 1; i <= 16; i++) {
		putchar(' ');
		print_hex(buffer[i], 2);
	}
	putchar('\n');
}

#ifdef ICEBREAKER
uint8_t cmd_read_flash_reg(uint8_t cmd)
{
	uint8_t buffer[2] = {cmd, 0};
	flashio(buffer, 2, 0);
	return buffer[1];
}

void print_reg_bit(int val, const char *name)
{
	for (int i = 0; i < 12; i++) {
		if (*name == 0)
			putchar(' ');
		else
			putchar(*(name++));
	}

	putchar(val ? '1' : '0');
	putchar('\n');
}

void cmd_read_flash_regs()
{
	putchar('\n');

	uint8_t sr1 = cmd_read_flash_reg(0x05);
	uint8_t sr2 = cmd_read_flash_reg(0x35);
	uint8_t sr3 = cmd_read_flash_reg(0x15);

	print_reg_bit(sr1 & 0x01, "S0  (BUSY)");
	print_reg_bit(sr1 & 0x02, "S1  (WEL)");
	print_reg_bit(sr1 & 0x04, "S2  (BP0)");
	print_reg_bit(sr1 & 0x08, "S3  (BP1)");
	print_reg_bit(sr1 & 0x10, "S4  (BP2)");
	print_reg_bit(sr1 & 0x20, "S5  (TB)");
	print_reg_bit(sr1 & 0x40, "S6  (SEC)");
	print_reg_bit(sr1 & 0x80, "S7  (SRP)");
	putchar('\n');

	print_reg_bit(sr2 & 0x01, "S8  (SRL)");
	print_reg_bit(sr2 & 0x02, "S9  (QE)");
	print_reg_bit(sr2 & 0x04, "S10 ----");
	print_reg_bit(sr2 & 0x08, "S11 (LB1)");
	print_reg_bit(sr2 & 0x10, "S12 (LB2)");
	print_reg_bit(sr2 & 0x20, "S13 (LB3)");
	print_reg_bit(sr2 & 0x40, "S14 (CMP)");
	print_reg_bit(sr2 & 0x80, "S15 (SUS)");
	putchar('\n');

	print_reg_bit(sr3 & 0x01, "S16 ----");
	print_reg_bit(sr3 & 0x02, "S17 ----");
	print_reg_bit(sr3 & 0x04, "S18 (WPS)");
	print_reg_bit(sr3 & 0x08, "S19 ----");
	print_reg_bit(sr3 & 0x10, "S20 ----");
	print_reg_bit(sr3 & 0x20, "S21 (DRV0)");
	print_reg_bit(sr3 & 0x40, "S22 (DRV1)");
	print_reg_bit(sr3 & 0x80, "S23 (HOLD)");
	putchar('\n');
}
#endif

// --------------------------------------------------------

// The interface to the configurable logic
#define FUNC_AND  0x00
#define FUNC_OR   0x01
#define FUNC_XOR  0x02
#define FUNC_NAND 0x03

typedef struct {
	uint32_t input[4];
	uint32_t func;
} logic_channel_t;

volatile logic_channel_t* logic_channel[] =
	{
		((volatile logic_channel_t*)0x04000000), // LED1 output (E)
		((volatile logic_channel_t*)0x04000400), // LED2 output (F)
		((volatile logic_channel_t*)0x04000800), // LED3 output (G)
		((volatile logic_channel_t*)0x04000C00), // LED4 output (H)
		((volatile logic_channel_t*)0x04001000), // LED5 output (I)
		((volatile logic_channel_t*)0x04001400), // Temporary X
		((volatile logic_channel_t*)0x04001800), // Temporary Y
		((volatile logic_channel_t*)0x04001C00), // Temporary Z
	};

#define reg_logic_readback (*(volatile uint32_t*)0x04000000)

void print_readback_bit(uint32_t readback, int bit, char name)
{
	putchar(name);
	putchar('=');
	putchar((readback & (1 << bit)) ? '1' : '0');
	putchar(' ');
}

int lookup_output(char c) {
	switch(c) {
		case 'E': return 0;
		case 'F': return 1;
		case 'G': return 2;
		case 'H': return 3;
		case 'I': return 4;
		case 'X': return 5;
		case 'Y': return 6;
		case 'Z': return 7;
		default:  return -1;
	};
};

int lookup_function(char c) {
	switch(c) {
		case '&': return FUNC_AND;
		case '|': return FUNC_OR;
		case '^': return FUNC_XOR;
		case '~': return FUNC_NAND;
		default: return -1;
	};
};

int lookup_input(char c) {
	switch(c) {
		case '0': return 0;
		case 'A': return 1;
		case 'B': return 2;
		case 'C': return 3;
		case 'X': return 4;
		case 'Y': return 5;
		case 'Z': return 6;
		case '1': return 7;
		case 'e': return 8;
		case 'f': return 9;
		case 'g': return 10;
		case 'h': return 11;
		case 'i': return 12;
		case 'x': return 13;
		case 'y': return 14;
		case 'z': return 15;
		default:  return -1;
	};
};

// --------------------------------------------------------

void main()
{
	reg_leds = 31;
	reg_uart_clkdiv = 104;
	print("Booting..\n");

	reg_leds = 63;
	set_flash_qspi_flag();

	reg_leds = 127;
	while (getchar_prompt("Press ENTER to continue..\n") != '\r') { /* wait */ }

	print("\n");
	print("  ____  _          ____         ____\n");
	print(" |  _ \\(_) ___ ___/ ___|  ___  / ___|\n");
	print(" | |_) | |/ __/ _ \\___ \\ / _ \\| |\n");
	print(" |  __/| | (_| (_) |__) | (_) | |___\n");
	print(" |_|   |_|\\___\\___/____/ \\___/ \\____|\n");
	print("\n");

	print("Total memory: ");
	print_dec(MEM_TOTAL / 1024);
	print(" KiB\n");
	print("\n");

	cmd_print_spi_state();
	print("\n");

	set_flash_mode_qddr();
	enable_flash_crm();
	print("Set flash to QDDR-CRM mode\n\n");

	cmd_print_spi_state();
	print("\n");

	// Turn LEDs on
	for (int i = 0; i < 5; i++) {
		logic_channel[i]->input[0] = 7;
		for (int j = 1; j < 4; j++)
			logic_channel[i]->input[j] = 0;
		logic_channel[i]->func = FUNC_OR;
	}

	// Command processor
	char c;
	int len, o, f, inp, ci, inv;
	print("Type ? for help.\n");
	while (1) {
		print("> ");
		len = getline();
		if (len == 0)
			continue;
		c = linebuf[0];
		if (c == 'R') {
			// Reset
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 4; j++)
					logic_channel[i]->input[j] = 0;
				logic_channel[i]->func = FUNC_OR;
			}
			continue;
		} else if (c == 'P') {
			uint32_t value = reg_logic_readback;
			print("Inputs:   ");
			print_readback_bit(value, 1, 'A');
			print_readback_bit(value, 2, 'B');
			print_readback_bit(value, 3, 'C');
			putchar('\n');
			print("Internal: ");
			print_readback_bit(value, 4, 'X');
			print_readback_bit(value, 5, 'Y');
			print_readback_bit(value, 6, 'Z');
			putchar('\n');
			print("Outputs:  ");
			print_readback_bit(value,  8, 'E');
			print_readback_bit(value,  9, 'F');
			print_readback_bit(value, 10, 'G');
			print_readback_bit(value, 11, 'H');
			print_readback_bit(value, 12, 'I');
			putchar('\n');
			continue;
		} else if (c == '?') {
			print("Programmable logic configuation.\n");
			print("\n");
			print("'R' resets all configuration.\n");
			print("\n");
			print("Command strings are of form:\n");
			print("<output> = <function> <input0> <input1> ...");
			print("\n");
			print("The 5 LED outputs are named E-H. The 3 button inputs\n");
			print("are named A-B. There are 3 temporaries X, Y, Z that\n");
			print("used as both inputs and outputs.\n");
			print("\n");
			print("<function> can be & (AND), | (OR), ^ (XOR) or ~ (NAND).\n");
			print("\n");
			print("Inputs can be inverted by prefixing with !, 0 and 1\n");
			print("literals can also be used for constant values.\n");
			print("\n");
			print("Example: full adder (LED1=sum, LED2=cout):\n");
			print("    > R\n");
			print("    > X=^AB\n");
			print("    > Y=&AB\n");
			print("    > Z=&XC\n");
			print("    > E=^XC\n");
			print("    > F=^YZ\n");
			print("\n");
			continue;
		}

		if (len < 3 || len > 7)
			goto syntax_error;

		if (linebuf[1] != '=')
			goto syntax_error;

		o = lookup_output(linebuf[0]);
		if (o == -1) goto syntax_error;

		f = lookup_function(linebuf[2]);
		if (f == -1) goto syntax_error;

		ci = 3;
		for (int i = 0; i < 4; i++) {
			inv = 0;
			inp = (f == FUNC_AND || f == FUNC_NAND) ? 7 : 0; // const1 default for AND/NAND, 0 otherwise
			if (ci < len) {
				if (linebuf[ci] == '!') {
					inv = 1;
					++ci;
				}
				if (ci >= len) goto syntax_error;
				inp = lookup_input(linebuf[ci++]);
			}
			if (inp == -1) goto syntax_error;
			logic_channel[o]->input[i] = (inv << 4) | inp;
		}

		logic_channel[o]->func = f;

		continue;
syntax_error:
		print("Syntax error. Type ? for help\n");
	}
}

module top (
	// 12MHz clock
	input  CLK,

	// 6 LEDs on snap-off section (active high)
	output LED1,
	output LED2,
	output LED3,
	output LED4,
	output LED5,

	// UBUTTON (active low)
	input BTN_N,
	// 3 buttons on snap-off section (active high)
	input BTN1,
	input BTN2,
	input BTN3,

	// Red and Green LEDs on mainboard (active LOW)
	output LEDR_N,
	output LEDG_N

	// PMOD extension pins
	//output P1A1, P1A2, P1A3, P1A4, P1A7, P1A8, P1A9, P1A10,
	//output P1B1, P1B2, P1B3, P1B4, P1B7, P1B8, P1B9, P1B10
);

	localparam BITS = 5;
	localparam LOG2DELAY = 22;

	reg [BITS+LOG2DELAY-1:0] counter = 0;
	reg [BITS-1:0] outcnt;

	always @(posedge CLK) begin
		counter <= counter + 1;
		outcnt <= counter >> LOG2DELAY;
	end

	assign {LED1, LED2, LED3, LED4, LED5} = outcnt ^ (outcnt >> 1);

	assign {LEDR_N, LEDG_N} = ~(!BTN_N + BTN1 + BTN2 + BTN3);

endmodule

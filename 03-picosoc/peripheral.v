/*
	An example of a peripheral for the picosoc
	Impelements a simple "programmable logic" device
	with 8 "function generators" driving 5 outputs
	and 3 internal signals. Each function generator
	can implement reduce-and, reduce-or, reduce-xor
	or reduce-nand of 4 values.

	Outputs E..I correspond to the head LEDs
	Inputs A..C correspond to the head buttons
	X,Y,Z are "temporary" signals

	Register map is repeated 8 times for the 8 function generators: for E-I and X-Z

	All input selections:
		0: constant 0
     1..3: inputs A-C
     4..6: temporaries X-Y
        7: constant 1

	0x0i00 input 0 for function i
	0x0i01 input 1 for function i
	0x0i02 input 2 for function i
	0x0i03 input 3 for function i
	0x0i04 function selection for function i:
		0x00: AND
		0X01: OR
		0x02: XOR
		0x03: NAND
*/

module configurable_logic (
	input clk,

	input [15:0] addr,
	input [31:0] wdata,
	input [3:0] wstrb,
	output [31:0] rdata,
	input valid,
	output reg ready,

	input  [2:0] btn,
	output [4:0] led
);

	// Register representing value of the 8 programmable functions
	// (outputs E..I and temps X..Z)
	reg [7:0] values;
	assign led = values[4:0];

	// The 16 possible inputs
	// 		(extension: only 8 are used, fill in the other 8)
	wire [15:0] inputs;
	assign inputs[0] = 1'b0;
	assign inputs[3:1] = btn;
	assign inputs[6:4] = values[7:5];
	assign inputs[7] = 1'b1;
	assign inputs[15:8] = 8'bx;

	// The configuration registers
	reg [3:0] input_sel[(8*4)-1:0];
	reg [1:0] function_sel[7:0];

	// The input muxes
	wire [3:0] selected_inputs[7:0];

	genvar i, j;
	generate
	for (i = 0; i < 8; i = i + 1) begin
		for (j = 0; j < 4; j = j + 1) begin : inmux
			wire [3:0] sel = input_sel[ 4*i + j ];
			assign selected_inputs[i][j] = inputs[sel];
		end
	end
	endgenerate

	// The function generators
	// Note that these are clocked with a "fast" clock to avoid
	// combinational loop issues. For the extension adding "registers"
	// these should be on a slower clock (see below)

	generate
	for (i = 0; i < 8; i = i + 1) begin
		always @(posedge clk) begin
			case (function_sel[i])
				2'b00: values[i] <= &(selected_inputs[i]);
				2'b01: values[i] <= |(selected_inputs[i]);
				2'b10: values[i] <= ^(selected_inputs[i]);
				2'b11: values[i] <= ~&(selected_inputs[i]);
			endcase
		end
	end
	endgenerate

	// Bus interface
	always @(posedge clk)
	begin
		ready <= 1'b0;
		if (valid && !ready) begin
			ready <= 1'b1;
			if (wstrb[0]) begin
				if (addr[3:0] < 4'h4)
					input_sel[{addr[10:8], addr[2:0]}] <= wdata[3:0];
				else if (addr[3:0] == 4'h4)
					function_sel[addr[10:8]] <= wdata[2:0];
			end
		end
	end

	// Sample divided strobe generator
	//
	// reg [19:0] ctr;
	// reg clkdiv;
	// always @(posedge clk)
	//    {clkdiv, ctr} <= ctr + 1'b1;


	// Extension: do something more with this
	assign rdata = 32'bx;
endmodule
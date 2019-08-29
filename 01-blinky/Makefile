PROJ = example

PIN_DEF = icebreaker.pcf

all: $(PROJ).bin

%.json: %.v
	yosys -p 'synth_ice40 -top top -json $@' $<

%.asc: %.json $(PIN_DEF)
	nextpnr-ice40 --up5k --package sg48 --placer heap --pcf $(PIN_DEF) --asc $@ --json $<

%.bin: %.asc
	icepack $< $@

%_tb: %_tb.v %.v
	iverilog -o $@ $^

%_tb.vcd: %_tb
	vvp -N $< +vcd=$@

%_syn.v: %.blif
	yosys -p 'read_json $^; write_verilog -norename  $@'

%_syntb: %_tb.v %_syn.v
	iverilog -o $@ $^ `yosys-config --datdir/ice40/cells_sim.v`

%_syntb.vcd: %_syntb
	vvp -N $< +vcd=$@

prog: $(PROJ).bin
	iceprog $<

show-rtl:
	yosys -p "prep -top top; show" $(PROJ).v

show-pnr: $(PROJ).json
	nextpnr-ice40 --up5k --package sg48 --placer heap --pcf $(PIN_DEF) --gui --json $<

sudo-prog: $(PROJ).bin
	@echo 'Executing prog as root!!!'
	sudo iceprog $<

clean:
	rm -f $(PROJ).json $(PROJ).asc $(PROJ).bin

.SECONDARY:
.PHONY: all prog clean show-rtl show-pnr

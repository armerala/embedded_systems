################################################# 
# delsim do file to run simuilation
##################################################

vlib work 
vmap work work

# Include Netlist and Testbench
vlog -incr fpga_testbench.sv
vlog -incr fpga_top_level.sv
vlog -incr vga_counters.sv
vlog -incr vga_display.sv
vlog -incr common.sv
vlog -incr memory.sv
vlog -incr fifo_buffer.sv 

# Run Simulator 
vsim -t ns -lib work fpga_testbench
do fpga_waveformat.do
run -all

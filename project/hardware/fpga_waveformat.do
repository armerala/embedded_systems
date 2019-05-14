onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate :fpga_testbench:clk
add wave -noupdate :fpga_testbench:i
add wave -noupdate -radix unsigned :fpga_testbench:hps_writedata
add wave -noupdate -radix unsigned :fpga_testbench:hps_addr
add wave -noupdate -radix unsigned :fpga_testbench:r
add wave -noupdate -radix unsigned :fpga_testbench:g
add wave -noupdate -radix unsigned :fpga_testbench:b
add wave -noupdate -radix unsigned :fpga_testbench:fpga:image_mem_addr
add wave -noupdate -radix unsigned :fpga_testbench:fpga:image_mem_we
add wave -noupdate -radix unsigned :fpga_testbench:fpga:image_mem_dout
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_disp:state
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_disp:img_load_cntr
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_render_q:buf_begin
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_render_q:buf_end
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_render_q:pop_front
add wave -noupdate -radix unsigned :fpga_testbench:fpga:vga_render_q:dout
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {3 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 223
configure wave -valuecolwidth 89
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ns} {12 ns}


module mediator(
	input reset,
	input clock,
	input [7:0]  writedata,
	input write,
	input chipselect,
	input [3:0]  address
);

	vga_display vga_disp();
	sdram_controller sdram_ctrl();
	
	always @(posedge reset)
	begin
		//TODO: load in from sdram & lock vga
	end

	always @(packet_data)
	begin
		//TODO: shove proper sprite out to vga
	end

endmodule

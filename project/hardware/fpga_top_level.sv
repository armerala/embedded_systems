module fpga_top_level(
	input clk,
	input reset,

	/*SDRAM INTERFACES*/
	inout [15:0] mem_dq,      //sdram data line
	output [12:0] mem_a,      //sdram address line
	output [1:0] mem_ba,      //sdram mem bank line
	input mem_ck,            //used by sdram
	output mem_cke,           //sdram clock enable
	output mem_ldqm,          //sdram upper data bits mask
	output mem_udqm,          //sdram upper data bits mask
	output mem_we_n,          //sdram write enable -negative
	output mem_cas_n,         //sdram col addr strobe - negative
	output mem_ras_n,         //sdram row addr strobe - negative 
	output mem_cs_n,          //sdram chip select - negative

	/*VGA INTERFACES*/
	output vga_clk,
	output [7:0] vga_r,
	output [7:0] vga_g, 
	output [7:0] vga_b,
	output vga_hs, 
	output vga_vs,
	output vga_blank_n,
	output vga_sync_n

	/*HPS INPUT INTERFACE*/
	input [7:0]  hps_writedata,
	input hps_write,
	input hps_chipselect,
	input [3:0]  hps_address,
);

	wire reset_n; //convenience
	assign reset_n = ~reset;

	//internal stuff to hook img_mem to vga and sdram
	reg img_mem_we;
	reg [3:0] img_mem_addr;
	reg [3:0] img_mem_dout;
	reg [3:0] img_mem_din;
	wire [3:0] img_mem_dout;

	//manage sdram outputs
	wire [2:0] sdram_data_available;

	//instance sdram controller
	sdram_controller sdram_ctrl(
		.mem_dq(mem_dq),
		.mem_cke(mem_cke),
		.mem_a(mem_a),
		.mem_ldqm(mem_ldqm),
		.mem_udqm(mem_udqm),
		.mem_we_n(mem_we_n),
		.mem_cas_n(mem_cas_n),
		.mem_raw_n(mem_ras_n),
		.mem_cs_n(mem_cs_n),
		.ck(mem_ck),
		.reset_n(reset_n),
		.data_avaialable(sdram_data_available)
	);
	
	//instance image memory
	image_memory image_mem(
		.clk(clk)
		.a(img_mem_addr),
		.din(img_mem_data),
		.we(img_mem_we),
		.dout(img_mem_din)
	);

	//instance our vga display
	vga_display vga_disp(
		//TODO: hook up data and magic number interpreation in vga display
		.VGA_R(vga_r),
		.VGA_G(vga_g),
		.VGA_B(vga_b),
		.VGA_CLK(vga_clk), 
		.VGA_HS(vga_hs), 
		.VGA_VS(vga_vs),
		.VGA_BLANK_N(vga_blank_n),
		.VGA_SYNC_N(vga_sync_n)
	);

endmodule

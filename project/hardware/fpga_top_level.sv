

`define FPGA_RESET_STATE 2'b00
`define FPGA_LOADING_STATE 2'b01
`define FPGA_RUNNING_STATE 2'b10


module fpga_top_level(

	input clk50,
	input clk143,
	input reset,

	/*SDRAM INTERFACES*/
	inout [15:0] mem_dq,      //sdram data line
	output [12:0] mem_a,      //sdram address line
	output [1:0] mem_ba,      //sdram mem bank line
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

	reg [1:0] state;
	reg [1:0] next_state;

	//internal stuff to hook img_mem to vga and sdram
	reg img_mem_we;
	reg [3:0] img_mem_addr;
	wire [3:0] img_mem_dout;

	reg fifo_pop_front;
	wire fifo_we;
	wire [15:0] fifo_din;
	wire [23:0] fifo_dout;
	reg [23:0] fifo_dout_buf;
	wire fifo_hw;
	wire fifo_lw;

	reg vga_render_queu_we;

	wire sdram_pause_load;
	wire sdram_unpause_load;
	assign sdram_pause_load = ((state != `FPGA_LOADING_STATE) | (fifo_hw));
	assign sdram_unpause_load = ((state == `FPGA_LOADING_STATE) & (fifo_lw))

	//counters and such for loading
	parameter total_bytes = 512;
	reg [31:0] bytes_to_load;

	//instance sdram controller
	sdram_controller sdram_ctrlr(
		.mem_dq(mem_dq),
		.mem_cke(mem_cke),
		.mem_a(mem_a),
		.mem_ldqm(mem_ldqm),
		.mem_udqm(mem_udqm),
		.mem_we_n(mem_we_n),
		.mem_cas_n(mem_cas_n),
		.mem_raw_n(mem_ras_n),
		.mem_cs_n(mem_cs_n),
		.ck143(clk143),
		.reset_n(reset_n),
		.pause(sdram_pause_load),
		.unpause(sdram_unpause_load),
		.data_available(fifo_we)
	);
	
	//instance image memory
	memory image_mem(
		.clk(clk50),
		.a(img_mem_addr),
		.we(img_mem_we),
		.din(fifo_dout_buf),
		.dout(img_mem_dout)
	);
	defparam image_mem.word_size = 24;
	defparam image_mem.n_words = bytes_to_load * 3;

	//instance our vga display
	vga_display vga_disp(
		//TODO: hook up data and magic number interpreation in vga display
		.clk50(.clk50),
		.reset(reset),
		.render_queue_we(),
		.render_queue_din(hps_write_data),
		.pixel_data(image_mem_dout),
		.pixel_addr(),
		.VGA_R(vga_r),
		.VGA_G(vga_g),
		.VGA_B(vga_b),
		.VGA_CLK(vga_clk), 
		.VGA_HS(vga_hs),
		.VGA_VS(vga_vs),
		.VGA_BLANK_N(vga_blank_n),
		.VGA_SYNC_N(vga_sync_n)
	);

	//instance our fifo buffer to connect sdram and on-board 50MHz memory
	fifo_buffer fifo_buf(
		.clk143(clk143),
		.we(fifo_we),
		.pop_front(fifo_pop_front),
		.din(fifo_din),
		.dout(fifo_dout),
		.buf_lw(fifo_lw),
		.buf_hw(fifo_hw)
	);

	//async reset to sync reset
	always @(posedge reset) begin
		if(reset)
			next_state <= `FPGA_RESET_STATE;
	end

	//synchronous state changes
	always @(negedge clk) begin
		state <= next_state;
		
		fifo_pop_front <= 1'b0;
		image_mem_we <= 1'b0;
	end

	//do business on posgedge (e.g. reading buffer, etc.)
	always @(posedge clk) begin
		
		case(state) begin
			`FPGA_RESET_STATE : begin
				//reset bytes to load and start loading
				bytes_to_load <= total_bytes;
				next_state <= FPGA_LOADING_STATE;
			end
			
			`FPGA_LOADING_STATE : begin
				//case: continue load
				if(bytes_to_load < total_bytes) begin
					fifo_dout_buf <= fifo_dout; //prevent fifo data from changing under us
					if(~fifo_lw || bytes_to_load < 128) begin
						image_mem_we <= 1'b1;
						fifo_pop_front <= 1'b1;
						bytes_to_load <= bytes_to_load - 3;
					end
				//case: load done
				else
					next_state <= `FPGA_RUNNING_STATE;
				end
			end

			`FPGA_RUNNING_STATE : begin
			end
		end

	end

endmodule

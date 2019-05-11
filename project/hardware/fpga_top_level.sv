

`define FPGA_RESET_STATE 2'b00
`define FPGA_LOADING_STATE 2'b01
`define FPGA_RUNNING_STATE 2'b10

`define FIFO_BUF_SIZE 512
`define FIFO_HW_MARK 384
`define FIFO_LW_MARK 128

`define PIXEL_SIZE_BYTES 3
`define N_TOTAL_PIXELS 512


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
	output vga_sync_n,

	/*HPS INPUT INTERFACE*/
	input [7:0]  hps_writedata,
	input hps_write,
	input hps_chipselect,
	input [3:0]  hps_address
);

/**************************************
 * COMPONENTS / WIRES / REGS
 **************************************/

	//img_mem i/o
	reg image_mem_we;
	reg [3:0] image_mem_addr;
	wire [3:0] image_mem_dout;

	//fifo i/o
	reg fifo_pop_front;
	wire fifo_we;
	wire [15:0] fifo_din;
	wire [23:0] fifo_dout;
	reg [23:0] fifo_dout_buf;
	wire fifo_hw;
	wire fifo_lw;

	reg vga_render_queu_we;
	
	//sdram pausing and unpausing
	wire sdram_pause;
	wire sdram_unpause;
	assign sdram_pause = ((state != `FPGA_LOADING_STATE) | fifo_hw);
	assign sdram_unpause = ((state == `FPGA_LOADING_STATE) & fifo_lw);

	//counters and such for loading
	parameter total_bytes = `N_TOTAL_PIXELS * `PIXEL_SIZE_BYTES;
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
		.pause(sdram_pause),
		.unpause(sdram_unpause),
		.data_available(fifo_we)
	);
	
	//instance image memory
	memory image_mem(
		.clk(clk50),
		.a(image_mem_addr),
		.we(image_mem_we),
		.din(fifo_dout_buf),
		.dout(image_mem_dout)
	);
	defparam image_mem.word_size = `PIXEL_SIZE_BYTES * 8;
	defparam image_mem.n_words = `N_TOTAL_PIXELS;

	//instance our vga display
	vga_display vga_disp(
		//TODO: hook up data and magic number interpreation in vga display
		.clk50(clk50),
		.reset(reset),
		.render_queue_we(),
		.render_queue_din(hps_writedata),
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
	defparam fifo_buf.buf_size = `FIFO_BUF_SIZE;
	defparam fifo_buf.hw_mark = `FIFO_HW_MARK;
	defparam fifo_buf.lw_mark = `FIFO_LW_MARK;

	//fsm wires
	wire reset_n;
	assign reset_n = ~reset;
	reg [1:0] state;
	reg [1:0] next_state;

/**************************************
 * FSM DEFINITION
 **************************************/

	//async reset to sync reset
	always @(posedge reset) begin
		if(reset)
			next_state <= `FPGA_RESET_STATE;
	end

	//synchronous state changes
	always @(negedge clk50) begin
		state <= next_state;
		
		fifo_pop_front <= 1'b0;
		image_mem_we <= 1'b0;
	end

	//do business on posgedge (e.g. reading buffer, etc.)
	always @(posedge clk50) begin
		
		case(state)
			`FPGA_RESET_STATE : begin
				//reset bytes to load and start loading
				bytes_to_load <= total_bytes;
				next_state <= `FPGA_LOADING_STATE;
			end
			
			`FPGA_LOADING_STATE : begin
				//case: continue load
				if(bytes_to_load < total_bytes) begin
					fifo_dout_buf <= fifo_dout; //prevent fifo data from changing under us
					if(~fifo_lw || bytes_to_load < `FIFO_LW_MARK) begin
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
		endcase

	end

endmodule

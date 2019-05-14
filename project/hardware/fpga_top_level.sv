`include "common.sv"

module fpga_top_level(

	input clk50,
	input reset,

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
	input [2:0]  hps_address
);

/**************************************
 * COMPONENTS / WIRES / REGS
 **************************************/
	reg do_clear;

	//instance image memory
	reg image_mem_we;
	reg [23:0] image_mem_din;
	wire [23:0] image_mem_dout;

	//i am so sorry
	reg [$clog2(`SPRITE_TOTAL_PIXELS)-1:0] image_mem_write_addr;
	reg [$clog2(`SPRITE_TOTAL_PIXELS)-1:0] image_mem_read_addr;

	reg use_image_read_addr;
	wire [$clog2(`SPRITE_TOTAL_PIXELS)-1:0] image_mem_addr;
	assign image_mem_addr = (use_image_read_addr) ? image_mem_write_addr : image_mem_read_addr;

	memory image_mem(
		.clk(clk50),
		.a(image_mem_addr),
		.we(image_mem_we),
		.din(image_mem_din),
		.dout(image_mem_dout)
	);
	defparam image_mem.word_size = 24;
	defparam image_mem.n_words = `SPRITE_TOTAL_PIXELS;

	//instance vga buffer
	wire vga_render_q_pop_front;
	wire [47:0] vga_render_q_dout;
	reg vga_render_q_we;
	reg [47:0] vga_render_q_din;
	wire lw;
	
	fifo_buffer vga_render_q(
		.clk(clk50),
		.clear(do_clear),
		.we(vga_render_q_we),
		.pop_front(vga_render_q_pop_front),
		.din(vga_render_q_din),
		.dout(vga_render_q_dout),
		.buf_hw(),
		.buf_lw(lw)
	);
	defparam vga_render_q.read_word_size = 48;
	defparam vga_render_q.write_word_size = 48;
	defparam vga_render_q.lw_mark = 48;
	defparam vga_render_q.buf_size = 1200; //48 * 25 words

	//vga display
	vga_display vga_disp(
		.clk50(clk50),
		.reset(reset),
		.render_q_lw(lw),
		.render_queue_pop_front(vga_render_q_pop_front),
		.render_queue_dout(vga_render_q_dout),
		.pixel_din(image_mem_dout),
		.pixel_addr(image_mem_read_addr),
		.VGA_R(vga_r),
		.VGA_G(vga_g),
		.VGA_B(vga_b),
		.VGA_CLK(vga_clk), 
		.VGA_HS(vga_hs),
		.VGA_VS(vga_vs),
		.VGA_BLANK_n(vga_blank_n),
		.VGA_SYNC_n(vga_sync_n)
	);

	reg [7:0] current_op;

	//do business on negedge b/c memory clocks on posedge
	always @(negedge clk50, posedge reset, posedge do_clear) begin

		//reset internal signals
		if(reset) begin
			vga_render_q_we <= 1'b0;
			image_mem_we <= 1'b0;
			use_image_read_addr <= 1'b1;
			do_clear <= 1'b0;
		end
		else if(do_clear) begin
			vga_render_q_we <= 1'b0;
			image_mem_we <= 1'b0;
			use_image_read_addr <= 1'b1;
			do_clear <= 1'b0;
		end
		
		//figure out what to do
		else if(hps_write && hps_chipselect) begin
			
			//cache the current op from first 8 bits
			if(hps_address == 3'h0) begin
				current_op <= hps_writedata;
				if(hps_writedata != 8'hfd && hps_writedata != 8'hfe )
					vga_render_q_din[47:40] <= hps_writedata;
			end

			//check for clear
			else if (current_op == 8'hfe) begin
				do_clear <= 1'b1;
			end

			//check for load pixel
			else if(current_op == 8'hfd) begin

				case(hps_address)
					3'h1 : image_mem_din[23:16] <= hps_writedata;
					3'h2 : image_mem_din[15:8] <= hps_writedata;
					3'h3 : image_mem_din[7:0] <= hps_writedata;
					3'h4 : image_mem_write_addr[19:16] <= hps_writedata;
					3'h5 : image_mem_write_addr[15:8] <= hps_writedata;
					3'h6 : begin 
						image_mem_write_addr[7:0] <= hps_writedata; 
						image_mem_we <= 1'b1; 
						use_image_read_addr <= 1'b0; 
					end
				endcase

			end

			//check for render request
			else begin 

				case(hps_address)
					3'h1 : vga_render_q_din[39:32] <= hps_writedata;
					3'h2 : vga_render_q_din[31:24] <= hps_writedata;
					3'h3 : vga_render_q_din[23:16] <= hps_writedata;
					3'h4 : vga_render_q_din[15:8] <= hps_writedata;
					3'h5 : begin vga_render_q_din[7:0] <= hps_writedata; vga_render_q_we <= 1'b1; end
				endcase

			end

		end
	end

endmodule

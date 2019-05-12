/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 *
 *
 * 	Name / UNI
 * 		Daniel Mesko / dpm2153
 * 		Cansu Cabuk / cc4455
 *   	Alan Armero / aa3938
 */

`define VGA_RESET 2'b00
`define VGA_WAITING_TO_RENDER 2'b00
`define VGA_INSTRUCTION_FETCH 2'b00
`define VGA_RENDERING 2'b10

module vga_display(
	input clk50,
	input reset,

    input render_queue_we,
    input [7:0] render_queue_din,

	input [23:0] pixel_din,
    output reg [14:0] pixel_addr,

	input do_render,
	output reg done_rendering,

	output [7:0] VGA_R,
	output [7:0] VGA_G,
	output [7:0] VGA_B,
	output VGA_CLK, 
	output VGA_HS, 
	output VGA_VS,
	output VGA_BLANK_n,
	output VGA_SYNC_n
  );

	reg state;
	reg next_state;

	/**DOUBLE BUFFER THE OUTPUT**/

	//buf 1
	reg buf1_we;
	reg buf1_addr;
	reg buf1_din;
	wire buf1_dout;
	memory buf1(
		.clk50(clk50),
		.we(buf1_we)
		.a(buf1_addr),
		.din(buf1_din),
		.dout(buf1_dout)
	;

	//buf 2
	reg buf2_we;
	reg buf2_addr;
	reg buf2_din;
	wire buf2_dout;
	memory buf2(
		.clk50(clk50),
		.we(buf2_we)
		.a(buf2_addr),
		.din(buf2_din),
		.dout(buf2_dout)
	);

	//control which is read or write buf
	reg read_b1;

	wire read_buf_we;
	wire read_buf_addr;
	wire read_buf_din;
	wire read_buf_dout;
	assign read_buf_we = (read_b1) ? buf1_we : buf2_we;
	assign read_buf_addr = (read_b1) ? buf1_addr : buf2_addr;
	assign read_buf_din = (read_b1) ? buf1_din : buf2_din;
	assign read_buf_dout = (read_b1) ? buf1_dout : buf2_dout;

	wire write_buf_we;
	wire write_buf_addr;
	wire write_buf_din;
	wire write_buf_dout;
	assign write_buf_we = (~read_b1) ? buf1_we : buf2_we;
	assign write_buf_addr = (~read_b1) ? buf1_addr : buf2_addr;
	assign write_buf_din = (~read_b1) ? buf1_din : buf2_din;
	assign write_buf_dout = (~read_b1) ? buf1_dout : buf2_dout;
	
	//keep trakc of currently loading sprite
	reg [14:0] img_load_cntr;
	reg [14:0] img_to_load_size;

	//render queue
	reg [$clog2(25)-1:0] render_q_addr;
	reg [$clog2(25)-1:0] render_q_len;
	reg [7:0] image_magic;
	wire render_q_dout;

	memory render_q(
		.clk50(clk50),
		.we(render_queue_we),
		.a(render_q_addr),
		.din(render_queue_din),
		.dout(render_queue_dout)
	);
	.defparam render_q.word_size = 8;
	.defparam render_q.n_words = 25;

	//async reset
	always @(posedge reset) begin
		if(reset)
			next_state <= VGA_RESET;
	end

	//state transitions on posedge
	always @(posedge clk50) begin
		state <= next_state;
	end

	//do work on negedge because data is put in the buffer on the posedge from
	//fpga top level
	always @(negedge clk50) begin
		
		clear_render_queue_internal <= 1'b0;

		case(state) begin

			`VGA_RESET : begin
				render_q_addr <= 0;
				image_magic <= 0;
			end

			//queue up render requests
			`VGA_WAITING_TO_RENDER : begin
				if(do_render) begin
					next_state <= `VGA_INSTRUCTION_FETCH;
					render_q_addr <= 0;
				end
				else if(we) begin
					render_q_addr <= render_q_addr + 1;
					render_q_len <= render_q_addr;
				end
			end
		
			//fetch next instruction
			`VGA_INSTRUCTION_FETCH : begin

				img_magic <= render_q_dout[7:0]; //TODO figure out format
				img_load_size <= render_q_dout[7:0];
				img_load_cntr <= 1;
				//pixel_addr <= 1; // + SPRITE_OFFSET //pre-fetch the first pixel

				//done rendering
				if(render_q_addr == render_q_len) begin
					render_q_len <= 0;
					render_q_addr <= 0;
					next_state <= `VGA_WAITING_TO_RENDER;
				end
				else begin
					next_state <= `VGA_RENDERING;
				end
			end

			//rendering phase
			`VGA_RENDERING : begin

				if(img_load_cntr == img_load_size)
					next_state <= `VGA_INSTRUCTION_FETCH;

				//pixel_addr <= img_load_cntr; // + offset;
				img_load_cntr < img_load_cntr + 1;
				//vga_out[addr] <= pixel_din;
			end

		end
	end


   logic [10:0]	   hcount;
   logic [9:0]     vcount;

   logic [7:0]	   background_r, background_g, background_b;
   logic [7:0] 	   pos_x, pos_y, spnum, pbit;

   vga_counters counters(.clk50(clk50), .*);

	/*
   always_ff @(posedge clk)
     if (reset) begin
	background_r <= 8'h0;
	background_g <= 8'h0;
	background_b <= 8'h80;
     end else if (chipselect && write)
       case (address)
	 3'h0 : pos_x <= writedata;
	 3'h1 : pos_y <= writedata;
	 3'h2 : spnum <= writedata;
	 3'h3 : pbit <= writedata;
       endcase

   always_comb begin
      {VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, 8'h0};
      if (VGA_BLANK_n )

	// dummy test
		if (spnum == 0)	  		
			{VGA_R, VGA_G, VGA_B} = {8'hff, 8'hff, 8'hff};
		else if (spnum == 1)
			{VGA_R, VGA_G, VGA_B} = {8'h00f, 8'h00f, 8'h00f};
	end else
	  {VGA_R, VGA_G, VGA_B} =
             {background_r, background_g, background_b};
   end
	*/
	       
endmodule

module vga_counters(
 input clk50, 
 input reset,
 output [10:0] hcount,  // hcount[10:1] is pixel column
 output [9:0]  vcount,  // vcount[9:0] is pixel row
 output VGA_CLK, 
 output VGA_HS,
 output VGA_VS, 
 output VGA_BLANK_n, 
 output VGA_SYNC_n);

/*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
   // Parameters for hcount
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

   logic endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else  	         hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;
       
   logic endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

   assign endOfField = vcount == VTOTAL - 1;

   // Horizontal sync: from 0x520 to 0x5DF (0x57F)
   // 101 0010 0000 to 101 1101 1111
   assign VGA_HS = !( (hcount[10:8] == 3'b101) &
		      !(hcount[7:5] == 3'b111));
   assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1'b0; // For putting sync on the green signal; unused
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to 479
   // 101 0000 0000  1280	       01 1110 0000  480
   // 110 0011 1111  1599	       10 0000 1100  524
   assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
			!( vcount[9] | (vcount[8:5] == 4'b1111) );

   /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
   assign VGA_CLK = hcount[0]; // 25 MHz clock: rising edge sensitive
   
endmodule
